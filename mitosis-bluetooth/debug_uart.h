#include "app_uart.h"

#ifndef TX_PIN_NUMBER

// there are 3 pins that are not broken out on the Mitosis: 11, 12, 20, plus LED pin (pin 17)
// pins 11 and 12 are the leftmost bottom on the module, pin 20 is the rightmost bottom
// we also can hook up to switches, e.g. pin 19 (bottom right switch) but it would be occupied
// RX, CTS, RTS pins are unused, you can set them to any value (except TX pin to avoid feedback)

#define TX_PIN_NUMBER  21
#define RX_PIN_NUMBER  20
#define CTS_PIN_NUMBER 20
#define RTS_PIN_NUMBER 20
#define HWFC true

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 256

#endif

void uart_error_handle (app_uart_evt_t * p_event) {
#if 0 // unused
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
		APP_ERROR_HANDLER (p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR) {
		APP_ERROR_HANDLER (p_event->data.error_code);
	}
#endif
}

void debug_init () {
#ifdef DEBUG
	uint32_t err_code;
	const app_uart_comm_params_t comm_params = { RX_PIN_NUMBER, TX_PIN_NUMBER, RTS_PIN_NUMBER, CTS_PIN_NUMBER,
		APP_UART_FLOW_CONTROL_DISABLED, false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};
	APP_UART_FIFO_INIT (&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOW, err_code);
	APP_ERROR_CHECK (err_code);
	printf ("---\nUART initialized.\n");
#endif
}


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
#endif

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

char * hciStatusName(int type) {
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
	TN(type);
}


#undef APP_ERROR_HANDLER
#define APP_ERROR_HANDLER(ERR_CODE) app_error_handler_custom((ERR_CODE), __LINE__, (uint8_t*) __FILE__);
void app_error_handler_custom (ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
	printf ("ERROR! code: %d status: %s line: %d file: %s\n", (int)error_code, hciStatusName((int)error_code), (int)line_num, p_file_name);
}

