#include <memory.h>
.intel_syntax noprefix

.section .data
.align PAGE_SIZE
.global BootP4
BootP4:
  .quad offset V2P(BootP3) + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 2
    .quad 0
  .endr
  .quad offset V2P(BootP3) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL)
BootP3:
  .quad offset V2P(BootP2) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP2:
  .quad offset V2P(BootP1) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP1:
  .set i, 0
  .rept ENTRIES_PER_PT
    .quad (i << 12) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL)
    .set i, (i+1)
  .endr
