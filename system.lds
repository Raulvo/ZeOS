/*
 *  ZeOS
 *  system.lds - Linker Script for ZeOS system image.
 */

SECTIONS
{
  . = 0x10000;
  .text : {
    /* 16-Byte gap to store user code size. Boot sequence jumps to 0x10010 linear address. */
    . += 0x10;
    *(.text.main.system);
    *(.text) 
  }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = ALIGN(4096);              /* task_structs array*/
  .data.task : { *(.data.task) }

}
