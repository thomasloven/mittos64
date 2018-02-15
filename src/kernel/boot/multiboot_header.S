#define MBOOT2_MAGIC 0xE85250D6
#define MBOOT2_ARCH 0
#define MBOOT2_LENGTH (Multiboot2HeaderEnd - Multiboot2Header)
#define MBOOT2_CHECKSUM -(MBOOT2_MAGIC + MBOOT2_ARCH + MBOOT2_LENGTH)

.section .multiboot

.align 0x8
Multiboot2Header:
.long MBOOT2_MAGIC
.long MBOOT2_ARCH
.long MBOOT2_LENGTH
.long MBOOT2_CHECKSUM

.short 0
.short 0
.long 8
Multiboot2HeaderEnd:
