ENTRY(_start)

SECTIONS
{
  .text :
  {
    *(.multiboot)
    *(.text)
  }
}
