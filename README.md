# Mitosis Keyboard Firmware

This branch adds YJ-14015 and IAR support.

I'm using [IAR 7.50.2](https://www.iar.com/) for ARM,
[nRF5_SDK_11](https://developer.nordicsemi.com/nRF5_SDK/) toolkit,
[OpenOCD g73b676c](http://www.freddiechopin.info/en/download/category/10-openocd-dev/) for Windows 10
and [ST-LINK/V2](http://www.ebay.com/itm/ST-Link-V2-Programming-Unit-mini-STM8-STM32-Emulator-Downloader-M89-Top-/331803020521) from ebay.

## IAR support

Original build used blue Waveshare Core51822 (B)
modules ([$6.99](http://www.waveshare.com/core51822-b.htm)), but aliexpress has slightly smaller black YJ-14015
modules that are also much cheaper (about [$3.50](https://www.aliexpress.com/item/BLE4-0-Bluetooth-2-4GHz-Wireless-Module-NRF51822-Board-Core51822-B/32633417101.html),
you can also find them on [ebay](http://www.ebay.com/itm/BLE4-0-Bluetooth-2-4GHz-Wireless-Module-NRF51822-Board-Core51822-B-/282575577879)).
It seems like precompiled firmware that works on Core51822 (B), doesn't work on YJ-14015 at all.

Not sure why it doesn't work after GCC toolchain, but you can build a working firmware in IAR, using provided project.
It also needs defined `CLOCK_ENABLED` and `RTC0_ENABLED` in the receiver firmware (`nrf_drv_config.h`),
otherwise it doesn't compile.

IAR 7.50.2 compatible project is based upon `peripheral/blinky`
sample from nRF5_SDK_11.

You may also use precompiled firmware from the `precompiled_iar` folder.
This version also makes use of the keyswitch board LED (it blinks two times on powering up to indicate that firmware works).

Bat file for flashing:

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









