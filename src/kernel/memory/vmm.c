#include <memory.h>

#define FLAGS_MASK (PAGE_SIZE-1)

struct PT {
  struct PT *p[ENTRIES_PER_PT];
}__attribute__((packed));

#define P(pt) ((struct PT *)((uintptr_t)pt & ~(FLAGS_MASK)))

#define P3(pt, addr) (P(pt)->p[P4_OFFSET(addr)])
#define P2(pt, addr) (P(P3(pt, addr))->p[P3_OFFSET(addr)])
#define P1(pt, addr) (P(P2(pt, addr))->p[P2_OFFSET(addr)])
#define P0(pt, addr) (P(P1(pt, addr))->p[P1_OFFSET(addr)])


uintptr_t vmm_get_page(uintptr_t *P4, uintptr_t addr)
{
  if(P4
      && P3(P4, addr)
      && P2(P4, addr)
      && P1(P4, addr)
    )
    return (uintptr_t)P0(P4, addr);
  return 0;
}
