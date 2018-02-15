#include <memory.h>
#include <multiboot.h>
#include <debug.h>

void memory_init()
{
  uintptr_t start, end;
  uint32_t type, i = 0;
  debug_info("Parsing memory map\n");
  while(!multiboot_get_memory_area(i++, &start, &end, &type))
  {
    debug("0x%016x-0x%016x (%d)\n", start, end, type);
    for(uintptr_t p = start; p < end; p += PAGE_SIZE)
    {
      if(p >= V2P(&kernel_start) && p < V2P(&kernel_end))
        continue;

      uintptr_t page = vmm_get_page(&BootP4, (uintptr_t)P2V(p));
      if(page == (uintptr_t)-1 || !(page & PAGE_PRESENT))
      {
        uint16_t flags = PAGE_GLOBAL | PAGE_HUGE | PAGE_WRITE;
        touch_page(&BootP4, (uintptr_t)P2V(p), flags);
        vmm_set_page(&BootP4, (uintptr_t)P2V(p), p, flags | PAGE_PRESENT);
      }

      if(type == MMAP_FREE)
        pmm_free(p);
    }
  }
}
