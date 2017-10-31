#include <multiboot.h>
.section .multiboot

.align 0x4
MultibootHeader1:
.long MBOOT1_MAGIC
.long MBOOT1_FLAGS
.long MBOOT1_CS

.align 0x8
MultibootHeader2:
.long MBOOT2_MAGIC
.long MBOOT2_ARCH
.long MBOOT2_LENGTH
.long MBOOT2_CHECKSUM

.short 0
.short 0
.long 8
MultibootHeader2End:
