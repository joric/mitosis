#include <stdarg.h>
#include "nrf_gzll.h"
#include "nrf_delay.h"
#include "nrf_drv_rtc.h"
#include "nrf_soc.h"

#define COMPILE_REVERSED

#ifdef COMPILE_REVERSED
#define COMPILE_LEFT
#else
#define COMPILE_RIGHT
#endif

#include "mitosis.h"
#include "mitosis_keymap.h"

// Receiver support
typedef enum
{
    BLE,
    GAZELL
} radio_mode_t;

radio_mode_t running_mode = BLE;

#if !(NRF_LOG_ENABLED)
#define SIMPLE_DEBUG
#endif

#ifdef SIMPLE_DEBUG
#include "app_uart.h"

#undef NRF_LOG_INFO
#undef NRF_LOG_DEBUG
#undef NRF_LOG_PROCESS
#define NRF_LOG_INFO printf
#define NRF_LOG_DEBUG printf
#define NRF_LOG_PROCESS() false

#undef NRF_LOG_INIT
#define NRF_LOG_INIT debug_init

#undef RX_PIN_NUMBER
#undef TX_PIN_NUMBER
#undef CTS_PIN_NUMBER
#undef RTS_PIN_NUMBER
#undef HWFC

#define RX_PIN_NUMBER  -1
#define TX_PIN_NUMBER  21
#define CTS_PIN_NUMBER -1
#define RTS_PIN_NUMBER -1
#define HWFC false

void debug_log(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    char buf[256] = { 0 };
    vsprintf(buf, fmt, list);
    va_end(list);
    for (char *p = buf; *p; p++)
        app_uart_put(*p); // needs fifo library
}

void uart_error_handle(app_uart_evt_t * p_event)
{
}

uint32_t debug_init()
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params = {
        RX_PIN_NUMBER, TX_PIN_NUMBER, RTS_PIN_NUMBER, CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED, false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };
    APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOW, err_code);
    APP_ERROR_CHECK(err_code);
    printf("\nUART initialized\n");
    return err_code;
}

#undef APP_ERROR_CHECK
#define APP_ERROR_CHECK(x) if (x!=NRF_SUCCESS) printf("ERROR 0x%04x in line %u\n", (int)x, __LINE__)

#endif // SIMPLE_DEBUG

// Define payload length
#define TX_PAYLOAD_LENGTH 3 ///< 3 byte payload length when transmitting

static uint8_t data_payload_left[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];    ///< Placeholder for data payload received from host.
static uint8_t data_payload_right[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];    ///< Placeholder for data payload received from host.

static bool packet_received_left, packet_received_right;
static uint8_t ack_payload[TX_PAYLOAD_LENGTH];    ///< Payload to attach to ACK sent to device.
uint32_t left_active = 0;
uint32_t right_active = 0;
void key_handler();

// Key buffers
static uint32_t keys = 0, keys_snapshot = 0;
static uint32_t keys_recv = 0, keys_recv_snapshot = 0;
static uint8_t data_buffer[10];


