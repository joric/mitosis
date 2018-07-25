# Mitosis-BT

## Video

[![](http://img.youtube.com/vi/Qv22OyWb81g/0.jpg)](https://youtu.be/Qv22OyWb81g)

## Firmware

I use a single plate (reversed) Mitosis version (modules soldered to the top of the PCB).
To make standard version, comment out `#define COMPILE_REVERSED` in `config/mitosis.h`
You may also use precompiled firmware from the [precompiled_iar](../precompiled_iar) folder.
You only need to flash the right half! Don't forget to flash softdevice s130 first (refer to [upload_bt.cmd](upload_bt.cmd) on Windows).

You can also use [$1.80 STM32 board](https://i.imgur.com/Ikt8yZz.jpg) instead of ST-Link v2 for firmware uploading. Read more about it here:
https://github.com/joric/mitosis/tree/devel#bluepill

## Status

### Works

* Mitosis debouncing code
* Bluetooth and Gazell timesharing
* Master and slave mode (left half doesn't need firmware update at all)
* Battery level reporting via Bluetooth
* Debugging via UART
* Basic QMK layout support

### TODO

* HID buffering and improved latency
* Better pairing and a pairing key shortcut
* Switching between RF and Bluetooth modes
* Switching between Bluetooth devices

## Debugging

You can hook up a single UART RX pin (115200 baud) via [pin 19](https://i.imgur.com/apx8W8W.png) (key S23 or key S15 for a non-reversed PCB).
You will also need common GND and VCC to make it work. Oddly enough it works along with the keyboard matrix so you can use any pin you want,
just don't use the same pin for TX and RX to avoid feedback.

### Example debug info

```
UART initialized.
erase bonds: 0
gazell_sd_radio_init
Gazell initialized
on_adv_evt - 3 (BLE_ADV_EVT_FAST)
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

* [My fork of the Mitosis repository (bonus documentation included)](https://github.com/joric/mitosis/tree/devel)
* [Mitosis-BT (Reddit thread)](https://redd.it/91s4pu)

