#include <memory.h>

uintptr_t first = 0;

void pmm_free(uintptr_t page)
{
  page = (uintptr_t)P2V(page);
  *(uintptr_t *)page = first;
  first = page;
}

uintptr_t pmm_alloc()
{
  uintptr_t page = first;
  first = page?*(uintptr_t *)page:0;
  page = (uintptr_t)(page?V2P(page):0);
  return page;
}

uintptr_t pmm_calloc()
{
  uintptr_t page = pmm_alloc();
  memset(P2V(page), 0, 0x1000);
  return page;
}