// Setup switch pins with pullups
static void gpio_config(void)
{
    nrf_gpio_cfg_sense_input(S01, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S02, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S03, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S04, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S05, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S06, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S07, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S08, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S09, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S10, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S11, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S12, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S13, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S14, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S15, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S16, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S17, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S18, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S19, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S20, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S21, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S22, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(S23, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
}


void keys_to_payload(uint8_t * data_payload, uint32_t keys)
{
    data_payload[0] = ((keys & 1<<S01) ? 1:0) << 7 | \
                      ((keys & 1<<S02) ? 1:0) << 6 | \
                      ((keys & 1<<S03) ? 1:0) << 5 | \
                      ((keys & 1<<S04) ? 1:0) << 4 | \
                      ((keys & 1<<S05) ? 1:0) << 3 | \
                      ((keys & 1<<S06) ? 1:0) << 2 | \
                      ((keys & 1<<S07) ? 1:0) << 1 | \
                      ((keys & 1<<S08) ? 1:0) << 0;

    data_payload[1] = ((keys & 1<<S09) ? 1:0) << 7 | \
                      ((keys & 1<<S10) ? 1:0) << 6 | \
                      ((keys & 1<<S11) ? 1:0) << 5 | \
                      ((keys & 1<<S12) ? 1:0) << 4 | \
                      ((keys & 1<<S13) ? 1:0) << 3 | \
                      ((keys & 1<<S14) ? 1:0) << 2 | \
                      ((keys & 1<<S15) ? 1:0) << 1 | \
                      ((keys & 1<<S16) ? 1:0) << 0;

    data_payload[2] = ((keys & 1<<S17) ? 1:0) << 7 | \
                      ((keys & 1<<S18) ? 1:0) << 6 | \
                      ((keys & 1<<S19) ? 1:0) << 5 | \
                      ((keys & 1<<S20) ? 1:0) << 4 | \
                      ((keys & 1<<S21) ? 1:0) << 3 | \
                      ((keys & 1<<S22) ? 1:0) << 2 | \
                      ((keys & 1<<S23) ? 1:0) << 1 | \
                      0 << 0;
}


void payload_to_data_left()
{
    data_buffer[0] = ((data_payload_left[0] & 1<<3) ? 1:0) << 0 |
                     ((data_payload_left[0] & 1<<4) ? 1:0) << 1 |
                     ((data_payload_left[0] & 1<<5) ? 1:0) << 2 |
                     ((data_payload_left[0] & 1<<6) ? 1:0) << 3 |
                     ((data_payload_left[0] & 1<<7) ? 1:0) << 4;

    data_buffer[2] = ((data_payload_left[1] & 1<<6) ? 1:0) << 0 |
                     ((data_payload_left[1] & 1<<7) ? 1:0) << 1 |
                     ((data_payload_left[0] & 1<<0) ? 1:0) << 2 |
                     ((data_payload_left[0] & 1<<1) ? 1:0) << 3 |
                     ((data_payload_left[0] & 1<<2) ? 1:0) << 4;

    data_buffer[4] = ((data_payload_left[1] & 1<<1) ? 1:0) << 0 |
                     ((data_payload_left[1] & 1<<2) ? 1:0) << 1 |
                     ((data_payload_left[1] & 1<<3) ? 1:0) << 2 |
                     ((data_payload_left[1] & 1<<4) ? 1:0) << 3 |
                     ((data_payload_left[1] & 1<<5) ? 1:0) << 4;

    data_buffer[6] = ((data_payload_left[2] & 1<<5) ? 1:0) << 1 |
                     ((data_payload_left[2] & 1<<6) ? 1:0) << 2 |
                     ((data_payload_left[2] & 1<<7) ? 1:0) << 3 |
                     ((data_payload_left[1] & 1<<0) ? 1:0) << 4;

    data_buffer[8] = ((data_payload_left[2] & 1<<1) ? 1:0) << 1 |
                     ((data_payload_left[2] & 1<<2) ? 1:0) << 2 |
                     ((data_payload_left[2] & 1<<3) ? 1:0) << 3 |
                     ((data_payload_left[2] & 1<<4) ? 1:0) << 4;
}


void payload_to_data_right()
{
    data_buffer[1] = ((data_payload_right[0] & 1<<7) ? 1:0) << 0 |
                     ((data_payload_right[0] & 1<<6) ? 1:0) << 1 |
                     ((data_payload_right[0] & 1<<5) ? 1:0) << 2 |
                     ((data_payload_right[0] & 1<<4) ? 1:0) << 3 |
                     ((data_payload_right[0] & 1<<3) ? 1:0) << 4;

    data_buffer[3] = ((data_payload_right[0] & 1<<2) ? 1:0) << 0 |
                     ((data_payload_right[0] & 1<<1) ? 1:0) << 1 |
                     ((data_payload_right[0] & 1<<0) ? 1:0) << 2 |
                     ((data_payload_right[1] & 1<<7) ? 1:0) << 3 |
                     ((data_payload_right[1] & 1<<6) ? 1:0) << 4;

    data_buffer[5] = ((data_payload_right[1] & 1<<5) ? 1:0) << 0 |
                     ((data_payload_right[1] & 1<<4) ? 1:0) << 1 |
                     ((data_payload_right[1] & 1<<3) ? 1:0) << 2 |
                     ((data_payload_right[1] & 1<<2) ? 1:0) << 3 |
                     ((data_payload_right[1] & 1<<1) ? 1:0) << 4;

    data_buffer[7] = ((data_payload_right[1] & 1<<0) ? 1:0) << 0 |
                     ((data_payload_right[2] & 1<<7) ? 1:0) << 1 |
                     ((data_payload_right[2] & 1<<6) ? 1:0) << 2 |
                     ((data_payload_right[2] & 1<<5) ? 1:0) << 3;

    data_buffer[9] = ((data_payload_right[2] & 1<<4) ? 1:0) << 0 |
                     ((data_payload_right[2] & 1<<3) ? 1:0) << 1 |
                     ((data_payload_right[2] & 1<<2) ? 1:0) << 2 |
                     ((data_payload_right[2] & 1<<1) ? 1:0) << 3;
}


//void notification_cb(nrf_impl_notification_t notification);
/*lint -e526 "Symbol RADIO_IRQHandler not defined" */
void RADIO_IRQHandler(void);

static nrf_radio_request_t m_timeslot_request;
static uint32_t m_slot_length;
static volatile bool m_cmd_received = false;
static volatile bool m_gzll_initialized = false;
static nrf_radio_signal_callback_return_param_t signal_callback_return_param;

void HardFault_Handler(uint32_t program_counter, uint32_t link_register)
{
}


void m_configure_next_event(void)
{
    m_slot_length = 10000;
    m_timeslot_request.request_type = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk = NRF_RADIO_HFCLK_CFG_NO_GUARANTEE;
    m_timeslot_request.params.earliest.priority = NRF_RADIO_PRIORITY_NORMAL;
    m_timeslot_request.params.earliest.length_us = m_slot_length;
    m_timeslot_request.params.earliest.timeout_us = 100000;
}


void sys_evt_dispatch_ble_gzll(uint32_t evt_id)
{
    uint32_t err_code;

    switch (evt_id)
    {
        case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
            ASSERT(false);
            break;

        case NRF_EVT_RADIO_SESSION_IDLE:
            ASSERT(false);
            break;

        case NRF_EVT_RADIO_SESSION_CLOSED:
            ASSERT(false);
            break;

        case NRF_EVT_RADIO_BLOCKED:
            //printf("Blocked\n");
            m_configure_next_event();
            err_code = sd_radio_request(&m_timeslot_request);
            APP_ERROR_CHECK(err_code);
            break;

        case NRF_EVT_RADIO_CANCELED:
            m_configure_next_event();
            err_code = sd_radio_request(&m_timeslot_request);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }

    fs_sys_event_handler(evt_id);
    ble_advertising_on_sys_evt(evt_id);
}


void m_process_gazell()
{
    // detecting received packet from interupt, and unpacking
    if (packet_received_left)
    {
        packet_received_left = false;
        payload_to_data_left();
    }

    if (packet_received_right)
    {
        packet_received_right = false;
        payload_to_data_right();
    }
}


static void m_on_start(void)
{
    bool res = false;
    signal_callback_return_param.params.request.p_next = NULL;
    signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
    nrf_gzll_mode_t mode = running_mode == BLE ? NRF_GZLL_MODE_HOST : NRF_GZLL_MODE_DEVICE;

    if (!m_gzll_initialized)
    {
        // Initialize Gazell
        nrf_gzll_init(mode);

        nrf_gzll_set_device_channel_selection_policy(NRF_GZLL_DEVICE_CHANNEL_SELECTION_POLICY_USE_CURRENT);
        nrf_gzll_set_xosc_ctl(NRF_GZLL_XOSC_CTL_MANUAL);
        nrf_gzll_set_max_tx_attempts(0);    // NB! 100?? reversebias

        // Addressing
        nrf_gzll_set_base_address_0(0x01020304);
        nrf_gzll_set_base_address_1(0x05060708);

        // Load data into TX queue
        ack_payload[0] = 0x55;
        nrf_gzll_add_packet_to_tx_fifo(0, data_payload_left, TX_PAYLOAD_LENGTH);
        nrf_gzll_add_packet_to_tx_fifo(1, data_payload_left, TX_PAYLOAD_LENGTH);

        // Enable Gazell to start sending over the air
        nrf_gzll_enable();

        m_gzll_initialized = true;
    } else {
        res = nrf_gzll_set_mode(mode);
        ASSERT(res);
    }

    NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    NRF_TIMER0->CC[0] = m_slot_length - 4000;    // TODO: Use define instead of magic number
    NVIC_EnableIRQ(TIMER0_IRQn);

    (void)res;
}


static void m_on_multitimer(void)
{
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
    if (nrf_gzll_get_mode() != NRF_GZLL_MODE_SUSPEND)
    {
        signal_callback_return_param.params.request.p_next = NULL;
        signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
        (void)nrf_gzll_set_mode(NRF_GZLL_MODE_SUSPEND);
        NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
        NRF_TIMER0->CC[0] = m_slot_length - 1000;
    }
    else
    {
        ASSERT(nrf_gzll_get_mode() == NRF_GZLL_MODE_SUSPEND);
        m_configure_next_event();
        signal_callback_return_param.params.request.p_next = &m_timeslot_request;
        signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
    }
}


nrf_radio_signal_callback_return_param_t *m_radio_callback(uint8_t signal_type)
{
    switch (signal_type)
    {
        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
            m_on_start();
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
            signal_callback_return_param.params.request.p_next = NULL;
            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
            RADIO_IRQHandler();
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
            m_on_multitimer();
            break;
    }
    return (&signal_callback_return_param);
}


uint32_t gazell_sd_radio_init(void)
{
    uint32_t err_code;
    err_code = sd_radio_session_open(m_radio_callback);
    if (err_code != NRF_SUCCESS)
        return err_code;
    m_configure_next_event();
    err_code = sd_radio_request(&m_timeslot_request);
    if (err_code != NRF_SUCCESS)
    {
        (void)sd_radio_session_close();
        return err_code;
    }
    return NRF_SUCCESS;
}


void nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
    if (running_mode == BLE)
        return;
    uint32_t ack_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
    if (tx_info.payload_received_in_ack)
    {
        // Pop packet and write first byte of the payload to the GPIO port.
        nrf_gzll_fetch_packet_from_rx_fifo(pipe, ack_payload, &ack_payload_length);
    }
}


void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
}


