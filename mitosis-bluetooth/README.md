# Mitosis-BT

## Video

[![](http://img.youtube.com/vi/Qv22OyWb81g/0.jpg)](https://youtu.be/Qv22OyWb81g)

## Precompiled Firmware

* [mitosis-bt.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/mitosis-bt.hex) (Mitosis-BT master, MJT-like layout, for the right half)

## Uploading

You only need to flash the right half! Don't forget to flash softdevice s130 first
(refer to [program.cmd](program.cmd) on Windows).
You can also use [$1.80](https://www.aliexpress.com/item//32583160323.html) STM32 board ([SWCLK - A5, SWDIO - B14](https://i.imgur.com/Ikt8yZz.jpg)) instead of ST-Link v2 for firmware uploading.
This is actually much better because it also has built in UART ([pin A3](https://i.imgur.com/ub1gT4U.jpg)) on the second virtual COM port
so you don't need to occupy another USB port for debugging.
Read more about it here: https://github.com/joric/mitosis/tree/devel#bluepill

## Building

Open mitosis-bluetooth.eww, select Release, hit Make, that's it.
I'm using a single plate (reversed) for the Debug build (modules soldered to the top of the PCB).
To make standard version, remove `COMPILE_REVERSED` from the preprocessor directives.
You may also use firmware from the [precompiled_iar](../precompiled_iar) folder.

## Debugging

You can hook up a single UART RX pin at 115200 baud ([currently pin 21, key S15 or S23](https://i.imgur.com/apx8W8W.png)).
You will also need common GND and VCC to make it work. It doesn't really interfere much with the keyboard matrix so you can use any pin you want,
just don't use the same pin for TX and RX to avoid feedback.

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

Please contribute!

## Default layout (Mitosis-BT)

[![](https://kle-render.herokuapp.com/api/3f5dd1c848bb9a7a723161ad5e0c8e39?3)](https://kle-render.herokuapp.com/api/3f5dd1c848bb9a7a723161ad5e0c8e39)

* [Keyboard Layout Editor](http://www.keyboard-layout-editor.com/#/gists/3f5dd1c848bb9a7a723161ad5e0c8e39)
* [Interactive 3d version](https://joric.github.io/keycaps/#/gists/3f5dd1c848bb9a7a723161ad5e0c8e39)

## References

* [My fork of the Mitosis repository (bonus documentation included)](https://github.com/joric/mitosis/tree/devel)
* [Reddit thread](https://redd.it/91s4pu)

