# Mitosis Keyboard Firmware

## Video

[![mitosis-mjt](http://img.youtube.com/vi/hMlQvZNmCc8/0.jpg)](https://www.youtube.com/watch?v=hMlQvZNmCc8)

## Summary

This branch adds support of [YJ-14015][yj-ebay] bluetooth modules and IAR.

It also makes use of the keyswitch board LED (it blinks two times on powering up to indicate that firmware works).

I'm using [modified QMK firmware](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/mitosis.c) (with mitosis-MJT keymap) that adds LED startup sequence to the receiver as well.

## Software

Original Mitosis software repository: https://github.com/reversebias/mitosis

* [IAR] - IDE that includes a C/C++ compiler (IAR 7.50.2 for ARM)
* [nRF5 SDK] - nRF51/52 toolchain ([nRF5_SDK_11.0.0_89a8197.zip])
* [OpenOCD] - embedded debugger for Windows 10 ([openocd-0.10.0-dev-00247-g73b676c.7z])
* [WinAVR] - firmware tools for AVR MCU ([WinAVR-20100110-install.exe])
* [Zadig] - you will need to install libusb in order to run OpenOCD ([zadig-2.3.exe])

## Hardware

Original Mitosis hardware repository: https://github.com/reversebias/mitosis-hardware

* [ST-LINK/V2][stlink]: $2.54, had in stock (you can also use $2 stm32 board instead).
* [YJ-14015][yj-ali]: nrf51822 modules, 3 pcs: $10.5 ($3.50 * 3), free shipping, need 2/3, so $7.
* [10 main PCB's][mitosis.zip] from [EasyEDA], $13.32 ($2 + $11.32 for trackable shipping), used 4/10, so $5.32.
* [3 receiver PCB's][receiver.zip] from [OshPark], $5.40, free shipping, used only 1/3, so $1.80.
* [Arduino Pro Micro](https://www.aliexpress.com/item/-/32648920631.html) from Aliexpress (price varies from $2.54 to $4), had in stock.
* [Si2302] mosfets: board survives reverse polarity for a short while, you may just [short the pads](https://i.imgur.com/h1Mx8Yw.jpg).
* [ASMB-MTB1-0A3A2] from Aliexpress (or Cree CLVBA-FKA, or 3 single LEDs, very optional)
* AMS1117: 5v to 3v regulator, had in stock. You probably can use diodes for 2v drop.
* 1206 4.7k resistor arrays, 2 pcs: had in stock (taken from an old motherboard).
* Switches and caps: most of you have more than you can handle.

### Total

* Keyboard: $12.32 ($7 + $5.32) and I got enough PCBs to build 3 and they can reuse receiver.
* Receiver: $7.84 ($3.50 + $1.80 + $2.54) firmware upgrade to ble and you wouldn't need it at all.

So, about $20 for a single keyboard.

### PCB Manufacturers

* https://oshpark.com - 3 purple receiver PCBs, $5.40, untracked, 21 days
* http://easyeda.com - 10 green PCBs for $2 + $11.32 shipping = $13.32, trackable, 10 days
* https://www.elecrow.com - 6 black PCBs for $4.90 + $6.42 shipping = $11.32, trackable, 36 days
* http://dirtypcbs.com - 10 black for $16.95 + $9.00 shipping = $25.95, untrackable, lost/refunded
* https://www.seeedstudio.com - 10 black PCBs for $4.90 + $16.50 shipping = $21.40 - untested
* https://jlcpcb.com - 10 black PCBs for $2 + $10.98 shipping = $12.98 - untested

## IAR support

Original build used blue Waveshare Core51822 (B)
modules ([$6.99](http://www.waveshare.com/core51822-b.htm)), but aliexpress has slightly smaller black YJ-14015
modules that are also much cheaper (about $3.50), you can find them on Aliexpress or [eBay](http://www.ebay.com/itm/BLE4-0-Bluetooth-2-4GHz-Wireless-Module-NRF51822-Board-Core51822-B-/282575577879).
It seems like stock firmware that works on Core51822 (B), doesn't work on YJ-14015 at all (update, already works, see https://github.com/reversebias/mitosis/pull/7).
You can build a working firmware in IAR, using provided project (open .eww, run build, that's all).
You may also use precompiled firmware from the `precompiled_iar` folder.

## GCC support

This repository now supports GCC for YJ-14015 as well, so there's no need for IAR.
Use GCC-prebuild firmware from the precompiled folder or apply this diff https://github.com/reversebias/mitosis/pull/7
(update: it's already merged into upstream).

## nRF51 Firmware

This firmware is used for the nRF51 modules on the keyboard halves and the receiver.

### Precompiled nRF51 Firmware

This precompiled firmware features YJ-14015 support and status LEDs support.

* left half: [precompiled-basic-left.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-left.hex)
* right half: [precompiled-basic-right.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-right.hex)
* receiver: [precompiled-basic-receiver.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-receiver.hex)

### Uploading nRF51 Firmware

To flash nRF modules, connect ST-LINK/V2 to the module programming pins (SWCLK, SWDIO, GND, 3.3V - top to bottom) and run this batch (windows 10):

```
@echo off
set path=C:\SDK\openocd-0.10.0-dev-00247-g73b676c\bin-x64;%path%
set file=%~dp0custom\iar\_build\nrf51822_xxac.hex
openocd -f interface/stlink-v2.cfg -f target/nrf51.cfg ^
-c init -c "reset halt" -c "flash write_image erase %file:\=/%" -c reset -c exit

```

### Building nRF51 Firmware

I am using Windows 10 and Windows Subsystem for Linux (WSL) for everything.

Building nRF firmware with GCC:

```
sudo apt install openocd gcc-arm-none-eabi
(edit nRF5_SDK_11/components/toolchain/gcc/Makefile.posix, set GNU_INSTALL_ROOT := /usr/)
cd nRF5_SDK_11
git clone https://github.com/joric/mitosis && cd mitosis && git checkout devel
cd mitosis-keyboard-basic/custom/armgcc && make

```

## QMK Firmware

This firmware is used for the Arduino Pro Micro on the keyboard receiver.

### Precompiled QMK Firmware

First of all, check [QMK repository](https://github.com/qmk/qmk_firmware/tree/master/keyboards/mitosis/keymaps) maybe your keymap is already merged.

* mitosis-default (original) from /u/reverse_bias: [kle](http://www.keyboard-layout-editor.com/#/gists/f89d2ff79b72e920939b), [keymap.c](https://github.com/qmk/qmk_firmware/blob/master/keyboards/mitosis/keymaps/default/keymap.c), [firmware](https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_default.hex), [pic](http://i.imgur.com/Ioh0Iix.png)
* mitosis-qwerty (qwerty layout) from /u/runninghack: [kle](http://www.keyboard-layout-editor.com/#/gists/acec41432b5ce0eb341d88245a20d9da), [keymap.c](https://github.com/runninghack/qmk_firmware/blob/master/keyboards/mitosis/keymaps/qwerty/keymap.c), [firmware](https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_qwerty.hex), [pic](https://i.imgur.com/Jt0lTWt.png)
* mitosis-mjt (qwerty layout) from /u/kaybeerry: [kle](http://www.keyboard-layout-editor.com/#/gists/e213fec721cb035614cef401cd6750d6), [keymap.c](https://github.com/mterhar/qmk_firmware/blob/master/keyboards/mitosis/keymaps/mjt/keymap.c), [firmware](https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_mjt.hex), [pic](https://i.imgur.com/BiKsnIV.png)
* mitosis-carvac_dv (qwerty layout) by /u/Carvac: [keymap.c](https://github.com/CarVac/qmk_firmware/blob/master/keyboards/mitosis/keymaps/carvac_dv/keymap.c)
* mitosis-workman (workman layout) by /u/mloffer: [kle](http://www.keyboard-layout-editor.com/#/gists/db73d647ad8a67c9654a4daeab0e0873), [keymap.c](https://github.com/mloffer/mitosis-workman)
* mitosis-atreus (atreus layout) from /u/iamjoric: [kle](http://www.keyboard-layout-editor.com/#/gists/e1b26b86c4e024d2f4f3185e5b769a00), [keymap.c](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/keymaps/atreus/keymap.c), [firmware](https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_atreus.hex)
* mitosis-joric (experimental) from /u/iamjoric: [kle](http://www.keyboard-layout-editor.com/#/gists/3f5dd1c848bb9a7a723161ad5e0c8e39), [keymap.c](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/keymaps/joric/keymap.c), [firmware](https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_joric.hex)

### Uploading QMK Firmware

To flash prebuilt QMK firmware on Pro Micro, press reset button on the receiver and run the script below.
You can look up the bootloader port in device manager (e.g. COM9), it shows only for a few seconds after reset.

```
set path=C:\WinAVR-20100110\bin;%path%
avrdude -p atmega32u4 -P COM9 -c avr109  -U flash:w:mitosis_default.hex
```

### Building QMK firmware:

```
sudo apt-get install gcc-avr avr-libc
cd qmk_firmware
make mitosis-default
```

## Bluetooth Version

Mitosis is not a Bluetooth keyboard. It uses a proprietary rf-protocol (Gazell) and a dedicated receiver.
Keyboard halves cannot be used as individual HID devices because global layers won't work.
We need two wireless channels - Bluetooth HID for the PC and something for the communication between halves.
There's no working Bluetooth Mitosis firmware yet, possible solutions are listed below.

### Bluetooth HID + Gazell protocol

There's an example of Bluetooth and Gazell running concurrently but
it seems a bit dated and doesn't support both Gazell host and BLE:
https://github.com/NordicPlayground/nrf51-ble-gzll-device-uart

### Bluetooth HID + UART service

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
* [Split Bluetooth Keyboard](https://gist.github.com/wez/b30683a4dfa329b86b9e0a2811a8c593) (gist) by [wez](https://gist.github.com/wez)

#### BlueMicro

This is a drop-in Pro Micro replacement that is compatible with Arduino nRF52 boards (it's NOT compatible with nRF51).
BlueMicro is open source, official repositories are [BlueMicro_BLE] (firmware) and [NRF52-Board] (hardware).

* [Iris gets the BLE treatment](https://redd.it/8rtvi7) by [/u/jpconstantineau](http://reddit.com/u/jpconstantineau)
* [Ergotravel 2](https://redd.it/8i2twe) by [/u/jpconstantineau](http://reddit.com/u/jpconstantineau)

[BlueMicro_BLE]: https://github.com/jpconstantineau/BlueMicro_BLE 
[NRF52-Board]: https://github.com/jpconstantineau/NRF52-Board

## Mitosis Clones

* [Interphase](https://github.com/Durburz/interphase) (66 keys, keyboard matrix with diodes, voltage regulator and AAA battery, KiCad project) ([Reddit](https://redd.it/7ggeww))
* [Meiosis](https://redd.it/7uasay), [Telophase](https://redd.it/7ruihw), [Helicase](https://redd.it/7zfj19) and [Centrometre](https://redd.it/8qkib4) (from [/u/SouthPawEngineer](https://www.reddit.com/u/SouthPawEngineer), no source files available) ([Site](https://southpawdesign.net))
* [Chimera](https://github.com/GlenPickle/Chimera) (Chimera Ergo, Chimera Ortho, Chimera Ergo Mini, Chimera Ergo 42 - no schematics, just gerbers)
* [Kissboard](https://github.com/fhtagnn/kissboard) ([AdNW](http://adnw.de)-inspired layout, PCB's are not tested) ([Reddit](https://redd.it/8bauoz)) ([Album](https://imgur.com/a/A95FF))
* [Dichotomy](https://redd.it/7g54l8) (48-key Mitosis clone with digital encoders, no sources available) ([Youtube](https://youtu.be/5jmmYbgtOgI)) ([Kickstarter](https://www.kickstarter.com/projects/1090732691/dichotomy-keyboard-and-mouse)) 
* [Trident](https://github.com/YCF/Trident) (Wireless Let's Split by [/u/imFengz](https://www.reddit.com/u/imFengz), module and battery placed between the switches) ([Reddit](https://redd.it/6um7eg)) ([Image](https://i.imgur.com/mCTgwu5.png))
* [Orthrus](https://github.com/bezmi/orthrus) (great 52-key Atreus/Mitosis crossover by [/u/bezmi](https://www.reddit.com/u/bezmi), KiCad project) ([Reddit](https://redd.it/8txry7)) 

## References

* [Wireless + Split + QMK = Mitosis (Reddit)](https://www.reddit.com/r/MechanicalKeyboards/comments/66588f/wireless_split_qmk_mitosis/)
* [A collection of Mitosis Kicad ports from Altium (GitHub)](https://github.com/joric/mitosis-hardware/tree/joric_kicad)

[EasyEDA]: https://easyeda.com
[OshPark]: https://oshpark.com
[ASMB-MTB1-0A3A2]: https://www.aliexpress.com/item/-/32809898075.html
[Si2302]: https://www.aliexpress.com/item/-/32883659198.html
[mitosis.zip]: https://github.com/reversebias/mitosis-hardware/blob/master/gerbers/mitosis.zip
[receiver.zip]: https://github.com/reversebias/mitosis-hardware/blob/master/gerbers/receiver.zip
[stlink]: http://www.ebay.com/itm/331803020521
[yj-ali]: https://www.aliexpress.com/item/-/32832872640.html
[yj-ebay]: https://www.ebay.com/itm/282575577879
[IAR]: https://www.iar.com
[NRF5 SDK]: https://developer.nordicsemi.com/nRF5_SDK
[OpenOCD]: http://www.freddiechopin.info/en/download/category/10-openocd-dev
[WinAVR]: https://sourceforge.net/projects/winavr
[Zadig]: https://zadig.akeo.ie
[nRF5_SDK_11.0.0_89a8197.zip]: https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip
[openocd-0.10.0-dev-00247-g73b676c.7z]: http://www.freddiechopin.info/en/download/category/10-openocd-dev?download=140%3Aopenocd-0.10.0-dev-00247-g73b676c
[WinAVR-20100110-install.exe]: https://sourceforge.net/projects/winavr/files/WinAVR/20100110/WinAVR-20100110-install.exe/download
[zadig-2.3.exe]: https://zadig.akeo.ie/downloads/zadig-2.3.exe

