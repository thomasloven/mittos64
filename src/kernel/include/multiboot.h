#pragma once

#define MBOOT2_MAGIC 0xE85250D6
#define MBOOT2_REPLY 0x36D76289
#define MBOOT2_ARCH 0
#define MBOOT2_LENGTH (Multiboot2HeaderEnd - Multiboot2Header)
#define MBOOT2_CHECKSUM -(MBOOT2_MAGIC + MBOOT2_ARCH + MBOOT2_LENGTH)
