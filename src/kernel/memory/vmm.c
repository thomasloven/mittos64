#include <memory.h>

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uint64_t)addr & ~FLAGS_MASK)

union PTE {
  uint64_t value;
  struct {
    uint64_t present:1;
    uint64_t write:1;
    uint64_t user:1;
    uint64_t write_through:1;
    uint64_t nocache:1;
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t huge:1;
    uint64_t global:1;
    uint64_t flags:3;
  };
};

#define PT(ptr) ((union PTE *)P2V(MASK_FLAGS(ptr)))
// Get the entry correspoding to address addr in page dir P4
// for P4 table (P4E), P3 table (P3E) and so on.
// Note: Those macros requires variables to be named
#define P4E (PT(P4)[P4_OFFSET(addr)])
#define P3E PT(P4E.value)[P3_OFFSET(addr)]
#define P2E PT(P3E.value)[P2_OFFSET(addr)]
#define P1E PT(P2E.value)[P1_OFFSET(addr)]

static int page_exists(uint64_t P4, uint64_t addr)
{
  if(P4 && P4E.present && P3E.present && P2E.present)
    return 1;
  return 0;
}

uint64_t vmm_get_page(uint64_t P4, uint64_t addr)
{
  if(page_exists(P4, addr))
  {
    if(P2E.huge)
      return P2E.value;
    return P1E.value;
  }
  return -1;
}

int vmm_set_page(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags, int touch)
{
  if(flags & PAGE_HUGE)
  {
    if(!(P4 && P4E.present && P3E.present))
    {
      if(!touch)
        return -1;
      touch_page(P4, addr, flags);
    }

    // Don't overwrite a non-huge page with a huge one
    if(P2E.present && !P2E.huge)
      return -1;
    P2E.value = page | flags;
    return 0;
  }

  if(!page_exists(P4, addr))
  {
    if(!touch)
      return -1;
    touch_page(P4, addr, flags);
  }

  P1E.value = page | flags;
  return 0;
}

int touch_page(uint64_t P4, uint64_t addr, uint16_t flags)
{
  if(!P4) return -1;

  int huge=(flags & PAGE_HUGE)?1:0;
  flags ^= PAGE_HUGE*huge;

  if((!P4E.present) && (!(P4E.value = pmm_calloc())))
    return -1;
  P4E.value |= flags | PAGE_PRESENT;

  if((!P3E.present) && (!(P3E.value = pmm_calloc())))
    return -1;
  P3E.value |= flags | PAGE_PRESENT;

  if(huge) return 0;

  if((!P2E.present) && (!(P2E.value = pmm_calloc())))
    return -1;
  P2E.value |= flags | PAGE_PRESENT;

  return 0;
}

void free_page(uint64_t P4, uint64_t addr, int free)
{
  if(!page_exists(P4, addr))
    return;

  union PTE *pt;

  if(P2E.huge)
  {
    P2E.value = 0;

    if(!free)
      return;
  } else {
    P1E.value = 0;

    if(!free)
      return;

    pt = PT(P2E.value);
    for(int i = 0; i < ENTRIES_PER_PT; i++)
      if(pt[i].value)
        return;
    pmm_free(MASK_FLAGS(P2E.value));
    P2E.value = 0;
  }

  pt = PT(P3E.value);
  for(int i = 0; i < ENTRIES_PER_PT; i++)
    if(pt[i].value)
      return;
  pmm_free(MASK_FLAGS(P3E.value));
  P3E.value = 0;

  pt = PT(P4E.value);
  for(int i = 0; i < ENTRIES_PER_PT; i++)
    if(pt[i].value)
      return;
  pmm_free(MASK_FLAGS(P4E.value));
  P4E.value = 0;
}
