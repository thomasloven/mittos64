#define MBOOT1_MAGIC 0x1BADB002
#define MBOOT1_PALIGN 0x01
#define MBOOT1_MEMINFO 0x02
#define MBOOT1_FLAGS (MBOOT1_PALIGN | MBOOT1_MEMINFO)
#define MBOOT1_CS -(MBOOT1_FLAGS + MBOOT1_MAGIC)

.intel_syntax noprefix

.section .multiboot
.align 0x1000
MultiBootHeader1:
.long MBOOT1_MAGIC
.long MBOOT1_FLAGS
.long MBOOT1_CS

.section .text
.global _start
.code32
_start:
cli

// Print a red exclamation mark in top left corner
mov eax, 0x0421
mov short [0xB8000], ax
// and loop infinitely
jmp $
