#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>

void kmain(uint64_t multiboot_magic, void *multiboot_data)
{
  serial_init(PORT_COM1);
  vga_init();
  debug_info("kernel loaded\n");

  multiboot_init(multiboot_magic, P2V(multiboot_data));

  debug_info("Kernel was loaded with command line \"%s\", by <%s>\n", kernel_boot_data.commandline, kernel_boot_data.bootloader);

  memory_init();

  cpu_init();


  debug_ok("Boot \"Complete\"\n");

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

