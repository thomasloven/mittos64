ENTRY(_start)

SECTIONS
{
  .text : AT(ADDR(.text))
  {
    *(.multiboot)
    *(.text)
    . = ALIGN(4096);
  }
}
