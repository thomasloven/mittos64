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
  debug_info("Here's some info\n");
  debug_ok("This thing worked well!\n");
  debug_warning("Careful!\n");
  debug_error("Something went wrong!\n");

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

