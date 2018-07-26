@echo off
set iar=C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.3\common\bin
set path=%iar%;%path%

IarBuild.exe mitosis\s130\iar\mitosis_bluetooth_s130.ewp Release
copy /Y mitosis\s130\iar\Release\Exe\ble_app_hids_keyboard_s130_custom.hex ..\precompiled_iar\mitosis-bt.hex

