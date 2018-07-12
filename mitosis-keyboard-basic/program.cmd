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

