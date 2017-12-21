#include <memory.h>

uintptr_t *first = 0;

void pmm_free(void *c)
{
  first = c;
}

void *pmm_alloc()
{
  return first;
}
