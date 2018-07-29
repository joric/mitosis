@echo off
set iar=C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.3\common\bin
set path=%iar%;%path%

if "%1"=="" (
	set build=Debug
) else (
	set build=%1
)

IarBuild.exe custom\iar\mitosis_bluetooth.ewp %build%

if "%build%"=="Release" (
copy /Y custom\iar\%build%\Exe\nrf51822_xxac.hex ..\precompiled_iar\mitosis-bt.hex
)


