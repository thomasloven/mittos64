#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>
#include <process.h>
#include <scheduler.h>

void thread_function()
{
  while(1);
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

  struct process *p1 = new_process((void (*)(void))0x10000);
  uint64_t page = pmm_alloc();
  vmm_set_page(p1->P4, 0x10000, page, PAGE_WRITE | PAGE_PRESENT | PAGE_USER);
  memcpy(P2V(page), (void *)(uintptr_t)thread_function, PAGE_SIZE);

  ready(p1);

  debug_ok("Boot \"Complete\"\n");

  start_scheduler();

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

