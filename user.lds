/*
 *  ZeOS - jcosta septembre 2006
 *  user.lds - Linker Script to create user memory image
 */

OUTPUT_FORMAT(elf32-i386)
ENTRY(main)
SECTIONS
{
  . = 0x100000; /* User CODE will start at this address */
    
  .text.main.user : { *(.text.main.user) }
  .text : { *(.text) }

  . = 0x108000; /* User DATA will start at this address */
  .rodata : { *(.rodata) }      /* Read Only Data */
  .data : { *(.data) }          /* Normal Data */
  .bss : { *(.bss) }            /* Uninitialized Data */
}
