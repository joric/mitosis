#include "nrf_drv_twi.h"

#define SCL_PIN 28
#define SDA_PIN 29

#define SCREEN_W 128
#define SCREEN_H 32

#define SSD1306_I2C_ADDRESS 0x3c

#define SSD1306_128_32
#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 32

static const uint8_t ASCII[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x14,
	0x7f, 0x14, 0x7f, 0x14, 0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62, 0x36, 0x49,
	0x55, 0x22, 0x50, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x41, 0x22,
	0x1c, 0x00, 0x14, 0x08, 0x3e, 0x08, 0x14, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x50, 0x30, 0x00,
	0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x60, 0x60, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02,
	0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x21,
	0x41, 0x45, 0x4b, 0x31, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x27, 0x45, 0x45, 0x45, 0x39, 0x3c, 0x4a,
	0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03, 0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49,
	0x29, 0x1e, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x08, 0x14, 0x22, 0x41,
	0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x51, 0x09, 0x06,
	0x32, 0x49, 0x79, 0x41, 0x3e, 0x7e, 0x11, 0x11, 0x11, 0x7e, 0x7f, 0x49, 0x49, 0x49, 0x36, 0x3e,
	0x41, 0x41, 0x41, 0x22, 0x7f, 0x41, 0x41, 0x22, 0x1c, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x7f, 0x09,
	0x09, 0x09, 0x01, 0x3e, 0x41, 0x49, 0x49, 0x7a, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00, 0x41, 0x7f,
	0x41, 0x00, 0x20, 0x40, 0x41, 0x3f, 0x01, 0x7f, 0x08, 0x14, 0x22, 0x41, 0x7f, 0x40, 0x40, 0x40,
	0x40, 0x7f, 0x02, 0x0c, 0x02, 0x7f, 0x7f, 0x04, 0x08, 0x10, 0x7f, 0x3e, 0x41, 0x41, 0x41, 0x3e,
	0x7f, 0x09, 0x09, 0x09, 0x06, 0x3e, 0x41, 0x51, 0x21, 0x5e, 0x7f, 0x09, 0x19, 0x29, 0x46, 0x46,
	0x49, 0x49, 0x49, 0x31, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x3f, 0x40, 0x40, 0x40, 0x3f, 0x1f, 0x20,
	0x40, 0x20, 0x1f, 0x3f, 0x40, 0x38, 0x40, 0x3f, 0x63, 0x14, 0x08, 0x14, 0x63, 0x07, 0x08, 0x70,
	0x08, 0x07, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x7f, 0x41, 0x41, 0x00, 0x02, 0x04, 0x08, 0x10,
	0x20, 0x00, 0x41, 0x41, 0x7f, 0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x00, 0x01, 0x02, 0x04, 0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x7f, 0x48, 0x44, 0x44, 0x38, 0x38,
	0x44, 0x44, 0x44, 0x20, 0x38, 0x44, 0x44, 0x48, 0x7f, 0x38, 0x54, 0x54, 0x54, 0x18, 0x08, 0x7e,
	0x09, 0x01, 0x02, 0x0c, 0x52, 0x52, 0x52, 0x3e, 0x7f, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7d,
	0x40, 0x00, 0x20, 0x40, 0x44, 0x3d, 0x00, 0x7f, 0x10, 0x28, 0x44, 0x00, 0x00, 0x41, 0x7f, 0x40,
	0x00, 0x7c, 0x04, 0x18, 0x04, 0x78, 0x7c, 0x08, 0x04, 0x04, 0x78, 0x38, 0x44, 0x44, 0x44, 0x38,
	0x7c, 0x14, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x18, 0x7c, 0x7c, 0x08, 0x04, 0x04, 0x08, 0x48,
	0x54, 0x54, 0x54, 0x20, 0x04, 0x3f, 0x44, 0x40, 0x20, 0x3c, 0x40, 0x40, 0x20, 0x7c, 0x1c, 0x20,
	0x40, 0x20, 0x1c, 0x3c, 0x40, 0x30, 0x40, 0x3c, 0x44, 0x28, 0x10, 0x28, 0x44, 0x0c, 0x50, 0x50,
	0x50, 0x3c, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x7f, 0x00,
	0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x10, 0x08, 0x08, 0x10, 0x08, 0x78, 0x46, 0x41, 0x46, 0x78
};

uint8_t buf[SCREEN_W * SCREEN_H / 8] PROGMEM;

