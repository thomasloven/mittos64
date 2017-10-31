#include <multiboot.h>
.intel_syntax noprefix

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
