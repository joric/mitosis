@echo off

set option=bluepill

goto %option%

:stlink

set openocd=C:\SDK\openocd-0.10.0-dev-00247-g73b676c\bin-x64
set path=%openocd%;%path%

::set file=%~dp0..\..\components\softdevice\s130\hex\s130_nrf51_2.0.0_softdevice.hex
set file=%~dp0mitosis\s130\iar\Release\Exe\ble_app_hids_keyboard_s130_custom.hex

openocd -f interface/stlink-v2.cfg -f target/nrf51.cfg ^
-c init -c "reset halt" -c "flash write_image erase %file:\=/%" -c reset -c exit

goto end

:bluepill

rem Blackmagic STM32-based board (Bluepill)

set port=COM5

set eabi=C:\Users\User\AppData\Local\Arduino15\packages\adafruit\tools\gcc-arm-none-eabi\5_2-2015q4\bin
set path=%eabi%;%path%

set nordic=C:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin
set path=%nordic%;%path%

set s130=%~dp0..\..\components\softdevice\s130\hex\s130_nrf51_2.0.0_softdevice.hex
set file=%~dp0mitosis\s130\iar\Debug\Exe\ble_app_hids_keyboard_s130_custom.hex

echo Merging files...

::mergehex.exe --quiet -m %s130% %file% -o out.hex
copy /Y %file% out.hex

echo Uploading...

arm-none-eabi-gdb.exe --quiet --batch -ex "target extended-remote \\.\%port%" -ex "mon swdp_scan" ^
-ex "file out.hex"  -ex "att 1" -ex "load" -ex "run"

:end

