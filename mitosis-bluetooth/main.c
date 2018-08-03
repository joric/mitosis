// Mitosis-BT, https://github.com/joric/mitosis/tree/devel

#define COMPILE_RIGHT
#include "mitosis.h"

#define MAX_DEVICES 3			// 2 bluetooth devices + 1 rf
#define KEY_BT S20
#define KEY_RF S23

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "nrf_drv_config.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf_gzll.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_conn_params.h"
#include "ble_dis.h"
#include "ble_hci.h"
#include "ble_hids.h"
#include "ble_srv_common.h"
#include "app_button.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "app_trace.h"
#include "app_util_platform.h"
#include "pstorage.h"
#include "device_manager.h"
#include "softdevice_handler_appsh.h"

#ifndef NRF_LOG_ENABLED
#define SIMPLE_DEBUG
#endif

#ifdef SIMPLE_DEBUG
#include "app_uart.h"

#define RX_PIN_NUMBER  -1
#define TX_PIN_NUMBER  21
#define CTS_PIN_NUMBER -1
#define RTS_PIN_NUMBER -1
#define HWFC false

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 256

#undef app_trace_log
void app_trace_log(const char *fmt, ...) {
	va_list list;
	va_start(list, fmt);
	char buf[256] = { 0 };
	vsprintf(buf, fmt, list);
	va_end(list);
	for (char *p = buf; *p; p++)
		app_uart_put(*p);
}

void uart_error_handle(app_uart_evt_t * p_event) {
}

#undef app_trace_init
void app_trace_init() {
	uint32_t err_code;
	const app_uart_comm_params_t comm_params = {
		RX_PIN_NUMBER, TX_PIN_NUMBER, RTS_PIN_NUMBER, CTS_PIN_NUMBER,
		APP_UART_FLOW_CONTROL_DISABLED, false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};
	APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOW, err_code);
	APP_ERROR_CHECK(err_code);
	app_trace_log("\nUART initialized\n");
}
#endif // SIMPLE_DEBUG

#define ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define ADDR_T(a) a[5], a[4], a[3], a[2], a[1], a[0]

#define TN(id) {static char buf[16]; sprintf(buf, "0x%04x", id); return buf; }
#define T(id) if (type == id) return #id; else

