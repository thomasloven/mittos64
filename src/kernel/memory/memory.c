#include <memory.h>
#include <multiboot.h>
#include <debug.h>

uint64_t kernel_P4;

void memory_init()
{
  kernel_P4 = (uint64_t)&BootP4;
  uint64_t start, end;
  uint32_t type, i = 0;
  debug_info("Parsing memory map\n");
  while(!multiboot_get_memory_area(i++, &start, &end, &type))
  {
    debug("0x%016x-0x%016x (%d)\n", start, end, type);
    for(uint64_t p = start; p < end; p += PAGE_SIZE)
    {
      if(p >= V2P(&kernel_start) && p < V2P(&kernel_end))
        continue;
      if(multiboot_page_used(p))
        continue;

      uint64_t addr = (uint64_t)P2V(p);
      uint64_t page = vmm_get_page(kernel_P4, addr);
      if(!PAGE_EXIST(page) || !(page & PAGE_PRESENT))
      {
        uint16_t flags = PAGE_GLOBAL | PAGE_HUGE | PAGE_WRITE;
        vmm_set_page(kernel_P4, addr, p, flags | PAGE_PRESENT, 1);
      }

      if(type == MMAP_FREE)
        pmm_free(p);
    }
  }
}
