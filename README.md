# Mitosis Keyboard Firmware

## Video

[![mitosis-mjt](http://img.youtube.com/vi/hMlQvZNmCc8/0.jpg)](https://www.youtube.com/watch?v=hMlQvZNmCc8)

## Summary

This branch adds support of [YJ-14015][yj-ebay] bluetooth modules and IAR.

It also makes use of the keyswitch board LED (it blinks two times on powering up to indicate that firmware works).

I'm using [modified QMK firmware](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/mitosis.c) (with mitosis-MJT keymap) that adds LED startup sequence to the receiver as well.

**Also check out my [Mitosis Bluetooth](https://github.com/joric/mitosis-bluetooth) firmware (turns Mitosis into a fully wireless split Bluetooth HID keyboard)**

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

* [precompiled-basic-left.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-left.hex) (left half)
* [precompiled-basic-right.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-right.hex) (right half)
* [precompiled-basic-receiver.hex](https://raw.githubusercontent.com/joric/mitosis/devel/precompiled_iar/precompiled-basic-receiver.hex) (receiver)

### Uploading nRF51 Firmware

#### ST-Link V2

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

* [mitosis_mjt.hex] (recommended) by [/u/kaybeerry](https://reddit.com/u/kaybeerry) ([KLE](http://www.keyboard-layout-editor.com/#/gists/e213fec721cb035614cef401cd6750d6), [keymap.c](https://github.com/mterhar/qmk_firmware/blob/master/keyboards/mitosis/keymaps/mjt/keymap.c), [Image](https://i.imgur.com/BiKsnIV.png))
* [mitosis_default.hex] (original) by [/u/reverse_bias](https://reddit.com/u/reverse_bias) ([KLE](http://www.keyboard-layout-editor.com/#/gists/f89d2ff79b72e920939b), [keymap.c](https://github.com/qmk/qmk_firmware/blob/master/keyboards/mitosis/keymaps/default/keymap.c), [Image](http://i.imgur.com/Ioh0Iix.png))
* [mitosis_qwerty.hex] by [/u/runninghack](https://reddit.com//u/runninghack) ([KLE](http://www.keyboard-layout-editor.com/#/gists/acec41432b5ce0eb341d88245a20d9da), [keymap.c](https://github.com/runninghack/qmk_firmware/blob/master/keyboards/mitosis/keymaps/qwerty/keymap.c), [Image](https://i.imgur.com/Jt0lTWt.png))
* [mitosis_atreus.hex] by [/u/iamjoric](https://reddit.com//u/iamjoric) ([KLE](http://www.keyboard-layout-editor.com/#/gists/e1b26b86c4e024d2f4f3185e5b769a00), [keymap.c](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/keymaps/atreus/keymap.c))
* [mitosis_joric.hex] (experimental) by [/u/iamjoric](https://reddit.com/u/iamjoric) ([KLE](http://www.keyboard-layout-editor.com/#/gists/3f5dd1c848bb9a7a723161ad5e0c8e39), [keymap.c](https://github.com/joric/qmk_firmware/blob/mitosis-joric/keyboards/mitosis/keymaps/joric/keymap.c))

[mitosis_default.hex]: https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_default.hex
[mitosis_qwerty.hex]: https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_qwerty.hex
[mitosis_mjt.hex]: https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_mjt.hex
[mitosis_atreus.hex]: https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_atreus.hex
[mitosis_joric.hex]: https://raw.githubusercontent.com/joric/qmk_firmware/mitosis-joric/precompiled/mitosis_joric.hex

Not compiled:

* [Datagrok](https://github.com/qmk/qmk_firmware/tree/master/keyboards/mitosis/keymaps/datagrok) by [datagrok](https://github.com/datagrok)
* [Carvac_dv](https://github.com/CarVac/qmk_firmware/blob/master/keyboards/mitosis/keymaps/carvac_dv) by [/u/Carvac](https://reddit.com/u/Carvac)
* [Workman](https://github.com/mloffer/mitosis-workman) by [/u/mloffer](https://reddit.com/u/mloffer) ([KLE](http://www.keyboard-layout-editor.com/#/gists/db73d647ad8a67c9654a4daeab0e0873), [Image](https://github.com/mloffer/mitosis-workman/blob/master/mitosis-workman.png?raw=true))


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

## References

* [Wireless + Split + QMK = Mitosis (Reddit)](https://www.reddit.com/r/MechanicalKeyboards/comments/66588f/wireless_split_qmk_mitosis/)
* [A collection of Mitosis Kicad ports from Altium (GitHub)](https://github.com/joric/mitosis-hardware/tree/joric_kicad)
* [Mitosis Bluetooth repository (in development)](https://github.com/joric/mitosis-bluetooth)

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
[AMS1117]: https://www.aliexpress.com/item/-/32826077143.html
[1206 4.7k]: https://www.aliexpress.com/item/-/32853745131.html

