#include <process.h>
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <debug.h>
#include <memory.h>

void procmm_init(struct process *p)
{
  p->P4 = new_P4();
  p->brk = 0;
  p->stack = KERNEL_OFFSET;

}

uint64_t procmm_brk(struct process *p, void *addr)
{
  while((uint64_t)addr > p->brk)
  {
    vmm_set_page(p->P4, p->brk, pmm_alloc(), PAGE_USER | PAGE_WRITE | PAGE_PRESENT);
    p->brk += PAGE_SIZE;
  }
  return p->brk;
}