char *error_name(int type) {
	T(BLE_HCI_STATUS_CODE_SUCCESS);
	T(BLE_HCI_STATUS_CODE_UNKNOWN_BTLE_COMMAND);
	T(BLE_HCI_STATUS_CODE_UNKNOWN_CONNECTION_IDENTIFIER);
	T(BLE_HCI_AUTHENTICATION_FAILURE);
	T(BLE_HCI_STATUS_CODE_PIN_OR_KEY_MISSING);
	T(BLE_HCI_MEMORY_CAPACITY_EXCEEDED);
	T(BLE_HCI_CONNECTION_TIMEOUT);
	T(BLE_HCI_STATUS_CODE_COMMAND_DISALLOWED);
	T(BLE_HCI_STATUS_CODE_INVALID_BTLE_COMMAND_PARAMETERS);
	T(BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
	T(BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES);
	T(BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF);
	T(BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION);
	T(BLE_HCI_UNSUPPORTED_REMOTE_FEATURE);
	T(BLE_HCI_STATUS_CODE_INVALID_LMP_PARAMETERS);
	T(BLE_HCI_STATUS_CODE_UNSPECIFIED_ERROR);
	T(BLE_HCI_STATUS_CODE_LMP_RESPONSE_TIMEOUT);
	T(BLE_HCI_STATUS_CODE_LMP_PDU_NOT_ALLOWED);
	T(BLE_HCI_INSTANT_PASSED);
	T(BLE_HCI_PAIRING_WITH_UNIT_KEY_UNSUPPORTED);
	T(BLE_HCI_DIFFERENT_TRANSACTION_COLLISION);
	T(BLE_HCI_CONTROLLER_BUSY);
	T(BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
	T(BLE_HCI_DIRECTED_ADVERTISER_TIMEOUT);
	T(BLE_HCI_CONN_TERMINATED_DUE_TO_MIC_FAILURE);
	T(BLE_HCI_CONN_FAILED_TO_BE_ESTABLISHED);
	T(BLE_ERROR_GATTS_SYS_ATTR_MISSING);
	TN(type);
}

#undef APP_ERROR_HANDLER
#define APP_ERROR_HANDLER(ERR_CODE) app_error_handler_custom((ERR_CODE), __LINE__, (uint8_t*) __FILE__);
void app_error_handler_custom(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
	app_trace_log("ERROR 0x%04x (%s) line: %d\n", (int)error_code, error_name(error_code), (int)line_num);
}

enum {
	MODE_BT = 0,
	MODE_RF = 1
};

static int m_keyboard_mode = 0;

#define printf app_trace_log

//////////////////////////////////////////////////////////////////////////////
// switch code

static uint16_t m_switch_handle = BLE_CONN_HANDLE_INVALID;

ret_code_t switch_whitelist_create(dm_application_instance_t const *p_handle, ble_gap_whitelist_t * p_whitelist);
#define _EEPROMSIZE 12
#define __EEPROM_DATA(a, b, c, d, e, f, g, h) \
    uint8_t eeprom_data[_EEPROMSIZE] = { (a), (b), (c), (d), (e), (f), (g) }
uint8_t eeprom_data[_EEPROMSIZE];

#define SIGNATURE   DEVICE_MANAGER_MAX_BONDS
#define EEPROM_SIZE _EEPROMSIZE

#define nSWITCH_DISABLE_LOGS	// Enable this macro to disable any logs from this module.

#ifndef SWITCH_DISABLE_LOGS
#define SW_LOG  app_trace_log
#else
#define SW_LOG(...)
#endif

typedef struct {
	uint32_t signature;
	dm_device_instance_t devices[DEVICE_MANAGER_MAX_BONDS];
	uint8_t current_index;
	uint8_t eeprom[DEVICE_MANAGER_MAX_BONDS][EEPROM_SIZE];
} switch_context_t;

STATIC_ASSERT(sizeof(switch_context_t) % 4 == 0);

static dm_device_instance_t m_current_device;
static pstorage_handle_t m_storage_handle;
static switch_context_t m_switch_context;

static void switch_pstorage_callback(pstorage_handle_t * handle, uint8_t op_code, uint32_t reason, uint8_t * p_data, uint32_t param_len) {
	if (reason != NRF_SUCCESS)
		SW_LOG("[SW]: switch_pstorage_callback: %lx\n", reason);
}

static uint32_t switch_save(void) {
	uint32_t err_code;
	pstorage_handle_t block_handle;

	err_code = pstorage_block_identifier_get(&m_storage_handle, 0, &block_handle);
	if (err_code == NRF_SUCCESS) {
		err_code = pstorage_update(&block_handle, (uint8_t *) & m_switch_context, sizeof(switch_context_t), 0);
	}
	return err_code;
}

uint32_t switch_init(bool erase_bonds) {
	uint32_t err_code;
	pstorage_module_param_t param;
	pstorage_handle_t block_handle;

	param.block_count = 1;
	param.block_size = sizeof(switch_context_t);
	param.cb = switch_pstorage_callback;
	err_code = pstorage_register(&param, &m_storage_handle);
	APP_ERROR_CHECK(err_code);

	err_code = pstorage_block_identifier_get(&m_storage_handle, 0, &block_handle);
	if (err_code == NRF_SUCCESS) {
		err_code = pstorage_load((uint8_t *) & m_switch_context, &block_handle, sizeof(switch_context_t), 0);
		if (err_code == NRF_SUCCESS && m_switch_context.signature == SIGNATURE && !erase_bonds) {
			if (DEVICE_MANAGER_MAX_BONDS <= m_switch_context.current_index)
				m_switch_context.current_index = 0;
			for (unsigned i = 0; i < DEVICE_MANAGER_MAX_BONDS; ++i) {
				if (DEVICE_MANAGER_MAX_BONDS <= m_switch_context.devices[i])
					m_switch_context.devices[i] = DM_INVALID_ID;
			}
		} else {
			m_switch_context.signature = SIGNATURE;
			memset(m_switch_context.devices, DM_INVALID_ID, DEVICE_MANAGER_MAX_BONDS);
			memmove(m_switch_context.eeprom, eeprom_data, EEPROM_SIZE);
			m_switch_context.current_index = 0;
			err_code = switch_save();
		}
	}

	m_current_device = m_switch_context.devices[m_switch_context.current_index];
	SW_LOG("[SW]: switch_init: %u [sizeof(switch_context_t) = %u]\n", m_switch_context.current_index, sizeof(switch_context_t));
	return err_code;
}

uint32_t switch_select(uint8_t index) {
	if (DEVICE_MANAGER_MAX_BONDS <= index)
		index = 0;
	m_switch_context.current_index = index;
	m_current_device = m_switch_context.devices[index];
	SW_LOG("[SW]: switch_select: %u\n", m_switch_context.current_index);
	return switch_save();
}

uint32_t switch_update(dm_handle_t const *p_handle) {
	if (p_handle->device_id < DEVICE_MANAGER_MAX_BONDS) {
		m_switch_context.devices[m_switch_context.current_index] = p_handle->device_id;
		m_current_device = p_handle->device_id;
	}
	SW_LOG("[SW]: switch_update: %u\n", m_switch_context.current_index);
	return switch_save();
}

uint32_t switch_reset(dm_handle_t const *p_handle) {
	dm_device_delete(p_handle);
	m_switch_context.devices[m_switch_context.current_index] = DM_INVALID_ID;
	m_current_device = DM_INVALID_ID;
	return switch_save();
}

uint8_t switch_get_current(void) {
	return m_switch_context.current_index;
}

uint8_t switch_get_current_device(void) {
	return m_current_device;
}

uint32_t switch_filter(dm_handle_t const *p_handle, dm_event_t const *p_event, uint16_t conn_handle) {
	uint32_t err_code = 0;
	bool disconnect = false;

	switch (p_event->event_id) {
		case DM_EVT_CONNECTION:
			if (p_handle->device_id != DM_INVALID_ID) {
				if (m_current_device != DM_INVALID_ID) {
					if (p_handle->device_id != m_current_device)
						disconnect = true;
				} else {
					for (unsigned i = 0; i < DEVICE_MANAGER_MAX_BONDS; ++i) {
						if (m_switch_context.devices[i] == p_handle->device_id) {
							disconnect = true;
							break;
						}
					}
				}
			}
			if (disconnect) {
				err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
				SW_LOG("[SW]: switch_filter: %02x %02x\n", m_current_device, p_handle->device_id);
				err_code = 1;
			}
			break;
		default:
			break;
	}

	return err_code;
}

uint8_t eeprom_read(uint8_t index) {
	if (EEPROM_SIZE <= index)
		return 0;
	return m_switch_context.eeprom[m_switch_context.current_index][index];
}

void eeprom_write(uint8_t index, uint8_t val) {
	if (EEPROM_SIZE <= index)
		return;
	m_switch_context.eeprom[m_switch_context.current_index][index] = val;
	switch_save();
}

//////////////////////////////////////////////////////////////////////////////

static ble_hids_t m_hids;									/**< Structure used to identify the HID service. */
static ble_bas_t m_bas;										/**< Structure used to identify the battery service. */
static bool m_in_boot_mode = false;							/**< Current protocol mode. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;	/**< Handle of the current connection. */
static dm_application_instance_t m_app_handle;				/**< Application identifier allocated by device manager. */
static dm_handle_t m_bonded_peer_handle;					/**< Device reference handle to the current 	 central. */

//void notification_cb(nrf_impl_notification_t notification);
/*lint -e526 "Symbol RADIO_IRQHandler not defined" */
void RADIO_IRQHandler(void);

#define TX_PAYLOAD_LENGTH   3
#define ACK_PAYLOAD_LENGTH  1

static nrf_radio_request_t m_timeslot_request;
static uint32_t m_slot_length;
static volatile bool m_cmd_received = false;
static volatile bool m_gzll_initialized = false;

static nrf_radio_signal_callback_return_param_t signal_callback_return_param;

void HardFault_Handler(uint32_t program_counter, uint32_t link_register) {
}

void m_configure_next_event(void) {
	m_slot_length = 10000;
	m_timeslot_request.request_type = NRF_RADIO_REQ_TYPE_EARLIEST;
	m_timeslot_request.params.earliest.hfclk = NRF_RADIO_HFCLK_CFG_NO_GUARANTEE;
	m_timeslot_request.params.earliest.priority = NRF_RADIO_PRIORITY_NORMAL;
	m_timeslot_request.params.earliest.length_us = m_slot_length;
	m_timeslot_request.params.earliest.timeout_us = 100000;
}

void sys_evt_dispatch(uint32_t evt_id) {
	uint32_t err_code;

	switch (evt_id) {
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
			//app_trace_log("Blocked\n");
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

	pstorage_sys_event_handler(evt_id);
	ble_advertising_on_sys_evt(evt_id);
}

bool was_pressed = false;
void send_winkey();

static uint8_t data_payload_left[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];	///< Placeholder for data payload received from host.
static uint8_t data_payload_right[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];	///< Placeholder for data payload received from host.

static bool packet_received_left, packet_received_right;
static uint8_t ack_payload[TX_PAYLOAD_LENGTH];	///< Payload to attach to ACK sent to device.
uint32_t left_active = 0;
uint32_t right_active = 0;
void key_handler();

// Key buffers
static uint32_t keys = 0, keys_snapshot = 0;
static uint32_t keys_recv = 0, keys_recv_snapshot = 0;
static uint8_t data_buffer[10];

void m_process_gazelle() {

	// detecting received packet from interupt, and unpacking
	if (packet_received_left) {
		packet_received_left = false;
		data_buffer[0] = ((data_payload_left[0] & 1 << 3) ? 1 : 0) << 0 | ((data_payload_left[0] & 1 << 4) ? 1 : 0) << 1 | ((data_payload_left[0] & 1 << 5) ? 1 : 0) << 2 | ((data_payload_left[0] & 1 << 6) ? 1 : 0) << 3 | ((data_payload_left[0] & 1 << 7) ? 1 : 0) << 4;
		data_buffer[2] = ((data_payload_left[1] & 1 << 6) ? 1 : 0) << 0 | ((data_payload_left[1] & 1 << 7) ? 1 : 0) << 1 | ((data_payload_left[0] & 1 << 0) ? 1 : 0) << 2 | ((data_payload_left[0] & 1 << 1) ? 1 : 0) << 3 | ((data_payload_left[0] & 1 << 2) ? 1 : 0) << 4;
		data_buffer[4] = ((data_payload_left[1] & 1 << 1) ? 1 : 0) << 0 | ((data_payload_left[1] & 1 << 2) ? 1 : 0) << 1 | ((data_payload_left[1] & 1 << 3) ? 1 : 0) << 2 | ((data_payload_left[1] & 1 << 4) ? 1 : 0) << 3 | ((data_payload_left[1] & 1 << 5) ? 1 : 0) << 4;
		data_buffer[6] = ((data_payload_left[2] & 1 << 5) ? 1 : 0) << 1 | ((data_payload_left[2] & 1 << 6) ? 1 : 0) << 2 | ((data_payload_left[2] & 1 << 7) ? 1 : 0) << 3 | ((data_payload_left[1] & 1 << 0) ? 1 : 0) << 4;
		data_buffer[8] = ((data_payload_left[2] & 1 << 1) ? 1 : 0) << 1 | ((data_payload_left[2] & 1 << 2) ? 1 : 0) << 2 | ((data_payload_left[2] & 1 << 3) ? 1 : 0) << 3 | ((data_payload_left[2] & 1 << 4) ? 1 : 0) << 4;
	}

	if (packet_received_right) {
		packet_received_right = false;
		data_buffer[1] = ((data_payload_right[0] & 1 << 7) ? 1 : 0) << 0 | ((data_payload_right[0] & 1 << 6) ? 1 : 0) << 1 | ((data_payload_right[0] & 1 << 5) ? 1 : 0) << 2 | ((data_payload_right[0] & 1 << 4) ? 1 : 0) << 3 | ((data_payload_right[0] & 1 << 3) ? 1 : 0) << 4;
		data_buffer[3] = ((data_payload_right[0] & 1 << 2) ? 1 : 0) << 0 | ((data_payload_right[0] & 1 << 1) ? 1 : 0) << 1 | ((data_payload_right[0] & 1 << 0) ? 1 : 0) << 2 | ((data_payload_right[1] & 1 << 7) ? 1 : 0) << 3 | ((data_payload_right[1] & 1 << 6) ? 1 : 0) << 4;
		data_buffer[5] = ((data_payload_right[1] & 1 << 5) ? 1 : 0) << 0 | ((data_payload_right[1] & 1 << 4) ? 1 : 0) << 1 | ((data_payload_right[1] & 1 << 3) ? 1 : 0) << 2 | ((data_payload_right[1] & 1 << 2) ? 1 : 0) << 3 | ((data_payload_right[1] & 1 << 1) ? 1 : 0) << 4;
		data_buffer[7] = ((data_payload_right[1] & 1 << 0) ? 1 : 0) << 0 | ((data_payload_right[2] & 1 << 7) ? 1 : 0) << 1 | ((data_payload_right[2] & 1 << 6) ? 1 : 0) << 2 | ((data_payload_right[2] & 1 << 5) ? 1 : 0) << 3;
		data_buffer[9] = ((data_payload_right[2] & 1 << 4) ? 1 : 0) << 0 | ((data_payload_right[2] & 1 << 3) ? 1 : 0) << 1 | ((data_payload_right[2] & 1 << 2) ? 1 : 0) << 2 | ((data_payload_right[2] & 1 << 1) ? 1 : 0) << 3;
	}
}


static void m_on_start(void) {
	bool res = false;
	signal_callback_return_param.params.request.p_next = NULL;
	signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;

	nrf_gzll_mode_t mode = m_keyboard_mode == MODE_BT ? NRF_GZLL_MODE_HOST : NRF_GZLL_MODE_DEVICE;

	if (!m_gzll_initialized) {
		// Initialize Gazell
		nrf_gzll_init(mode);

		nrf_gzll_set_device_channel_selection_policy(NRF_GZLL_DEVICE_CHANNEL_SELECTION_POLICY_USE_CURRENT);
		nrf_gzll_set_xosc_ctl(NRF_GZLL_XOSC_CTL_MANUAL);
		nrf_gzll_set_max_tx_attempts(0);	// NB! 100?? reversebias

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
	NRF_TIMER0->CC[0] = m_slot_length - 4000;	// TODO: Use define instead of magic number
	NVIC_EnableIRQ(TIMER0_IRQn);

	(void)res;
}

static void m_on_multitimer(void) {

	NRF_TIMER0->EVENTS_COMPARE[0] = 0;
	if (nrf_gzll_get_mode() != NRF_GZLL_MODE_SUSPEND) {
		signal_callback_return_param.params.request.p_next = NULL;
		signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
		(void)nrf_gzll_set_mode(NRF_GZLL_MODE_SUSPEND);
		NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
		NRF_TIMER0->CC[0] = m_slot_length - 1000;
	} else {
		ASSERT(nrf_gzll_get_mode() == NRF_GZLL_MODE_SUSPEND);
		m_configure_next_event();
		signal_callback_return_param.params.request.p_next = &m_timeslot_request;
		signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
	}
}

nrf_radio_signal_callback_return_param_t *m_radio_callback(uint8_t signal_type) {
	switch (signal_type) {
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

uint32_t gazell_sd_radio_init(void) {

	uint32_t err_code;
	err_code = sd_radio_session_open(m_radio_callback);
	if (err_code != NRF_SUCCESS)
		return err_code;
	m_configure_next_event();
	err_code = sd_radio_request(&m_timeslot_request);
	if (err_code != NRF_SUCCESS) {
		(void)sd_radio_session_close();
		return err_code;
	}
	return NRF_SUCCESS;
}


void nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info) {
	if (m_keyboard_mode == MODE_BT)
		return;
	uint32_t ack_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
	if (tx_info.payload_received_in_ack) {
		// Pop packet and write first byte of the payload to the GPIO port.
		nrf_gzll_fetch_packet_from_rx_fifo(pipe, ack_payload, &ack_payload_length);
	}
}

void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info) {
}

void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info) {
	if (m_keyboard_mode == MODE_RF)
		return;

	uint32_t data_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;

	if (pipe == 0) {
		packet_received_left = true;
		left_active = 0;
		// Pop packet and write first byte of the payload to the GPIO port.
		nrf_gzll_fetch_packet_from_rx_fifo(pipe, data_payload_left, &data_payload_length);
	} else if (pipe == 1) {
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

	m_process_gazelle();
}

void nrf_gzll_disabled(void) {
}

bool debug_cmd_available(void) {
	return m_cmd_received;
}

char get_debug_cmd(void) {
	char cmd = ack_payload[0];
	m_cmd_received = false;
	return cmd;
}

#define VBAT_MAX_IN_MV 3000
uint8_t battery_level_get(void) {
	// Configure ADC
	NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos) | (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) | (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) | (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);

	NRF_ADC->EVENTS_END = 0;
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
	NRF_ADC->EVENTS_END = 0;	// Stop any running conversions.
	NRF_ADC->TASKS_START = 1;

	while (!NRF_ADC->EVENTS_END) {
	}

	uint16_t vbg_in_mv = 1200;
	uint8_t adc_max = 255;
	uint16_t vbat_current_in_mv = (NRF_ADC->RESULT * 3 * vbg_in_mv) / adc_max;

	NRF_ADC->EVENTS_END = 0;
	NRF_ADC->TASKS_STOP = 1;

	int percent = ((vbat_current_in_mv * 100) / VBAT_MAX_IN_MV);
	if (percent > 100)
		percent = 100;

	app_trace_log("Sending BAS report: %d%% (%dmV)\n", percent, vbat_current_in_mv);

	return (uint8_t) percent;
}

#define IS_SRVC_CHANGED_CHARACT_PRESENT		0	/**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/
#define CENTRAL_LINK_COUNT					0	/**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT				1	/**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#define KEY_PRESS_BUTTON_ID					0	/**< Button used as Keyboard key press. */
#define SHIFT_BUTTON_ID						1	/**< Button used as 'SHIFT' Key. */
#define DEVICE_NAME							"Mitosis-BT"	/**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME					"NordicSemiconductor"	/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_TIMER_PRESCALER					0	/**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE				4	/**< Size of timer operation queues. */
#define BATTERY_LEVEL_MEAS_INTERVAL			APP_TIMER_TICKS(5*60*1000, APP_TIMER_PRESCALER)	/**< Battery level measurement interval (ticks). */
#define PNP_ID_VENDOR_ID_SOURCE				0x02	/**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID					0x1915	/**< Vendor ID. */
#define PNP_ID_PRODUCT_ID					0xEEEE	/**< Product ID. */
#define PNP_ID_PRODUCT_VERSION				0x0001	/**< Product Version. */
#define APP_ADV_FAST_INTERVAL				0x0028	/**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL				0x0C80	/**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT				30		/**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT				180		/**< The duration of the slow advertising period (in seconds). */
/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
#define MIN_CONN_INTERVAL					MSEC_TO_UNITS(7.5, UNIT_1_25_MS)	/**< Minimum ion interval (7.5 ms) */
#define MAX_CONN_INTERVAL					MSEC_TO_UNITS(30, UNIT_1_25_MS)	/**< Maximum connection interval (30 ms). */
#define SLAVE_LATENCY						6		/**< Slave latency. */
#define CONN_SUP_TIMEOUT					MSEC_TO_UNITS(430, UNIT_10_MS)	/**< Connection supervisory timeout (430 ms). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY		APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)	/**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY		APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)	/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT		3	/**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND						1	/**< Perform bonding. */
#define SEC_PARAM_MITM						0	/**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC						0	/**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS					0	/**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES			BLE_GAP_IO_CAPS_NONE	/**< No I/O capabilities. */
#define SEC_PARAM_OOB						0	/**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE				7	/**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE				16	/**< Maximum encryption key size. */

#define OUTPUT_REPORT_INDEX					0	/**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN				1	/**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX				0	/**< Index of Input Report. */
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK	0x02	/**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID					0	/**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID					0	/**< Id of reference to Keyboard Output Report. */

#define APP_FEATURE_NOT_SUPPORTED			BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2	/**< Reply when unsupported features are requested. */
#define BASE_USB_HID_SPEC_VERSION			0x0101	/**< Version number of base USB HID Specification implemented by this application. */
#define INPUT_REPORT_KEYS_MAX_LEN			8	/**< Maximum length of the Input Report characteristic. */
#define DEAD_BEEF							0xDEADBEEF				/**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define SCHED_MAX_EVENT_DATA_SIZE			MAX(APP_TIMER_SCHED_EVT_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE)	 /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE					10	/**< Maximum number of events in the scheduler queue. */
#define MODIFIER_KEY_POS					0	/**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS						2	/**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */
#define SHIFT_KEY_CODE						0x02	/**< Key code indicating the press of the Shift Key. */
#define MAX_KEYS_IN_ONE_REPORT				(INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)		/**< Maximum number of key presses that can be sent in one Input Report. */

int biton32(int x) {
	return x;
}

uint8_t m_layer = 0;
static bool m_caps_on = false;

uint8_t get_modifier(uint16_t key) {
	const int modifiers[] = { KC_LCTRL, KC_LSHIFT, KC_LALT, KC_LGUI, KC_RCTRL, KC_RSHIFT, KC_RALT, KC_RGUI };
	for (int b = 0; b < 8; b++)
		if (key == modifiers[b])
			return 1 << b;
	return 0;
}

void key_handler() {
	//app_trace_log("%s %d %d\n", __FUNCTION__, keys, keys_recv);

	if (m_keyboard_mode == MODE_RF)
		return;

	const int MAX_KEYS = 6;
	uint8_t buf[8];
	int modifiers = 0;
	int keys_pressed = 0;
	int keys_sent = 0;
	memset(buf, 0, sizeof(buf));

	for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
		uint16_t val = (uint16_t) data_buffer[row * 2] | (uint16_t) data_buffer[row * 2 + 1] << 5;
		if (val) {
			for (int col = 0; col < 16; col++) {
				if (val & (1 << col)) {
					keys_pressed++;
					uint16_t key = keymaps[m_layer][row][col];
					if (key == KC_TRNS)
						key = keymaps[0][row][col];
					uint8_t modifier = get_modifier(key);
					if (modifier) {
						modifiers |= modifier;
					} else if (key & QK_LAYER_TAP) {
						m_layer = (key >> 8) & 0xf;
					} else if (keys_sent < MAX_KEYS) {
						buf[2 + keys_sent++] = key;
					}
				}
			}
		}
	}

	if (!keys_pressed)
		m_layer = 0;

	buf[0] = modifiers;
	buf[1] = 0;					// reserved

	if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
		printf("Sending HID report: %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		uint32_t err_code = ble_hids_inp_rep_send(&m_hids, INPUT_REPORT_KEYS_INDEX, INPUT_REPORT_KEYS_MAX_LEN, buf);
		APP_ERROR_CHECK(err_code);
	}
}


// Setup switch pins with pullups
static void gpio_config(void) {
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


// Return the key states, masked with valid key pins
static uint32_t read_keys(void) {
//#define FILTER_OUT_UART_PIN_KEY // uncomment on unwanted HID reports
#ifdef FILTER_OUT_UART_PIN_KEY
	uint32_t uart_pin = 1 << TX_PIN_NUMBER;
	return ~NRF_GPIO->IN & INPUT_MASK & ~uart_pin;
#else
	return ~NRF_GPIO->IN & INPUT_MASK;
#endif
}

static void send_data() {
	static uint8_t *data_payload = data_payload_right;

	data_payload[0] = ((keys & 1 << S01) ? 1 : 0) << 7 | ((keys & 1 << S02) ? 1 : 0) << 6 | ((keys & 1 << S03) ? 1 : 0) << 5 | ((keys & 1 << S04) ? 1 : 0) << 4 | ((keys & 1 << S05) ? 1 : 0) << 3 | ((keys & 1 << S06) ? 1 : 0) << 2 | ((keys & 1 << S07) ? 1 : 0) << 1 | ((keys & 1 << S08) ? 1 : 0) << 0;
	data_payload[1] = ((keys & 1 << S09) ? 1 : 0) << 7 | ((keys & 1 << S10) ? 1 : 0) << 6 | ((keys & 1 << S11) ? 1 : 0) << 5 | ((keys & 1 << S12) ? 1 : 0) << 4 | ((keys & 1 << S13) ? 1 : 0) << 3 | ((keys & 1 << S14) ? 1 : 0) << 2 | ((keys & 1 << S15) ? 1 : 0) << 1 | ((keys & 1 << S16) ? 1 : 0) << 0;
	data_payload[2] = ((keys & 1 << S17) ? 1 : 0) << 7 | ((keys & 1 << S18) ? 1 : 0) << 6 | ((keys & 1 << S19) ? 1 : 0) << 5 | ((keys & 1 << S20) ? 1 : 0) << 4 | ((keys & 1 << S21) ? 1 : 0) << 3 | ((keys & 1 << S22) ? 1 : 0) << 2 | ((keys & 1 << S23) ? 1 : 0) << 1 | 0 << 0;

	if (m_keyboard_mode == MODE_RF) {
		nrf_gzll_add_packet_to_tx_fifo(PIPE_NUMBER, data_payload, TX_PAYLOAD_LENGTH);
		return;
	}

	data_buffer[1] = ((data_payload_right[0] & 1 << 7) ? 1 : 0) << 0 | ((data_payload_right[0] & 1 << 6) ? 1 : 0) << 1 | ((data_payload_right[0] & 1 << 5) ? 1 : 0) << 2 | ((data_payload_right[0] & 1 << 4) ? 1 : 0) << 3 | ((data_payload_right[0] & 1 << 3) ? 1 : 0) << 4;
	data_buffer[3] = ((data_payload_right[0] & 1 << 2) ? 1 : 0) << 0 | ((data_payload_right[0] & 1 << 1) ? 1 : 0) << 1 | ((data_payload_right[0] & 1 << 0) ? 1 : 0) << 2 | ((data_payload_right[1] & 1 << 7) ? 1 : 0) << 3 | ((data_payload_right[1] & 1 << 6) ? 1 : 0) << 4;
	data_buffer[5] = ((data_payload_right[1] & 1 << 5) ? 1 : 0) << 0 | ((data_payload_right[1] & 1 << 4) ? 1 : 0) << 1 | ((data_payload_right[1] & 1 << 3) ? 1 : 0) << 2 | ((data_payload_right[1] & 1 << 2) ? 1 : 0) << 3 | ((data_payload_right[1] & 1 << 1) ? 1 : 0) << 4;
	data_buffer[7] = ((data_payload_right[1] & 1 << 0) ? 1 : 0) << 0 | ((data_payload_right[2] & 1 << 7) ? 1 : 0) << 1 | ((data_payload_right[2] & 1 << 6) ? 1 : 0) << 2 | ((data_payload_right[2] & 1 << 5) ? 1 : 0) << 3;
	data_buffer[9] = ((data_payload_right[2] & 1 << 4) ? 1 : 0) << 0 | ((data_payload_right[2] & 1 << 3) ? 1 : 0) << 1 | ((data_payload_right[2] & 1 << 2) ? 1 : 0) << 2 | ((data_payload_right[2] & 1 << 1) ? 1 : 0) << 3;
}


#define DEBOUNCE 25				// standard 25 ms refresh
#define ACTIVITY 4*60*1000		// unactivity time till power off (4 minutes)
#define REBOOT 6*1000			// hold reboot for a few seconds
#define PAIRING 2*1000			// hold switch key for a few seconds
#define DEBOUNCE_MEAS_INTERVAL APP_TIMER_TICKS(DEBOUNCE, APP_TIMER_PRESCALER)
static uint32_t activity_ticks = 0;
static uint32_t reset_ticks = 0;
static uint32_t pairing_ticks = 0;

static uint32_t advertising_restart(ble_adv_mode_t mode) {
	uint32_t err_code;

	if (m_conn_handle == BLE_CONN_HANDLE_INVALID) {
		sd_ble_gap_adv_stop();
		err_code = ble_advertising_start(mode);
	} else {
		err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
	}
	return err_code;
}

// former 1000Hz debounce sampling - it's 25 ms now. do we still need debouncing? seem to work fine
static void handler_debounce(void *p_context) {

	// right half
	keys_snapshot = read_keys();
	if (keys != keys_snapshot) {
		keys = keys_snapshot;
		send_data();
		key_handler();

		if (m_keyboard_mode == MODE_BT && keys == (1 << KEY_RF)) {	// menu key
			m_switch_context.current_index = (m_switch_context.current_index + 1) % (MAX_DEVICES - 1);
			switch_select(m_switch_context.current_index);
			advertising_restart(BLE_ADV_MODE_FAST);
		}
	}
	// left half
	keys_recv = data_payload_left[0] | (data_payload_left[1] << 8) | (data_payload_left[2] << 16);
	if (keys_recv != keys_recv_snapshot) {
		keys_recv_snapshot = keys_recv;
		key_handler();
	}

	if (keys == 0 && keys_recv == 0) {
		activity_ticks++;
		if (activity_ticks * DEBOUNCE > ACTIVITY) {
			app_trace_log("Shutting down on inactivity...\n");
			nrf_delay_ms(50);
			sd_power_system_off();
		}
	} else {
		activity_ticks = 0;
	}

	if (keys == (1 << KEY_BT) || keys == (1 << KEY_RF)) {	// hardware keys
		reset_ticks++;
		if (reset_ticks * DEBOUNCE > REBOOT) {
			reset_ticks = 0;
			app_trace_log("Resetting...\n");
			nrf_delay_ms(50);
			sd_power_system_off();
		}
	} else {
		reset_ticks = 0;
	}

	if (keys == (1 << KEY_RF)) {	// hardware keys
		pairing_ticks++;
		if (pairing_ticks * DEBOUNCE > PAIRING) {
			pairing_ticks = 0;
			app_trace_log("Pairing...\n");
			switch_reset(&m_bonded_peer_handle);
			advertising_restart(BLE_ADV_MODE_FAST);
		}
	} else {
		pairing_ticks = 0;
	}
}

typedef enum {
	BLE_NO_ADV,				/**< No advertising running. */
	BLE_DIRECTED_ADV,		/**< Direct advertising to the latest central. */
	BLE_FAST_ADV_WHITELIST,	/**< Advertising with whitelist. */
	BLE_FAST_ADV,			/**< Fast advertising running. */
	BLE_SLOW_ADV,			/**< Slow advertising running. */
	BLE_SLEEP,				/**< Go to system-off. */
} ble_advertising_mode_t;

APP_TIMER_DEF(m_battery_timer_id); /**< Battery timer. */
APP_TIMER_DEF(m_debounce_timer_id);

static ble_uuid_t m_adv_uuids[] = { {BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE} };

static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
	app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


static void service_error_handler(uint32_t nrf_error) {
	APP_ERROR_HANDLER(nrf_error);
}


static void ble_advertising_error_handler(uint32_t nrf_error) {
	APP_ERROR_HANDLER(nrf_error);
}


static void battery_level_update(void) {
	uint32_t err_code;
	uint8_t battery_level;

	battery_level = battery_level_get();

	err_code = ble_bas_battery_level_update(&m_bas, battery_level);
	if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE)
		&& (err_code != BLE_ERROR_NO_TX_PACKETS)
		&& (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
		APP_ERROR_HANDLER(err_code);
	}
}


static void battery_level_meas_timeout_handler(void *p_context) {
	UNUSED_PARAMETER(p_context);
	battery_level_update();
}


static void timers_init(void) {
	uint32_t err_code;

	// Initialize timer module, making it use the scheduler.
	APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

	// Create battery timer.
	err_code = app_timer_create(&m_battery_timer_id, APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&m_debounce_timer_id, APP_TIMER_MODE_REPEATED, handler_debounce);
	APP_ERROR_CHECK(err_code);

}


static void gap_params_init(void) {
	uint32_t err_code;
	ble_gap_conn_params_t gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
	APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD);
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}


static void dis_init(void) {
	uint32_t err_code;
	ble_dis_init_t dis_init_obj;
	ble_dis_pnp_id_t pnp_id;

	pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
	pnp_id.vendor_id = PNP_ID_VENDOR_ID;
	pnp_id.product_id = PNP_ID_PRODUCT_ID;
	pnp_id.product_version = PNP_ID_PRODUCT_VERSION;

	memset(&dis_init_obj, 0, sizeof(dis_init_obj));

	ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
	dis_init_obj.p_pnp_id = &pnp_id;

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init_obj);
	APP_ERROR_CHECK(err_code);
}


