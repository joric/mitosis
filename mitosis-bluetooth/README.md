# Mitosis-BT

## Status

Bluetooth version doesn't work yet!

## Summary

Mitosis is not a Bluetooth keyboard. It uses a proprietary rf-protocol (Gazell) and a dedicated receiver.
Global layers need two wireless channels - Bluetooth HID and something for the communication between halves.
Possible solutions are listed below.

### Gazell

There are examples of Bluetooth and Gazell running concurrently but no working HID firmware yet.

* https://github.com/NordicPlayground/nrf51-ble-gzll-device-uart (no Gazell host supported)
* https://github.com/NordicPlayground/nRF51-multi-role-conn-observer-advertiser (usage of the timeslot API)


### Bluetooth UART service

There are a few split keyboards that successfully run Bluetooth HID and Bluetooth UART service concurrently.
Most of them use Adafruit nRF52 library and simple sketches written and compiled in Arduino IDE.

#### Arduino nRF5

There is [Arduino-nRF5 by Sandeep Mistry] that supports nRF51.
You could use [arduino-BLEPeripheral] library for sketches.
Works fine with BLE400 board ([Arduino IDE setup](https://i.imgur.com/8dfPZFm.jpg), [wiring](https://i.imgur.com/A9QIN2j.jpg)).
Sadly this library has [multiple issues](https://github.com/sandeepmistry/arduino-BLEPeripheral/issues/160) with Windows 10.

[Arduino-nRF5 by Sandeep Mistry]: https://github.com/sandeepmistry/arduino-nRF5
[arduino-BLEPeripheral]: https://github.com/sandeepmistry/arduino-BLEPeripheral

#### Arduino nRF52

Note that Arduino nRF52 builds (usually based on [Bluefruit nRF52](https://www.adafruit.com/product/3406) boards)
are NOT compatible with the Mitosis keyboard (softdevice s132 and all the software is nRF52-only, Mitosis is nRF51-based).

* [Curves - my bluetooth split](https://redd.it/86asf6) by [/u/JKPro777](http://reddit.com/u/JKPro777)
* [Split Bluetooth Keyboard](https://redd.it/7fdrdz) by [/u/wezfurlong](https://www.reddit.com/u/wezfurlong) ([gist](https://gist.github.com/wez/b30683a4dfa329b86b9e0a2811a8c593))

#### BlueMicro

This is a drop-in Pro Micro replacement that is compatible with Arduino nRF52 boards (it's NOT compatible with nRF51).
BlueMicro is open source, official repositories are [BlueMicro_BLE] (firmware) and [NRF52-Board] (hardware).

* [Iris gets the BLE treatment](https://redd.it/8rtvi7) by [/u/jpconstantineau](http://reddit.com/u/jpconstantineau)
* [Ergotravel 2](https://redd.it/8i2twe) by [/u/jpconstantineau](http://reddit.com/u/jpconstantineau)

[BlueMicro_BLE]: https://github.com/jpconstantineau/BlueMicro_BLE 
[NRF52-Board]: https://github.com/jpconstantineau/NRF52-Board

## My setup

I'm using ST-Link V2, BLE400 board and its built-in USB-UART.

BLE400 also has built-in UART (unfortunately, ST-Link V2 doesn't have SWO pin for printf, so I had to use UART at 115200).

Also it seems SWO inherently doesn't work on nRF51822's ([there is no tracing hardware in the nRF51 series](https://devzone.nordicsemi.com/f/nordic-q-a/1875/nrf51822---debug-output-via-j-link-swo)).


### ST-Link V2

![](https://i.imgur.com/A9QIN2j.jpg)

or like this:

![](https://i.imgur.com/5khy8c0.jpg)

![](https://i.imgur.com/ZwavvEz.jpg)


### Blue Pill

There are $2 STM32F103 modules (Blue Pill) that you can use as ST-Link replacement.
Most likely you get 64K device (page is not writeable, etc.) so just run [STM32 Flash loader demonstrator](https://www.st.com/en/development-tools/flasher-stm32.html)
GUI, hook up STM32F103 via UART (use 5V, RX to A9, TX to A10), force select 128K device, 0x08002000, and flash blackmagic.bin from there.
See detailed instructions: https://gojimmypi.blogspot.com/2017/07/BluePill-STM32F103-to-BlackMagic-Probe.html

![](https://i.imgur.com/9fDjU5q.jpg)

Programming nRF51 is tricky, first merge s130 softdevice hex with your hex using nRF Command line tools, then use GDB.

```
mergehex.exe -m s130.hex mitosis.hex -o out.hex
arm-none-eabi-gdb.exe -ex "target extended-remote \\.\COM5" -ex "mon swdp_scan" -ex "att 1" ^
-ex "mon erase_mass" –ex "load out.hex" –ex "quit"

```

Then disconnect the programmer and reconnect power, or run the program with gdb prompt - "load out.hex", "run".

As you can see, no OpenOCD needed! Pins on STM32F103 are: SWCLK/SWD - A5, SDO/SWIO/SWDIO - B14.

![](https://i.imgur.com/V3RtvBr.jpg)

## Debugging

Since nRF51 and ST-Link doesn't support SWO I just hooked up a single TX pin (pin 19) for debug. It actually works!

So you only need ONE pin to print messages in the terminal (I use Arduino IDE Serial Monitor) via UART.

![](https://i.imgur.com/tXjixUZ.jpg)

The second half is connected to power pins only.

![](https://i.imgur.com/IozHbrJ.jpg)


## Schematics

There were speculations that Core 51822 has 32 GPIO pins available, so it's possible to make an Atreus62 without
using a keyboard matrix. It is not true. GPIO 26/27 are shared by the 32kHz crystal. Pin 31 (AINT7) isn't
routed outside as well. So mitosis only have 4 extra pins available: 11, 12, 20, plus LED pin (17 or 23)
and the best case scenario for nRF51822 Core-B (reference design) is 26 or 27 keys on each side,
52 keys total (54 if you get rid of the LED).

### nRF51822 Core-B Schematics
![](http://i.imgur.com/8aF2mbI.png)

### nRF51822 Core-B Pinout
![](https://www.waveshare.com/img/devkit/accBoard/Core51822-B/Core51822-B-pin.jpg)

### Mitosis PCB

![](https://i.imgur.com/8JwVmML.jpg)


