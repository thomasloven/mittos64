#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>

void kmain()
{
  serial_init(PORT_COM1);
  vga_init();

  debug("Hello from debug printing function!\n");
  debug("A number:%d\n", 12345);
  for(;;);
}