static void bas_init(void) {
	uint32_t err_code;
	ble_bas_init_t bas_init_obj;

	memset(&bas_init_obj, 0, sizeof(bas_init_obj));

	bas_init_obj.evt_handler = NULL;
	bas_init_obj.support_notification = true;
	bas_init_obj.p_report_ref = NULL;
	bas_init_obj.initial_batt_level = 100;

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_report_read_perm);

	err_code = ble_bas_init(&m_bas, &bas_init_obj);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing HID Service.
 */
static void hids_init(void) {
	uint32_t err_code;
	ble_hids_init_t hids_init_obj;
	ble_hids_inp_rep_init_t input_report_array[1];
	ble_hids_inp_rep_init_t *p_input_report;
	ble_hids_outp_rep_init_t output_report_array[1];
	ble_hids_outp_rep_init_t *p_output_report;
	uint8_t hid_info_flags;

	memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
	memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

	static uint8_t report_map_data[] = {
		0x05, 0x01,				// Usage Page (Generic Desktop)
		0x09, 0x06,				// Usage (Keyboard)
		0xA1, 0x01,				// Collection (Application)
		0x05, 0x07,				// Usage Page (Key Codes)
		0x19, 0xe0,				// Usage Minimum (224)
		0x29, 0xe7,				// Usage Maximum (231)
		0x15, 0x00,				// Logical Minimum (0)
		0x25, 0x01,				// Logical Maximum (1)
		0x75, 0x01,				// Report Size (1)
		0x95, 0x08,				// Report Count (8)
		0x81, 0x02,				// Input (Data, Variable, Absolute)

		0x95, 0x01,				// Report Count (1)
		0x75, 0x08,				// Report Size (8)
		0x81, 0x01,				// Input (Constant) reserved byte(1)

		0x95, 0x05,				// Report Count (5)
		0x75, 0x01,				// Report Size (1)
		0x05, 0x08,				// Usage Page (Page# for LEDs)
		0x19, 0x01,				// Usage Minimum (1)
		0x29, 0x05,				// Usage Maximum (5)
		0x91, 0x02,				// Output (Data, Variable, Absolute), Led report
		0x95, 0x01,				// Report Count (1)
		0x75, 0x03,				// Report Size (3)
		0x91, 0x01,				// Output (Data, Variable, Absolute), Led report padding

		0x95, 0x06,				// Report Count (6)
		0x75, 0x08,				// Report Size (8)
		0x15, 0x00,				// Logical Minimum (0)
		0x25, 0x65,				// Logical Maximum (101)
		0x05, 0x07,				// Usage Page (Key codes)
		0x19, 0x00,				// Usage Minimum (0)
		0x29, 0x65,				// Usage Maximum (101)
		0x81, 0x00,				// Input (Data, Array) Key array(6 bytes)

		0x09, 0x05,				// Usage (Vendor Defined)
		0x15, 0x00,				// Logical Minimum (0)
		0x26, 0xFF, 0x00,		// Logical Maximum (255)
		0x75, 0x08,				// Report Count (2)
		0x95, 0x02,				// Report Size (8 bit)
		0xB1, 0x02,				// Feature (Data, Variable, Absolute)

		0xC0					// End Collection (Application)
	};

	// Initialize HID Service
	p_input_report = &input_report_array[INPUT_REPORT_KEYS_INDEX];
	p_input_report->max_len = INPUT_REPORT_KEYS_MAX_LEN;
	p_input_report->rep_ref.report_id = INPUT_REP_REF_ID;
	p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

	p_output_report = &output_report_array[OUTPUT_REPORT_INDEX];
	p_output_report->max_len = OUTPUT_REPORT_MAX_LEN;
	p_output_report->rep_ref.report_id = OUTPUT_REP_REF_ID;
	p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.write_perm);

	hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

	memset(&hids_init_obj, 0, sizeof(hids_init_obj));

	hids_init_obj.evt_handler = on_hids_evt;
	hids_init_obj.error_handler = service_error_handler;
	hids_init_obj.is_kb = true;
	hids_init_obj.is_mouse = false;
	hids_init_obj.inp_rep_count = 1;
	hids_init_obj.p_inp_rep_array = input_report_array;
	hids_init_obj.outp_rep_count = 1;
	hids_init_obj.p_outp_rep_array = output_report_array;
	hids_init_obj.feature_rep_count = 0;
	hids_init_obj.p_feature_rep_array = NULL;
	hids_init_obj.rep_map.data_len = sizeof(report_map_data);
	hids_init_obj.rep_map.p_data = report_map_data;
	hids_init_obj.hid_information.bcd_hid = BASE_USB_HID_SPEC_VERSION;
	hids_init_obj.hid_information.b_country_code = 0;
	hids_init_obj.hid_information.flags = hid_info_flags;
	hids_init_obj.included_services_count = 0;
	hids_init_obj.p_included_services_array = NULL;

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_boot_kb_inp_rep.write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.write_perm);

	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

	err_code = ble_hids_init(&m_hids, &hids_init_obj);
	APP_ERROR_CHECK(err_code);
}


