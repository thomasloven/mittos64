#include <memory.h>

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uintptr_t)addr & ~FLAGS_MASK)

union PTE {
  uint64_t value;
  struct {
    uintptr_t present:1;
    uintptr_t write:1;
    uintptr_t user:1;
    uintptr_t write_through:1;
    uintptr_t nocache:1;
    uintptr_t accessed:1;
    uintptr_t dirty:1;
    uintptr_t huge:1;
    uintptr_t global:1;
    uintptr_t flags:3;
  };
};

#define PT(ptr) ((union PTE *)MASK_FLAGS(ptr))
#define P4e(pt, addr) (PT(pt)[P4_OFFSET(addr)])
#define P3e(pt, addr) PT(P4e(pt, addr).value)[P3_OFFSET(addr)]
#define P2e(pt, addr) PT(P3e(pt, addr).value)[P2_OFFSET(addr)]
#define P1e(pt, addr) PT(P2e(pt, addr).value)[P1_OFFSET(addr)]

uintptr_t vmm_get_page(void *P4, uintptr_t addr)
{
  if(P4
      && P4e(P4, addr).present
      && P3e(P4, addr).present
      && P2e(P4, addr).present
    )
    return P1e(P4, addr).value;
  else
    return -1;
}

int vmm_set_page(void *P4, uintptr_t addr, uintptr_t page, uint16_t flags)
{
  if(!(P4
      && P4e(P4, addr).present
      && P3e(P4, addr).present
      && P2e(P4, addr).present
    ))
    return -1;

  P1e(P4, addr).value = page | flags;
  return 0;
}

int touch_page(void *P4, uintptr_t addr, uint16_t flags)
{
  if(!P4) return -1;

  if((!P4e(P4, addr).present)
      && (!(P4e(P4, addr).value = pmm_alloc())))
    return -1;
  P4e(P4, addr).value |= flags | PAGE_PRESENT;

  if((!P3e(P4, addr).present)
      && (!(P3e(P4, addr).value = pmm_alloc())))
    return -1;
  P3e(P4, addr).value |= flags | PAGE_PRESENT;

  if((!P2e(P4, addr).present)
      && (!(P2e(P4, addr).value = pmm_alloc())))
    return -1;
  P2e(P4, addr).value |= flags | PAGE_PRESENT;

  return 0;
}

void free_page(void *P4, uintptr_t addr, int free)
{
  (void)free;
  if(!(P4
      && P4e(P4, addr).present
      && P3e(P4, addr).present
      && P2e(P4, addr).present
    ))
    return;
  P1e(P4, addr).value = 0;
}
