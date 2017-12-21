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
  page = (uintptr_t)V2P(page);
  return page;
}