static void services_init(void) {
	dis_init();
	bas_init();
	hids_init();
}


static void conn_params_error_handler(uint32_t nrf_error) {
	APP_ERROR_HANDLER(nrf_error);
}


static void conn_params_init(void) {
	uint32_t err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
	cp_init.disconnect_on_fail = false;
	cp_init.evt_handler = NULL;
	cp_init.error_handler = conn_params_error_handler;

	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
}


static void timers_start(void) {
	uint32_t err_code;

	err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_debounce_timer_id, DEBOUNCE_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
}


static void on_hid_rep_char_write(ble_hids_evt_t * p_evt) {
	if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
		uint32_t err_code;
		uint8_t report_val;
		uint8_t report_index = p_evt->params.char_write.char_id.rep_index;

		if (report_index == OUTPUT_REPORT_INDEX) {
			// This code assumes that the outptu report is one byte long. Hence the following
			// static assert is made.
			STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

			err_code = ble_hids_outp_rep_get(&m_hids, report_index, OUTPUT_REPORT_MAX_LEN, 0, &report_val);
			APP_ERROR_CHECK(err_code);

			if (!m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0)) {
				APP_ERROR_CHECK(err_code);
				m_caps_on = true;
			} else if (m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) == 0)) {
				APP_ERROR_CHECK(err_code);
				m_caps_on = false;
			} else {
				// The report received is not supported by this application. Do nothing.
			}
		}
	}
}


