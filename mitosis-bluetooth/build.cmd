@echo off
set iar=C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.3\common\bin
set path=%iar%;%path%

set nordic=C:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin
set path=%nordic%;%path%

set file=custom\armgcc\_build\nrf51822_xxac.hex
set s130=..\..\components\softdevice\s130\hex\s130_nrf51_2.0.0_softdevice.hex
set dest=..\precompiled_iar\mitosis-bt.hex

if "%1"=="Release" goto :release

IarBuild.exe custom\iar\mitosis_bluetooth.ewp Debug

goto end

:release

::cd custom\armgcc && bash -c make && cd /d %~dp0 || exit

set file=custom\iar\Release\Exe\nrf51822_xxac.hex
IarBuild.exe custom\iar\mitosis_bluetooth.ewp Release

echo Merging %s130% and %file% to %dest%...
mergehex.exe --quiet -m %s130% %file% -o %dest%
::copy /Y %file% %dest%

:end
