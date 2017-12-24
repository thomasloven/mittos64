#include <memory.h>

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uintptr_t)addr & ~FLAGS_MASK)

union PTE {
  uint64_t value;
  struct {
    uintptr_t flags:12;
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
      && P4e(P4, addr).value
      && P3e(P4, addr).value
      && P2e(P4, addr).value
    )
    return MASK_FLAGS(P1e(P4, addr).value);
  else
    return 0;
}
