.intel_syntax noprefix

.section .data
.align 0x1000
.global BootP4
BootP4:
  .quad offset BootP3 + 0x3
  .rept 511
    .quad 0x0
  .endr
BootP3:
  .quad offset BootP2 + 0x3
  .rept 511
    .quad 0x0
  .endr
BootP2:
  .set i, 0
  .rept 512
    .quad (i << 21 ) + 0x83
    .set i, (i+1)
  .endr

.section .rodata
.align 0x1000
.global BootGDT
.global BootGDTp
BootGDT:
  .quad 0
  .quad ((1<<47) + (1<<44) + (1<<41) + (1<<43) + (1<<53))
  .quad ((1<<47) + (1<<44) + (1<<41))
BootGDTp:
  .short 3*8-1
  .quad offset BootGDT
