# Mitosis-BT

## Status

### Works

* Mitosis debouncing code
* Bluetooth and Gazell timesharing
* Master and slave mode (left half doesn't need firmware update at all)
* Battery level reporting via Bluetooth
* Debugging via UART (pin 19)
* Initial qwerty layout support

### TODO

* QMK layout support
* Pairing key shortcut
* Switching between RF and Bluetooth modes
* Switching between Bluetooth devices

## Example debug info

```
UART initialized.
erase bonds: 0
gazell_sd_radio_init
Gazell initialized
on_adv_evt - 3(BLE_ADV_EVT_FAST)
Endpoint: fd:5d:49:00:a2:af
Entering main loop.
on_ble_evt - 16 (BLE_GAP_EVT_CONNECTED)
Connected to b0:35:9f:91:94:e7
on_ble_evt - 20 (BLE_GAP_EVT_SEC_INFO_REQUEST)
on_ble_evt - 26 (BLE_GAP_EVT_CONN_SEC_UPDATE)
on_ble_evt - 80 (BLE_GATTS_EVT_WRITE)
on_hids_evt - 4 (BLE_HIDS_EVT_REP_CHAR_WRITE)
on_hid_rep_char_write - 4 (BLE_HIDS_EVT_REP_CHAR_WRITE)
on_hid_rep_char_write - report value: 0
Sending HID report: 00 00 12 00 00 00 00 00
Sending HID report: 00 00 00 00 00 00 00 00
Sending battery report: 100% (3331mV of 3000mV)
Sending HID report: 80 00 00 00 00 00 00 00
Sending HID report: 00 00 00 00 00 00 00 00
```

## References

* https://github.com/joric/mitosis/tree/devel
