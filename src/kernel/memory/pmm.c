#include <memory.h>

uint64_t first = 0;

void pmm_free(uint64_t page)
{
  page = (uint64_t)P2V(page);
  *(uint64_t *)page = first;
  first = page;
}

uint64_t pmm_alloc()
{
  uint64_t page = first;
  first = page?*(uint64_t *)page:0;
  page = (uint64_t)(page?V2P(page):0);
  return page;
}

uint64_t pmm_calloc()
{
  uint64_t page = pmm_alloc();
  memset(P2V(page), 0, 0x1000);
  return page;
}
