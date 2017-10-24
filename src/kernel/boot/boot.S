#define MBOOT1_MAGIC 0x1BADB002
#define MBOOT1_PALIGN 0x01
#define MBOOT1_MEMINFO 0x02
#define MBOOT1_FLAGS (MBOOT1_PALIGN | MBOOT1_MEMINFO)
#define MBOOT1_CS -(MBOOT1_FLAGS + MBOOT1_MAGIC)

#define MBOOT2_MAGIC 0xE85250D6
#define MBOOT2_ARCH 0
#define MBOOT2_LENGTH (MultiBootHeader2End - MultiBootHeader2)
#define MBOOT2_CHECKSUM -(MBOOT2_MAGIC + MBOOT2_ARCH + MBOOT2_LENGTH)

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

// Print a red exclamation mark in top left corner
mov eax, 0x0421
mov short [0xB8000], ax
// and loop infinitely
jmp $
