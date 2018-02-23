#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>
#include <process.h>
#include <scheduler.h>
#include <smp.h>

void thread_function()
{
  char *p = 0xBADC0FFEE;
  while(1)
    p[0] = 1;
}


void kmain(uint64_t multiboot_magic, void *multiboot_data)
{
  serial_init(PORT_COM1);
  vga_init();
  debug_info("kernel loaded\n");

  multiboot_init(multiboot_magic, P2V(multiboot_data));

  debug_info("Kernel was loaded with command line \"%s\", by <%s>\n", kernel_boot_data.commandline, kernel_boot_data.bootloader);

  memory_init();

  cpu_init();

  acpi_init();

  struct process *p1 = new_process((void (*)(void))0x10000);
  procmm_brk(p1, (void *)0x10100);
  memcpy_to_p4(p1->P4, (void *)0x10000, (void *)(uintptr_t)thread_function, 100);
  ready(p1);

  struct process *p2 = new_process((void (*)(void))0x10000);
  procmm_brk(p2, (void *)0x10100);
  memcpy_to_p4(p2->P4, (void *)0x10000, (void *)(uintptr_t)thread_function, 100);
  ready(p2);

  debug_ok("Boot \"Complete\"\n");

  start_scheduler();

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

