#include <memory.h>

uintptr_t *first = 0;

void pmm_free(void *c)
{
  c = (void *)P2V(c);
  *(uintptr_t **)c = first;
  first = c;
}

void *pmm_alloc()
{
  void *c = first;
  first = c?*(uintptr_t **)c:0;
  c = (void *)V2P(c);
  return c;
}