void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info)
{
    if (running_mode == GAZELL)
        return;

    //printf("gzll data received\n");

    uint32_t data_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;

    if (pipe == 0)
    {
        packet_received_left = true;
        left_active = 0;
        // Pop packet and write first byte of the payload to the GPIO port.
        nrf_gzll_fetch_packet_from_rx_fifo(pipe, data_payload_left, &data_payload_length);
    }
    else if (pipe == 1)
    {
        packet_received_right = true;
        right_active = 0;
        // Pop packet and write first byte of the payload to the GPIO port.
        nrf_gzll_fetch_packet_from_rx_fifo(pipe, data_payload_right, &data_payload_length);
    }
    // not sure if required, I guess if enough packets are missed during blocking uart
    nrf_gzll_flush_rx_fifo(pipe);

    //load ACK payload into TX queue
    ack_payload[0] = 0x55;
    nrf_gzll_add_packet_to_tx_fifo(pipe, ack_payload, TX_PAYLOAD_LENGTH);


    m_process_gazell();
}


void nrf_gzll_disabled(void)
{
}


bool debug_cmd_available(void)
{
    return m_cmd_received;
}


char get_debug_cmd(void)
{
    char cmd = ack_payload[0];
    m_cmd_received = false;
    return cmd;
}


uint8_t get_battery_level(void)
{
    // Configure ADC
    NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos) | (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) | (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) | (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);

    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
    NRF_ADC->EVENTS_END = 0;    // Stop any running conversions.
    NRF_ADC->TASKS_START = 1;

    while (!NRF_ADC->EVENTS_END)
    {
    }

    uint16_t vbg_in_mv = 1200;
    uint8_t adc_max = 255;
    uint16_t vbat_current_in_mv = (NRF_ADC->RESULT * 3 * vbg_in_mv) / adc_max;

    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->TASKS_STOP = 1;

    int percent = battery_level_in_percent(vbat_current_in_mv); // app_util.h

    printf("Sending BAS report: %u%% (%umV)\n", percent, vbat_current_in_mv);

    return (uint8_t) percent;
}


