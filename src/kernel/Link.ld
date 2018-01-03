ENTRY(_start)

KERNEL_OFFSET = 0xFFFFFF8000000000;
KERNEL_START = 0x10000;

SECTIONS
{
  . = KERNEL_START + KERNEL_OFFSET;
  kernel_start = .;
  .text : AT(ADDR(.text) - KERNEL_OFFSET)
  {
    *(.multiboot)
    *(.text)
  }
  .rodata : AT(ADDR(.rodata) - KERNEL_OFFSET)
  {
    *(.rodata*)
  }
  .data : AT(ADDR(.data) - KERNEL_OFFSET)
  {
    *(.data)
  }
  .bss : AT(ADDR(.bss) - KERNEL_OFFSET)
  {
    *(.COMMON)
    *(.bss)
  }
  kernel_end = .;
}
