# Mitosis-Bluetooth

Bluetooth firmware for the Mitosis keyboard (BLE and Gazell timesharing via timeslot API)

## Video

[![](http://img.youtube.com/vi/Qv22OyWb81g/0.jpg)](https://youtu.be/Qv22OyWb81g)

## Precompiled Firmware

* [mitosis-bt.hex] (firmware upgrade for the right half, turns Mitosis into a fully wireless split Bluetooth HID keyboard)

Hold Fn key ([S20][pinout]) while powering on to erase bonding data from memory or hold (only) Fn key for 10 seconds to reboot.

## Uploading Firmware

You need to flash the right half only.
Use [ST-LINK/V2] and [OpenOCD].
Precompiled firmware is already merged with softdevice s130 so you don't need to flash softdevice first.

```
openocd -f interface/stlink-v2.cfg -f target/nrf51.cfg ^
-c init -c "reset halt" -c "flash write_image erase mitosis-bt.hex" -c reset -c exit
```

### Default layout (Mitosis-BT)

[![](https://kle-render.herokuapp.com/api/3f5dd1c848bb9a7a723161ad5e0c8e39?4)](http://www.keyboard-layout-editor.com/#/gists/3f5dd1c848bb9a7a723161ad5e0c8e39)

## Building

### IAR

Open mitosis-bluetooth.eww, hit Make, that's it.
I'm using a single plate (reversed) version for
the Debug configuration (modules soldered to the top of the PCB),
to debug standard version remove `COMPILE_REVERSED` from the preprocessor directives or switch
to Release configuration.

### GCC

As usual, change directory to custom/armgcc, make.
Working GCC linker settings for softdevice s130 2.0.0 and [YJ-14015] modules (256K ROM, 16K RAM) appear to be:
```
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x25000
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 0x2000
```

## Debugging

You can hook up a single UART RX pin at 115200 baud ([currently pin 21, key S15 or S23](https://i.imgur.com/apx8W8W.png)).
You will also need common GND and VCC to make it work. It doesn't really interfere much with the keyboard matrix so you can use any pin you want,
just don't use the same pin for TX and RX to avoid feedback.

You can also use [$1.80](https://www.aliexpress.com/item//32583160323.html) STM32 board,
upgrade it with UART adapter ([RX - A9, TX - A10](https://i.imgur.com/sLyYM27.jpg))
into a [Blackmagic] board,
and use it as an [ST-LINK/V2] replacement ([SWCLK - A5, SWDIO - B14](https://i.imgur.com/Ikt8yZz.jpg)).
It is actually much better because it also has a built in UART ([pin A3][pinout])
on the second virtual COM port so you don't need another USB.
See https://github.com/joric/mitosis/tree/devel#bluepill

I'm using [nRF5 SDK 11] (mostly because original Mitosis using it).
There's no softdevice s110 support so we are limited to 6K RAM.
Bluetooth devices seem to shutdown and restart a lot (sleep mode is actually power off mode
with a hardware interrupt from the pin that restarts the device).
Default pairing works on power on only, hold the button (currently [S20][pinout])
to erase pairing information.

There is a built in app_trace_log but it doesn't work with GCC (probably lacks free memory)
so I had to write a small drop-in replacement, but in IAR you can just use the following preprocessor
directives (the last one is optional if it's too verbose):

```
DEBUG
NRF_LOG_USES_UART=1
NRF_LOG_ENABLED=1
ENABLE_DEBUG_LOG_SUPPORT=1
DM_DISABLE_LOGS=1
```

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
* Switching between RF and Bluetooth modes
* Switching between Bluetooth devices

Please contribute!

## References

* [My fork of the Mitosis repository (bonus documentation included)](https://github.com/joric/mitosis/tree/devel)
* [Reddit thread](https://redd.it/91s4pu)

[mitosis-bt.hex]: https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/mitosis-bt.hex
[ST-LINK/V2]: http://www.ebay.com/itm/331803020521
[OpenOCD]: http://www.freddiechopin.info/en/download/category/10-openocd-dev
[YJ-14015]: https://www.ebay.com/itm/282575577879
[Blackmagic]: https://gojimmypi.blogspot.com/2017/07/BluePill-STM32F103-to-BlackMagic-Probe.html
[nRF5 SDK 11]: https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip
[pinout]: https://i.imgur.com/apx8W8W.png
[RAM]: https://devzone.nordicsemi.com/b/blog/posts/rom-and-ram-management
