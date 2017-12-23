#include <memory.h>

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uintptr_t)addr & ~FLAGS_MASK)

union PTE {
  uint64_t value;
  struct {
    uintptr_t flags:12;
  };
};

typedef union PTE PT[512];

#define PT(ptr) ((PT *)MASK_FLAGS(ptr->value))
#define P4e(pt, addr) (((PT *)pt)[P4_OFFSET(addr)])
#define P3e(pt, addr) PT(P4e(pt, addr))[P3_OFFSET(addr)]
#define P2e(pt, addr) PT(P3e(pt, addr))[P2_OFFSET(addr)]
#define P1e(pt, addr) PT(P2e(pt, addr))[P1_OFFSET(addr)]

uintptr_t vmm_get_page(void *P4, uintptr_t addr)
{
  if(P4
      && P4e(P4, addr)
      && P3e(P4, addr)
      && P2e(P4, addr)
    )
    return MASK_FLAGS(P1e(P4, addr)->value);
  else
    return 0;
}