static void sleep_mode_enter(void) {
	uint32_t err_code;
	// Go to system-off mode (this function will not return; wakeup will cause a reset).
	err_code = sd_power_system_off();
	APP_ERROR_CHECK(err_code);
}


static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt) {

	switch (p_evt->evt_type) {
		case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
			m_in_boot_mode = true;
			break;

		case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
			m_in_boot_mode = false;
			break;

		case BLE_HIDS_EVT_REP_CHAR_WRITE:
			on_hid_rep_char_write(p_evt);
			break;

		case BLE_HIDS_EVT_NOTIF_ENABLED:
		{
			dm_service_context_t service_context;
			service_context.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
			service_context.context_data.len = 0;
			service_context.context_data.p_data = NULL;

			if (m_in_boot_mode) {
				// Protocol mode is Boot Protocol mode.
				if (p_evt->params.notification.char_id.uuid == BLE_UUID_BOOT_KEYBOARD_INPUT_REPORT_CHAR) {
					// The notification of boot keyboard input report has been enabled.
					// Save the system attribute (CCCD) information into the flash.
					uint32_t err_code;

					err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
					if (err_code != NRF_ERROR_INVALID_STATE) {
						APP_ERROR_CHECK(err_code);
					} else {
						// The system attributes could not be written to the flash because
						// the connected central is not a new central. The system attributes
						// will only be written to flash only when disconnected from this central.
						// Do nothing now.
					}

				} else {
					// Do nothing.
				}

			} else if (p_evt->params.notification.char_id.rep_type == BLE_HIDS_REP_TYPE_INPUT) {
				// The protocol mode is Report Protocol mode. And the CCCD for the input report
				// is changed. It is now time to store all the CCCD information (system
				// attributes) into the flash.
				uint32_t err_code;

				err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
				if (err_code != NRF_ERROR_INVALID_STATE) {
					APP_ERROR_CHECK(err_code);
				} else {
					// The system attributes could not be written to the flash because
					// the connected central is not a new central. The system attributes
					// will only be written to flash only when disconnected from this central.
					// Do nothing now.
				}
			} else {
				// The notification of the report that was enabled by the central is not interesting
				// to this application. So do nothing.
			}
			break;
		}

		default:
			// No implementation needed.
			break;
	}
}