void oledWriteCharacter (char c, uint8_t ** pp) {
	uint8_t *p = *pp;
	const uint8_t * src = ASCII + (c - 0x20)*5;
	for (int i = 0; i < 5; i++)
		*p++ = *src++;
	*p++ = 0x00; // 1px space
	*pp = p;
}

void oledWriteString (const char *s, int x, int y) {
	uint8_t *p = buf + y * SCREEN_W / 8 + x;
	while (*s)
		oledWriteCharacter(*s++, &p);
}


static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(1);
static uint16_t send_count = 0;
static volatile bool twi_evt_done = false;	// volatile works omg

void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context) {
	if (p_event->type == NRF_DRV_TWI_EVT_DONE) {
		send_count++;
		twi_evt_done = true;
	}
}

void ssd1306_command(uint8_t c) {
	uint8_t dta_send[] = { 0x00, c };
	if (!nrf_drv_twi_tx(&m_twi, SSD1306_I2C_ADDRESS, dta_send, 2, false)) {
		while (twi_evt_done != true);
		twi_evt_done = false;
	} else {
		nrf_delay_ms(10);
	}
}

void cmd(uint8_t b, ...) {
	va_list v;
	va_start(v, b);
	ssd1306_command(b);
	for (;;) {
		uint8_t c = va_arg(v, int);
		if (c == 0xff)
			break;
		ssd1306_command(c);
	}
}

void ssd1306_write(uint8_t c) {
	ret_code_t ret;
	ret = nrf_drv_twi_tx(&m_twi, SSD1306_I2C_ADDRESS, &c, 1, true);

        if (ret != 0) {
                printf("ssd1306 send data failed err = %d\r\n", ret);
                nrf_delay_ms(10);
        } else {
                while (twi_evt_done != true);
                twi_evt_done = false;
        }

        UNUSED_VARIABLE(ret);
}

void ssd1306_data(uint8_t data){
        ret_code_t ret;
        uint8_t dta_send[] = {0x40, data};
        ret = nrf_drv_twi_tx(&m_twi, SSD1306_I2C_ADDRESS, dta_send, 2, false);
        if (ret != 0) {
                printf("ssd1306 send data %02X failed err = %d\r\n", data, ret);
                nrf_delay_ms(10);
        } else {
                while (twi_evt_done != true);
                twi_evt_done = false;
        }

        UNUSED_VARIABLE(ret);
}


void Oled_DrawArea(int x, int y, int w, int h, uint8_t * buf) {

	send_count = 0;

		cmd(0x21, 0, SCREEN_W - 1, 0x22, 0, SCREEN_H==64 ? 7 : SCREEN_H==32 ? 3 : 1, 0xff);

#if 1
        for (int i = 0; i < w * h / 8; i++) {
                ssd1306_data(buf[i]);
        }
#else
	static uint8_t control = 0x40;
	nrf_drv_twi_tx(&m_twi, SSD1306_I2C_ADDRESS, &control, 1, true);
	nrf_drv_twi_tx(&m_twi, SSD1306_I2C_ADDRESS, buf, w * h / 8, false);

	// ^ does not work
#endif


	//printf("send_count: %d\n", send_count);

}

void display_init() {
	//printf("%s\n", __FUNCTION__);

	nrf_drv_twi_config_t twi_config = {
		.scl = SCL_PIN,
		.sda = SDA_PIN,
		.frequency = NRF_TWI_FREQ_100K,
		.interrupt_priority = APP_IRQ_PRIORITY_HIGH
	};

	nrf_drv_twi_init(&m_twi, &twi_config, twi_handler, NULL);
	nrf_drv_twi_enable(&m_twi);

	cmd(0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x02, 0x81, 0x8F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0x2E, 0xAF, -1);

	for (int i = 0; i < SCREEN_W * SCREEN_H / 8; i++) {
		buf[i] = 0x00;
	}

	oledWriteString("Mitosis-BT2", 0, 0);
	oledWriteString(" Main menu", 0, 16);
	oledWriteString(">Connected devices 4", 0, 24);

	Oled_DrawArea(0, 0, SCREEN_W, SCREEN_H, buf);
}

static bool m_display_init = false;
static int m_display_counter = 0;
void display_update() {

	for (int i = 0; i < SCREEN_W * SCREEN_H / 8; i++) {
		buf[i] = m_display_counter % 0xff;
	}

	//Oled_DrawArea(0, 0, 4, 8, buf);

	m_display_counter++;
}


