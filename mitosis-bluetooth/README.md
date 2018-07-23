# Mitosis-BT

## Status

Bluetooth version doesn't work yet!

### Summary

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

