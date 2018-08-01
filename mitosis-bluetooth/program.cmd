@echo off

set build=Debug
set merge=0

set file=%~dp0custom\iar\%build%\Exe\nrf51822_xxac.hex
set s130=%~dp0..\..\components\softdevice\s130\hex\s130_nrf51_2.0.0_softdevice.hex

::GCC version
::set file=%~dp0custom\armgcc\_build\nrf51822_xxac.hex

set option=bluepill

goto %option%

:stlink

rem ST-Link V2 and OpenOCD

set openocd=C:\SDK\openocd-0.10.0-dev-00247-g73b676c\bin-x64
set path=%openocd%;%path%

::set file=%s130%

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

set opt=
if "%merge%"=="1" (
echo Merging softdevice...
mergehex.exe --quiet -m %s130% %file% -o out.hex || exit
set opt=-ex "mon erase"
) else (
copy /Y %file% out.hex
)

echo Uploading...

arm-none-eabi-gdb.exe --quiet --batch -ex "target extended-remote \\.\%port%" -ex "mon swdp_scan" ^
-ex "file out.hex" -ex "att 1" %opt% -ex "load" -ex "run"

:end

