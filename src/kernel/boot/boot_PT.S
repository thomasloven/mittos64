#include <memory.h>
.intel_syntax noprefix

.section .data
.align PAGE_SIZE
.global BootP4
BootP4:
  .quad offset BootP3 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP3:
  .quad offset BootP2 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP2:
  .quad (PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