// Return the key states, masked with valid key pins
static uint32_t read_keys(void)
{
//#define FILTER_OUT_UART_PIN_KEY // uncomment on unwanted HID reports
#ifdef FILTER_OUT_UART_PIN_KEY
    uint32_t uart_pin = 1 << TX_PIN_NUMBER;
    return ~NRF_GPIO->IN & INPUT_MASK & ~uart_pin;
#else
    return ~NRF_GPIO->IN & INPUT_MASK;
#endif
}


static void send_data()
{
    keys_to_payload(data_payload_right, keys);

    if (running_mode == GAZELL)
    {
        nrf_gzll_add_packet_to_tx_fifo(PIPE_NUMBER, data_payload_right, TX_PAYLOAD_LENGTH);
    }
    else
    {
        payload_to_data_right();
    }
}


#define DEBOUNCE 25                // standard 25 ms refresh
#define ACTIVITY 4*60*1000        // unactivity time till power off (4 minutes)
#define RESET_DELAY 250            // delayed reset

//static uint32_t activity_ticks = 0;
//static uint32_t reset_ticks = 0;
//static bool m_delayed_reset = false;

#define KEY_FN S20
#define KEY_ADJUST S23
#define KEY_D1 S16
#define KEY_D2 S17
#define KEY_D3 S18
#define KEY_D4 S19

