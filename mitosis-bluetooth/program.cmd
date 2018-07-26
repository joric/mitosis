@echo off

set path=C:\SDK\openocd-0.10.0-dev-00247-g73b676c\bin-x64;%path%

:: flash softdevice first, then flash custom hex
::set file=%~dp0..\..\components\softdevice\s130\hex\s130_nrf51_2.0.0_softdevice.hex
set file=%~dp0mitosis\s130\iar\Release\Exe\ble_app_hids_keyboard_s130_custom.hex

openocd ^
-f interface/stlink-v2.cfg ^
-f target/nrf51.cfg ^
-c init ^
-c "reset halt" ^
-c "flash write_image erase %file:\=/%" ^
-c "reset" ^
-c exit

