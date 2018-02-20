#include <memory.h>
#include <serial.h>
#include <vga.h>
#include <debug.h>
#include <multiboot.h>
#include <cpu.h>
#include <interrupts.h>
#include <thread.h>
#include <scheduler.h>

int thread_id;
void thread_function()
{
  thread_id = thread()->tid;

  while(1)
  {
    debug("Thread %d\n", thread_id);
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

  uintptr_t P4 = new_P4();
  write_cr3(P4);

  debug_ok("Boot \"Complete\"\n");

  ready(new_thread(thread_function));
  ready(new_thread(thread_function));
  ready(new_thread(thread_function));

  start_scheduler();

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

