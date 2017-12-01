#include <memory.h>
#include <serial.h>
#include <debug.h>

void clear_screen()
{
  unsigned char *vidmem = P2V(0xB8000);
  for(int i=0; i < 80*24*2; i++)
    *vidmem++ = 0;
}

void print_string(char *str)
{
  unsigned char *vidmem = P2V(0xB8000);
  while(*str)
  {
    *vidmem++ = *str++;
    *vidmem++ = 0x7;
  }
}

void vga_write(char c)
{
// TODO: DUMMY
  (void)c;
}

void kmain()
{
  clear_screen();
  print_string("Hello from c, world!");

  serial_init(PORT_COM1);
  debug_printf("Hello from debug printing function!\n");
  debug_printf("A number:%d\n", 12345);
  for(;;);
}

