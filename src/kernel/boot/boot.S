#include <multiboot.h>
.intel_syntax noprefix

.section .text
.global _start
.code32
_start:
cli

mov ecx, eax

// Print a red digit in the upper left corner to indicate the multiboot version
// If not loaded by multiboot, print an exclamation mark
cmp eax, MBOOT1_REPLY
je mboot1
cmp eax, MBOOT2_REPLY
je mboot2
mov eax, 0x0421 // Red exclamation point
jmp print

mboot1:
mov eax, 0x0431 // Red 1
jmp print

mboot2:
mov eax, 0x0432 // Red 2
jmp print

print:
mov short [0xB8000], ax // Place in upper left corner

// loop infinitely
jmp $