static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
	uint32_t err_code;

	switch (ble_adv_evt) {
		case BLE_ADV_EVT_DIRECTED:
			app_trace_log("Directed advertising...\n");
			break;
		case BLE_ADV_EVT_FAST:
			app_trace_log("Fast advertising...\n");
			break;
		case BLE_ADV_EVT_SLOW:
			app_trace_log("Slow advertising...\n");
			break;
		case BLE_ADV_EVT_FAST_WHITELIST:
			app_trace_log("Fast advertising with whitelist...\n");
			break;
		case BLE_ADV_EVT_SLOW_WHITELIST:
			app_trace_log("Slow advertising without whitelist...\n");
			//err_code = ble_advertising_restart_without_whitelist();
			//APP_ERROR_CHECK(err_code);
			break;
		case BLE_ADV_EVT_IDLE:
			sleep_mode_enter();
			break;

		case BLE_ADV_EVT_WHITELIST_REQUEST:
		{
			ble_gap_whitelist_t whitelist;
			ble_gap_addr_t *p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
			ble_gap_irk_t *p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

			whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
			whitelist.irk_count = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
			whitelist.pp_addrs = p_whitelist_addr;
			whitelist.pp_irks = p_whitelist_irk;

			err_code = switch_whitelist_create(&m_app_handle, &whitelist);
			//err_code = dm_whitelist_create(&m_app_handle, &whitelist);
			APP_ERROR_CHECK(err_code);

			err_code = ble_advertising_whitelist_reply(&whitelist);
			APP_ERROR_CHECK(err_code);
			break;
		}
		case BLE_ADV_EVT_PEER_ADDR_REQUEST:
		{
			ble_gap_addr_t peer_address;
			// Only Give peer address if we have a handle to the bonded peer.
			if (m_bonded_peer_handle.appl_id != DM_INVALID_ID) {

				err_code = dm_peer_addr_get(&m_bonded_peer_handle, &peer_address);
				APP_ERROR_CHECK(err_code);

				err_code = ble_advertising_peer_addr_reply(&peer_address);
				APP_ERROR_CHECK(err_code);

				/*
				   if (err_code != (NRF_ERROR_NOT_FOUND | DEVICE_MANAGER_ERR_BASE)) {
				   APP_ERROR_CHECK(err_code);
				   err_code = ble_advertising_peer_addr_reply(&peer_address);
				   APP_ERROR_CHECK(err_code);
				   printf("Address requested: " ADDR_FMT "\n", ADDR_T(peer_address.addr));
				   }
				 *///NB!

			}
			break;
		}
		default:
			break;
	}
}


