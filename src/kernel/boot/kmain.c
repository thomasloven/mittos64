#include <memory.h>

void clear_screen()
{
  unsigned char *vidmem = (void *)(0xB8000 + KERNEL_OFFSET);
  for(int i=0; i < 80*24*2; i++)
    *vidmem++ = 0;
}

void print_string(char *str)
{
  unsigned char *vidmem = (void *)(0xB8000 + KERNEL_OFFSET);
  while(*str)
  {
    *vidmem++ = *str++;
    *vidmem++ = 0x7;
  }
}

void kmain()
{
  clear_screen();
  print_string("Hello from c, world!");
  for(;;);
}

