/* Linker script to configure memory regions. */

/*
  Can't make YJ-14015 work yet. Maybe it's Makefile options, maybe Linker, maybe it's not GCC compatible at all
  used mergegex with s130 and -ex "mon erase" for flashing.

  Defaults are:
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x25000
  RAM (rwx) :  ORIGIN = 0x20002080, LENGTH = 0x5f80
  Considering YJ-14015 requires LENGTH=0x4000 maybe waveshare modules will work?

  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x4000 //doesn't crash, no uart output
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 0x2000 //overflowed with stack
  RAM (rwx) :  ORIGIN = 0x20001000, LENGTH = 0x3000 //error: cannot init softdevice   // ERROR! code: 4 status: 0x0004 line: 1172 file: /mnt/d/Projects/mitosis-keyboard/nRF5_SDK_11/mitosis/mitosis-bluetooth/main.c
  RAM (rwx) :  ORIGIN = 0x20001100, LENGTH = 0x2900 //error: cannot init softdevice + cannot set name // ERROR! code: 12289 status: 0x3001 line: 646 file: /mnt/d/Projects/mitosis-keyboard/nRF5_SDK_11/mitosis/mitosis-bluetooth/main.c
  RAM (rwx) :  ORIGIN = 0x20001000, LENGTH = 0x5000 //sigtrap
  RAM (rwx) :  ORIGIN = 0x20000100, LENGTH = 0x3900 //passes uart init, then sigtrap
  RAM (rwx) :  ORIGIN = 0x20000E00, LENGTH = 0x3200 //error: cannot init softdevice
  RAM (rwx) :  ORIGIN = 0x20001800, LENGTH = 0x2800 //error: cannot init softdevice + cannot set name
  RAM (rwx) :  ORIGIN = 0x20001A00, LENGTH = 0x2600 //same
*/

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1b000, LENGTH = 0x25000
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 0x2000
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"