static void on_ble_evt(ble_evt_t * p_ble_evt) {
	uint32_t err_code;
	ble_gatts_rw_authorize_reply_params_t auth_reply;
	ble_gap_addr_t *p_addr;

	switch (p_ble_evt->header.evt_id) {

		case BLE_GAP_EVT_CONNECTED:
			m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			p_addr = &p_ble_evt->evt.gap_evt.params.connected.peer_addr;
			app_trace_log("Connected to " ADDR_FMT "\n", ADDR_T(p_addr->addr));
			//battery_level_update();
			break;

		case BLE_EVT_TX_COMPLETE:
			break;

		case BLE_GAP_EVT_DISCONNECTED:
		{
			m_conn_handle = BLE_CONN_HANDLE_INVALID;

			app_trace_log("Disconnected\n");

			switch (p_ble_evt->evt.gap_evt.params.disconnected.reason) {
				case BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF:
					// do nothing, wait for the host
					break;
				case BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION:
					// see https://devzone.nordicsemi.com/f/nordic-q-a/9506/bond-deletion-notification
					/*
					   app_trace_log("You unpaired us, we unpaired you!\n");
					   dm_device_delete(&m_bonded_peer_handle);
					   ble_advertising_start(BLE_ADV_MODE_FAST);
					   ble_advertising_restart_without_whitelist();
					 */
					break;
			}
			m_caps_on = false;
			break;
		}

		case BLE_EVT_USER_MEM_REQUEST:
			err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			if (p_ble_evt->evt.gatts_evt.params.authorize_request.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
				if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)
					|| (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
					|| (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
					if (p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
						auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
					} else {
						auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
					}
					auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
					err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle, &auth_reply);
					APP_ERROR_CHECK(err_code);
				}
			}
			break;

		case BLE_GATTC_EVT_TIMEOUT:
		case BLE_GATTS_EVT_TIMEOUT:
			app_trace_log("Remote user terminated connection\n");
			// Disconnect on GATT Server and Client timeout events.
			err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
			// No system attributes have been stored.
			//err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
			//APP_ERROR_CHECK(err_code);
			break;

		default:
			// No implementation needed.
			break;
	}
}


