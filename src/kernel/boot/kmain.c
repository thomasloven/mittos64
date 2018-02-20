#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>
#include <thread.h>
#include <scheduler.h>

int *thread_id = (int *)0x20000;
void thread_function()
{
  *thread_id = thread()->tid;

  while(1)
  {
    debug("Thread %d\n", *thread_id);
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

  struct thread *th1 = new_thread(thread_function);
  vmm_set_page(th1->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);
  struct thread *th2 = new_thread(thread_function);
  vmm_set_page(th2->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);
  struct thread *th3 = new_thread(thread_function);
  vmm_set_page(th3->P4, 0x20000, pmm_alloc(), PAGE_WRITE | PAGE_PRESENT);

  ready(th1);
  ready(th2);
  ready(th3);

  debug_ok("Boot \"Complete\"\n");

  start_scheduler();

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

