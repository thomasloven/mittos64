#include <memory.h>
#include <debug.h>

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uint64_t)addr & ~FLAGS_MASK)

#define PRESENT(p) (p & PAGE_PRESENT)

#define PT(ptr) ((uint64_t *)P2V(MASK_FLAGS(ptr)))
// Get the entry correspoding to address addr in page dir P4
// for P4 table (P4E), P3 table (P3E) and so on.
// Note: Those macros requires variables to be named
#define P4E (PT(P4)[P4_OFFSET(addr)])
#define P3E PT(P4E)[P3_OFFSET(addr)]
#define P2E PT(P3E)[P2_OFFSET(addr)]
#define P1E PT(P2E)[P1_OFFSET(addr)]

uint64_t new_P4()
{
  uint64_t p4 = pmm_alloc();
  memcpy(P2V(p4), (void *)kernel_P4, PAGE_SIZE);
  return p4;
}

static int page_exists(uint64_t P4, uint64_t addr)
{
  if(P4 && PRESENT(P4E) && PRESENT(P3E) && PRESENT(P2E))
    return 1;
  return 0;
}

static int touch_page(uint64_t P4, uint64_t addr, uint16_t flags)
{
  if(!P4) return -1;

  if((!PRESENT(P4E)) && (!(P4E = pmm_calloc())))
    return -1;
  P4E |= flags | PAGE_PRESENT;

  if((!PRESENT(P3E)) && (!(P3E = pmm_calloc())))
    return -1;
  P3E |= flags | PAGE_PRESENT;

  if((!PRESENT(P2E)) && (!(P2E = pmm_calloc())))
    return -1;
  P2E |= flags | PAGE_PRESENT;

  return 0;
}

uint64_t vmm_get_page(uint64_t P4, uint64_t addr)
{
  if(page_exists(P4, addr))
  {
    return P1E;
  }
  return -1;
}

int vmm_set_page(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags)
{
  if(!page_exists(P4, addr))
  {
    if(touch_page(P4, addr, flags))
      return -1;
  }

  P1E = page | flags;
  return 0;
}

void vmm_clear_page(uint64_t P4, uint64_t addr, int free)
{
  if(!page_exists(P4, addr))
    return;

  uint64_t *pt;

  P1E = 0;

  if(!free)
    return;

  pt = PT(P2E);
  for(int i = 0; i < ENTRIES_PER_PT; i++)
    if(pt[i])
      return;
  pmm_free(MASK_FLAGS(P2E));
  P2E = 0;

  pt = PT(P3E);
  for(int i = 0; i < ENTRIES_PER_PT; i++)
    if(pt[i])
      return;
  pmm_free(MASK_FLAGS(P3E));
  P3E = 0;

  pt = PT(P4E);
  for(int i = 0; i < ENTRIES_PER_PT; i++)
    if(pt[i])
      return;
  pmm_free(MASK_FLAGS(P4E));
  P4E = 0;
}

#define min(a,b) (((a) < (b))?(a):(b))
#define offset(p) ((uintptr_t)(p) % PAGE_SIZE)
#define remaining(p) (PAGE_SIZE - offset(p))
size_t memcpy_to_p4(uint64_t P4, void *dst, void *src, size_t n)
{
  size_t copied = 0;
  while(n)
  {
    size_t bytes = min(remaining(dst), n);
    uintptr_t page = vmm_get_page(P4, (uintptr_t)dst);
    if(!PAGE_EXIST(page))
      return copied;

    void *to = P2V(MASK_FLAGS(page) + offset(dst));
    memcpy(to, src, bytes);

    copied += bytes;
    n -= bytes;
    dst = incptr(dst, bytes);
    src = incptr(src, bytes);
  }
  return copied;
}

size_t memcpy_from_p4(void *dst, uint64_t P4, void *src, size_t n)
{
  size_t copied = 0;
  while(n)
  {
    size_t bytes = min(remaining(src), n);
    uintptr_t page = vmm_get_page(P4, (uintptr_t)src);
    if(!PAGE_EXIST(page))
      return copied;

    void *from = P2V(MASK_FLAGS(page) + offset(src));
    memcpy(dst, from, bytes);

    copied += bytes;
    n -= bytes;
    dst = incptr(dst, bytes);
    src = incptr(src, bytes);
  }
  return copied;
}
