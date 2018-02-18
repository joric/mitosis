# Mitosis Keyboard Firmware

This branch adds YJ-14015 and IAR support.

[![mitosis-mjt](http://img.youtube.com/vi/hMlQvZNmCc8/0.jpg)](https://www.youtube.com/watch?v=hMlQvZNmCc8)

It also makes use of the keyswitch board LED (it blinks two times on powering up to indicate that firmware works).

I'm using [modified QMK firmware](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/mitosis.c) (with mitosis-MJT keymap) that adds LED startup sequence to the receiver as well.

## Hardware

* [ST-LINK/V2](http://www.ebay.com/itm/ST-Link-V2-Programming-Unit-mini-STM8-STM32-Emulator-Downloader-M89-Top-/331803020521) (OpenOCD-compatible debugger/programmer)
* [YJ-14015](https://www.aliexpress.com/item/Core51822-BLE-4-0-Bluetooth-2-4G-Wireless-Module-NRF51822-Antenna-Board-For-ULP-SPI-I2C/32832872640.html) (nRF51822-module, 32-bit ARM Cortex M0, 128kB flash + 32kB RAM)


## Software

* [IAR 7.50.2 for ARM](https://www.iar.com/) (IDE that includes a C/C++ compiler)
* [nRF5_SDK_11](https://developer.nordicsemi.com/nRF5_SDK/) (nRF51/52 toolchain)
* [OpenOCD g73b676c](http://www.freddiechopin.info/en/download/category/10-openocd-dev/) (embedded debugger for Windows 10)
* [WinAVR 20100110](https://sourceforge.net/projects/winavr/) (firmware tools for AVR MCU)

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

This firmware is used for the keyboard halves.

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
openocd ^
-f interface/stlink-v2.cfg ^
-f target/nrf51.cfg ^
-c init ^
-c "reset halt" ^
-c "flash write_image erase %file:\=/%" ^
-c "reset" ^
-c exit

```

### Building nRF51 Firmware

I am using Windows 10 and Windows Subsystem for Linux (WSL) for everything.

Building nRF firmware with GCC:

```
cd nRF5_SDK_11
git clone https://github.com/joric/mitosis
sudo apt install openocd gcc-arm-none-eabi
(edit /components/toolchain/gcc/Makefile.posix, set GNU_INSTALL_ROOT := /usr/)
sudo cp mitosis/49-stlinkv2.rules /etc/udev/rules.d/
cd mitosis
git checkout devel
cd mitosis-keyboard-basic/custom/armgcc
make

```

## QMK Firmware

This firmware is used for the keyboard receiver.

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