#if 0
// former 1000Hz debounce sampling - it's 25 ms now. do we still need debouncing? seem to work fine
static void handler_debounce(void *p_context)
{
    // right half
    keys_snapshot = read_keys();
    if (keys != keys_snapshot)
    {
        keys = keys_snapshot;
        send_data();
        key_handler();

        ///////////////////////////////////////
        // hardware keys
        int index = keys & (1<<KEY_D1) ? 0 : keys & (1<<KEY_D2) ? 1 : keys & (1<<KEY_D3) ? 2 : keys & (1<<KEY_D4) ? 3 : -1;
        bool adjust_pressed = keys & (1<<KEY_ADJUST);
        bool fn_pressed = keys & (1<<KEY_FN);
        bool rf_pressed = keys & (1<<KEY_D4);

        if (adjust_pressed && index!=-1)
        {
            int next_mode = (rf_pressed && !fn_pressed) ? GAZELL : BLE;
            bool erase_bonds = rf_pressed && fn_pressed && adjust_pressed;

            if (erase_bonds)
            {
                index = 0;
                next_mode = BLE;
            }

            //switch_select(index);

            if (next_mode!=running_mode || erase_bonds)
            {
                m_delayed_reset = true;
            }

            if (running_mode==BLE && next_mode==BLE)
            {
                if (fn_pressed)
                {
                    //switch_reset(&m_bonded_peer_handle);
                }
                //advertising_restart(BLE_ADV_MODE_FAST);
            }
        }
        /// hardware keys
        ///////////////////////////////////////
    }

    // left half
    keys_recv = data_payload_left[0] | (data_payload_left[1] << 8) | (data_payload_left[2] << 16);
    if (keys_recv != keys_recv_snapshot)
    {
        keys_recv_snapshot = keys_recv;
        key_handler();
    }

    if (keys == 0 && keys_recv == 0)
    {
        activity_ticks++;
        if (activity_ticks * DEBOUNCE > ACTIVITY)
        {
            printf("Shutting down on inactivity...\n");
            //nrf_delay_ms(50);
            sd_power_system_off();
        }
    } else {
        activity_ticks = 0;
    }

    if (m_delayed_reset)
    {
        reset_ticks++;
        if (reset_ticks * DEBOUNCE > RESET_DELAY)
        {
            reset_ticks = 0;
            m_delayed_reset = false;
            NVIC_SystemReset();
        }
    }
    else
    {
        reset_ticks = 0;
    }
}
#endif


