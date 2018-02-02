#include <gdt.h>
#include <memory.h>
.intel_syntax noprefix

.section .rodata
.align PAGE_SIZE
.global BootGDT
.global BootGDTp
BootGDT:
  .long 0, 0
  .long 0, (GDT_PRESENT | GDT_DPL(0) | GDT_CODE | GDT_LONG)
BootGDTp:
  .short 2*8-1
  .quad offset BootGDT
