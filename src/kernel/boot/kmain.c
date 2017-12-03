#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>

void kmain()
{
  serial_init(PORT_COM1);
  vga_init();

  debug_printf("Hello from debug printing function!\n");
  debug_printf("A number:%d\n", 12345);
  for(;;);
}

