#include <multiboot.h>
.intel_syntax noprefix

.section .multiboot
.align 0x1000
MultiBootHeader1:
.long MBOOT1_MAGIC
.long MBOOT1_FLAGS
.long MBOOT1_CS

.align 0x1000
MultiBootHeader2:
.long MBOOT2_MAGIC
.long MBOOT2_ARCH
.long MBOOT2_LENGTH
.long MBOOT2_CHECKSUM

.short 0
.short 0
.long 8
MultiBootHeader2End:

.section .text
.global _start
.code32
_start:
cli

# Save the multiboot magic number
mov ecx, eax

// Print a red exclamation mark in top left corner
mov eax, 0x0421
mov short [0xB8000], ax
// and loop infinitely
jmp $
