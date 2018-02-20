#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>
#include <process.h>
#include <scheduler.h>

int *pid = (int *)0x20000;
void thread_function()
{
  *pid = process()->pid;

  while(1)
  {
    debug("Process %d\n", *pid);
    yield();
  }
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

  struct process *p1 = new_process(thread_function);
  vmm_set_page(p1->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);
  struct process *p2 = new_process(thread_function);
  vmm_set_page(p2->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);
  struct process *p3 = new_process(thread_function);
  vmm_set_page(p3->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);

  ready(p1);
  ready(p2);
  ready(p3);

  debug_ok("Boot \"Complete\"\n");

  start_scheduler();

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

