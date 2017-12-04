#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>

void kmain(uint64_t multiboot_magic, void *multiboot_data)
{
  serial_init(PORT_COM1);
  vga_init();
  debug_printf("kernel loaded\n");


  parse_multiboot(multiboot_magic, P2V(multiboot_data));

  debug_printf("Hello from debug printing function!\n");
  debug_printf("A number:%d\n", 12345);
  debug_printf("Multiboot data: %x, magic: %x\n", multiboot_data, multiboot_magic);

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

