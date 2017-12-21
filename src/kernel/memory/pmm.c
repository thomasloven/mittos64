#include <memory.h>

uintptr_t *first = 0;

void pmm_free(void *c)
{
  *(uintptr_t *)c = (uintptr_t)first;
  first = c;
}

void *pmm_alloc()
{
  void *c = first;
  first = (uintptr_t *)(c?*(uintptr_t *)c:0);
  return c;
}
