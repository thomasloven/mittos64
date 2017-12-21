#include <memory.h>

uintptr_t *first = 0;

void pmm_free(void *c)
{
  c = (uintptr_t *)P2V(c);
  *(uintptr_t *)c = (uintptr_t)first;
  first = c;
}

void *pmm_alloc()
{
  void *c = first;
  first = (uintptr_t *)(c?*(uintptr_t *)c:0);
  c = (uintptr_t *)V2P(c);
  return c;
}
