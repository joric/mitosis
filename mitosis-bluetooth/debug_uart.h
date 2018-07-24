#include "app_uart.h"

#ifndef TX_PIN_NUMBER

#define TX_PIN_NUMBER  19
#define RX_PIN_NUMBER  18
#define CTS_PIN_NUMBER 8
#define RTS_PIN_NUMBER 10
#define HWFC true

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 256

#endif

// some debug specific-code
#ifdef DEBUG
#undef printf
int printf (const char *fmt, ...) {
	va_list list;
	int i;
	va_start (list, fmt);
	char buf[256] = { 0 };
	i = vsprintf (buf, fmt, list);
	va_end (list);
	for (char *p = buf; *p; p++)
		app_uart_put (*p);
	return i;
}
#else
//#undef printf
//#define printf(x,...) false
#endif // DEBUG

#undef APP_ERROR_HANDLER
#define APP_ERROR_HANDLER(ERR_CODE) app_error_handler_custom((ERR_CODE), __LINE__, (uint8_t*) __FILE__);
void app_error_handler_custom (ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
	printf ("ERROR! code: %d line: %d file: %s\n", error_code, line_num, p_file_name);
}

#define TN(id) {static char buf[32]; sprintf(buf, "0x%04x", id); return buf; }
#define T(id) if (type == id) return #id; else

char *gapEventName (int type) {
	T (BLE_GAP_EVT_CONNECTED);
	T (BLE_GAP_EVT_DISCONNECTED);
	T (BLE_GAP_EVT_CONN_PARAM_UPDATE);
	T (BLE_GAP_EVT_SEC_PARAMS_REQUEST);
	T (BLE_GAP_EVT_SEC_INFO_REQUEST);
	T (BLE_GAP_EVT_PASSKEY_DISPLAY);
	T (BLE_GAP_EVT_AUTH_KEY_REQUEST);
	T (BLE_GAP_EVT_AUTH_STATUS);
	T (BLE_GAP_EVT_CONN_SEC_UPDATE);
	T (BLE_GAP_EVT_TIMEOUT);
	T (BLE_GAP_EVT_RSSI_CHANGED);

	T (BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP);
	T (BLE_GATTC_EVT_REL_DISC_RSP);
	T (BLE_GATTC_EVT_CHAR_DISC_RSP);
	T (BLE_GATTC_EVT_DESC_DISC_RSP);
	T (BLE_GATTC_EVT_ATTR_INFO_DISC_RSP);
	T (BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP);
	T (BLE_GATTC_EVT_READ_RSP);
	T (BLE_GATTC_EVT_CHAR_VALS_READ_RSP);
	T (BLE_GATTC_EVT_WRITE_RSP);
	T (BLE_GATTC_EVT_HVX);
	T (BLE_GATTC_EVT_TIMEOUT);

	T (BLE_GATTS_EVT_WRITE);
	T (BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST);
	T (BLE_GATTS_EVT_SYS_ATTR_MISSING);
	T (BLE_GATTS_EVT_HVC);
	T (BLE_GATTS_EVT_SC_CONFIRM);
	T (BLE_GATTS_EVT_TIMEOUT);

	TN (type);
};

char *hidsEventName (int type) {
	T (BLE_HIDS_EVT_HOST_SUSP);
	T (BLE_HIDS_EVT_HOST_EXIT_SUSP);
	T (BLE_HIDS_EVT_NOTIF_ENABLED);
	T (BLE_HIDS_EVT_NOTIF_DISABLED);
	T (BLE_HIDS_EVT_REP_CHAR_WRITE);
	T (BLE_HIDS_EVT_BOOT_MODE_ENTERED);
	T (BLE_HIDS_EVT_REPORT_MODE_ENTERED);
	T (BLE_HIDS_EVT_REPORT_READ);
	TN (type);
};

char *bleAdvEvtName (int type) {
	T (BLE_ADV_EVT_IDLE);
	T (BLE_ADV_EVT_DIRECTED);
	T (BLE_ADV_EVT_DIRECTED_SLOW);
	T (BLE_ADV_EVT_FAST);
	T (BLE_ADV_EVT_SLOW);
	T (BLE_ADV_EVT_FAST_WHITELIST);
	T (BLE_ADV_EVT_SLOW_WHITELIST);
	T (BLE_ADV_EVT_WHITELIST_REQUEST);
	T (BLE_ADV_EVT_PEER_ADDR_REQUEST);
	TN (type);
};

char *nrfEvtName (int type) {
	T (NRF_EVT_HFCLKSTARTED);
	T (NRF_EVT_POWER_FAILURE_WARNING);
	T (NRF_EVT_FLASH_OPERATION_SUCCESS);
	T (NRF_EVT_FLASH_OPERATION_ERROR);
	T (NRF_EVT_RADIO_BLOCKED);
	T (NRF_EVT_RADIO_CANCELED);
	T (NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN);
	T (NRF_EVT_RADIO_SESSION_IDLE);
	T (NRF_EVT_RADIO_SESSION_CLOSED);
	T (NRF_EVT_NUMBER_OF_EVTS);
	TN (type);
}

void uart_error_handle (app_uart_evt_t * p_event) {
#if 0
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
		APP_ERROR_HANDLER (p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR) {
		APP_ERROR_HANDLER (p_event->data.error_code);
	}
#endif
}

void debug_init () {
	uint32_t err_code;
	const app_uart_comm_params_t comm_params = { RX_PIN_NUMBER, TX_PIN_NUMBER, RTS_PIN_NUMBER, CTS_PIN_NUMBER,
		APP_UART_FLOW_CONTROL_DISABLED, false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};
	APP_UART_FIFO_INIT (&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOW, err_code);
	APP_ERROR_CHECK (err_code);
	printf ("---\nUART initialized.\n");
}