static void ble_evt_dispatch(ble_evt_t * p_ble_evt) {

	if (BLE_GAP_EVT_BASE <= p_ble_evt->header.evt_id && p_ble_evt->header.evt_id <= BLE_GAP_EVT_LAST)
		m_switch_handle = p_ble_evt->evt.gap_evt.conn_handle;

	dm_ble_evt_handler(p_ble_evt);
	on_ble_evt(p_ble_evt);
	ble_advertising_on_ble_evt(p_ble_evt);
	ble_conn_params_on_ble_evt(p_ble_evt);
	ble_hids_on_ble_evt(&m_hids, p_ble_evt);
	ble_bas_on_ble_evt(&m_bas, p_ble_evt);
}

static void ble_stack_init(void) {
	uint32_t err_code;

	nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

	// Initialize the SoftDevice handler module.
	SOFTDEVICE_HANDLER_APPSH_INIT(&clock_lf_cfg, true);

/*
    // Enable BLE stack
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#ifdef S130
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params, NULL);
    APP_ERROR_CHECK(err_code);
*/

	ble_enable_params_t ble_enable_params;
	err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
	APP_ERROR_CHECK(err_code);

	//Check the ram settings against the used number of links
	CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

	// Enable BLE stack.
	err_code = softdevice_enable(&ble_enable_params);
	APP_ERROR_CHECK(err_code);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
	APP_ERROR_CHECK(err_code);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
	APP_ERROR_CHECK(err_code);
}


static void scheduler_init(void) {
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

static void advertising_init(void) {
	uint32_t err_code;
	uint8_t adv_flags;
	ble_advdata_t advdata;

	// Build and set advertising data
	memset(&advdata, 0, sizeof(advdata));

	adv_flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
	advdata.name_type = BLE_ADVDATA_FULL_NAME;
	advdata.include_appearance = true;
	advdata.flags = adv_flags;
	advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
	advdata.uuids_complete.p_uuids = m_adv_uuids;

	ble_adv_modes_config_t options = {
		BLE_ADV_WHITELIST_ENABLED,
		BLE_ADV_DIRECTED_DISABLED,
		BLE_ADV_DIRECTED_SLOW_DISABLED, 0, 0,
		BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
		BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
	};

	err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
	APP_ERROR_CHECK(err_code);
}


static uint32_t device_manager_evt_handler(dm_handle_t const *p_handle, dm_event_t const *p_event, ret_code_t event_result) {
	APP_ERROR_CHECK(event_result);

	//SW_LOG("device_manager_evt_handler: %x %x\n", p_event->event_id, p_handle->device_id);

	if (switch_filter(p_handle, p_event, m_switch_handle))
		return NRF_SUCCESS;

	switch (p_event->event_id) {
		case DM_EVT_DEVICE_CONTEXT_LOADED:	// Fall through.
		case DM_EVT_SECURITY_SETUP_COMPLETE:
			m_bonded_peer_handle = (*p_handle);
			break;
		case DM_EVT_LINK_SECURED:
			switch_update(p_handle);
			break;
	}

	return NRF_SUCCESS;
}


static void device_manager_init(bool erase_bonds) {
	uint32_t err_code;
	dm_init_param_t init_param = {.clear_persistent_data = erase_bonds };
	dm_application_param_t register_param;

	// Initialize peer device handle.
	err_code = dm_handle_initialize(&m_bonded_peer_handle);
	APP_ERROR_CHECK(err_code);

	// Initialize persistent storage module.
	err_code = pstorage_init();
	APP_ERROR_CHECK(err_code);

	err_code = dm_init(&init_param);
	APP_ERROR_CHECK(err_code);

	memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

	register_param.sec_param.bond = SEC_PARAM_BOND;
	register_param.sec_param.mitm = SEC_PARAM_MITM;
	register_param.sec_param.lesc = SEC_PARAM_LESC;
	register_param.sec_param.keypress = SEC_PARAM_KEYPRESS;
	register_param.sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
	register_param.sec_param.oob = SEC_PARAM_OOB;
	register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
	register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
	register_param.evt_handler = device_manager_evt_handler;
	register_param.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

	err_code = dm_register(&m_app_handle, &register_param);
	APP_ERROR_CHECK(err_code);

	err_code = switch_init(erase_bonds);
	APP_ERROR_CHECK(err_code);

	if (m_switch_context.current_index == MAX_DEVICES - 1) {
		m_keyboard_mode = MODE_RF;
	}

	if (erase_bonds) {
		m_keyboard_mode = MODE_BT;
		switch_select(0);
	}
}


static void buttons_leds_init(bool * p_erase_bonds) {
	gpio_config();

	nrf_gpio_cfg_output(LED_PIN);
	for (int i = 0; i < 3; i++) {
		nrf_gpio_pin_set(LED_PIN);
		nrf_delay_ms(100);
		nrf_gpio_pin_clear(LED_PIN);
		nrf_delay_ms(100);
	}

	sd_power_dcdc_mode_set(NRF_POWER_DCDC_DISABLE);

	*p_erase_bonds = ((read_keys() & (1 << KEY_BT))) ? 1 : 0;

	if (read_keys() == (1 << KEY_RF)) {
		m_keyboard_mode = MODE_RF;
		switch_select(MAX_DEVICES - 1);
	}
}


static void power_manage(void) {
	uint32_t err_code = sd_app_evt_wait();
	APP_ERROR_CHECK(err_code);
}


int main(void) {
	bool erase_bonds;
	uint32_t err_code;

	// Initialize.
	app_trace_init();
	timers_init();
	buttons_leds_init(&erase_bonds);
	ble_stack_init();
	scheduler_init();
	device_manager_init(erase_bonds);
	gap_params_init();
	advertising_init();
	services_init();
	conn_params_init();
	gazell_sd_radio_init();

	// Start execution.
	timers_start();

	app_trace_log("Started (erase_bonds: %d, device: %d)\n", erase_bonds, m_switch_context.current_index);

	if (m_switch_context.current_index == MAX_DEVICES - 1) {
		m_keyboard_mode = MODE_RF;
	}

	app_trace_log("Keyboard mode: %s\n", m_keyboard_mode == MODE_RF ? "RECEIVER" : "BLUETOOTH");

	if (m_keyboard_mode == MODE_BT) {
		err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
		APP_ERROR_CHECK(err_code);
	}

	// Enter main loop.
	for (;;) {
		app_sched_execute();
		power_manage();
	}
}
