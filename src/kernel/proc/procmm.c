#include <process.h>
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <debug.h>
#include <memory.h>
#include <cpu.h>

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

registers *proc_pagefault(registers *r)
{
  if(!(r->rflags & (3<<12)))
  {
    debug_error("Page fault in kernel!\n");
    debug("Interrupt number: %d Error code: %d\n", r->int_no, r->err_code);
    debug_print_registers(r);
    PANIC("Page fault in kernel\n");
  }

  PANIC("Page fault in process\n");

  return r;
}
