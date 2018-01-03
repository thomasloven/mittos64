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

  cpu_init();


  uintptr_t start, end;
  uint32_t type, i=0;
  while(!multiboot_get_memory_area(i++, &start, &end, &type))
  {
    debug("Mem %d 0x%x-0x%x\n", type, start, end);
    for(uintptr_t p = start; p < end; p += PAGE_SIZE)
    {
      if(p >= V2P(&kernel_start) && p < V2P(&kernel_end))
        continue;
      if(vmm_get_page(&BootP4, (uintptr_t)P2V(p)) == (uintptr_t)-1)
      {
        touch_page(&BootP4, (uintptr_t)P2V(p), PAGE_GLOBAL | PAGE_WRITE | PAGE_HUGE);
        vmm_set_page(&BootP4, (uintptr_t)P2V(p), p, PAGE_GLOBAL | PAGE_HUGE | PAGE_WRITE | PAGE_PRESENT);
      }
      if(type == 1)
        pmm_free(p);
    }
  }

  debug_ok("Boot \"Complete\"\n");

  PANIC("Reached end of kernel main function\n");
  for(;;);
}