uint8_t m_layer = 0;


uint8_t get_modifier(uint16_t key)
{
    const int modifiers[] = { KC_LCTRL, KC_LSHIFT, KC_LALT, KC_LGUI, KC_RCTRL, KC_RSHIFT, KC_RALT, KC_RGUI };
    for (int b = 0; b < 8; b++)
        if (key == modifiers[b])
            return 1 << b;
    return 0;
}


void key_handler()
{
    printf("key_handler %d %d\n", (int)keys_recv, (int)keys);

    if (running_mode == GAZELL)
        return;

    const int MAX_KEYS = 6;
    uint8_t buf[8];
    int modifiers = 0;
    int keys_pressed = 0;
    int keys_sent = 0;
    memset(buf, 0, sizeof(buf));

    for (uint8_t row = 0; row < MATRIX_ROWS; row++)
    {
        uint16_t val = (uint16_t) data_buffer[row * 2] | (uint16_t) data_buffer[row * 2 + 1] << 5;
        if (val)
        {
            for (int col = 0; col < 16; col++)
            {
                if (val & (1 << col))
                {
                    keys_pressed++;
                    uint16_t key = keymaps[m_layer][row][col];
                    if (key == KC_TRNS)
                        key = keymaps[0][row][col];
                    uint8_t modifier = get_modifier(key);

                    if (modifier)
                    {
                        modifiers |= modifier;
                    }
                    else if (key & QK_LAYER_TAP)
                    {
                        m_layer = (key >> 8) & 0xf;
                    }
                    else if (keys_sent < MAX_KEYS)
                    {
                        buf[2 + keys_sent++] = key;
                    }
                }
            }
        }
    }

    if (!keys_pressed)
        m_layer = 0;

    buf[0] = modifiers;
    buf[1] = 0; // reserved

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        printf("report %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        uint32_t err_code = ble_hids_inp_rep_send(&m_hids, INPUT_REPORT_KEYS_INDEX, INPUT_REPORT_KEYS_MAX_LEN, buf);
        APP_ERROR_CHECK(err_code);
    }

}


void keyboard_task()
{
    keys_snapshot = read_keys();

    if (keys != keys_snapshot)
    {
        keys = keys_snapshot;
        send_data();
        key_handler();
    }

    keys_recv = data_payload_left[0] | (data_payload_left[1] << 8) | (data_payload_left[2] << 16);

    if (keys_recv != keys_recv_snapshot)
    {
        keys_recv_snapshot = keys_recv;
        key_handler();
    }
}


/*   add to main.c:

    // Create scan timer (timers_init)
    err_code = app_timer_create(&m_keyboard_scan_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                keyboard_scan_timeout_handler);
    APP_ERROR_CHECK(err_code);

    // Start scan timer (timers_start)
    err_code = app_timer_start(m_keyboard_scan_timer_id, KEYBOARD_SCAN_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
*/


APP_TIMER_DEF(m_keyboard_scan_timer_id);
#define KEYBOARD_FAST_SCAN_INTERVAL 25
#define KEYBOARD_SCAN_INTERVAL APP_TIMER_TICKS(KEYBOARD_FAST_SCAN_INTERVAL, APP_TIMER_PRESCALER)                  /**< Keyboard scan interval (ticks). */


static void keyboard_scan_timeout_handler(void *p_context)
{
    UNUSED_PARAMETER(p_context);
    // well, as fast as possible, it's impossible.
    keyboard_task();
}


void mitosis_init()
{
    printf("Mitosis init\n");
    gpio_config();

    nrf_gpio_cfg_output(LED_PIN);
    for (int i = 0; i < 3; i++)
    {
        nrf_gpio_pin_set(LED_PIN);
        nrf_delay_ms(100);
        nrf_gpio_pin_clear(LED_PIN);
        nrf_delay_ms(100);
    }

    gazell_sd_radio_init();
}
