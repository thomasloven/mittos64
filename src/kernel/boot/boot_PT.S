#include <x86_64/memory.h>
.intel_syntax noprefix

.section .data
.align PAGE_SIZE
.global BootP4
BootP4:
  .quad offset BootP3 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0x0
  .endr
BootP3:
  .quad offset BootP2 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0x0
  .endr
BootP2:
  .set i, 0
  .rept ENTRIES_PER_PT
    .quad (i << 21 ) + (PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE)
    .set i, (i+1)
  .endr
