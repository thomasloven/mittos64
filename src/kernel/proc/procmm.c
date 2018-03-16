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

  uint64_t fault_addr = read_cr2();
  uint64_t stack = cpu->proc->stack;
  if(fault_addr + PAGE_SIZE >= stack)
  {
    // Page fault happened just below stack. Add another page to it.
    // Unless it's about to run into brk.
    if(stack - PAGE_SIZE <= cpu->proc->brk)
      PANIC("Stack overflow in process %d\n", cpu->proc->pid);

    stack -= PAGE_SIZE;
    vmm_set_page(cpu->proc->P4, stack, pmm_alloc(), PAGE_USER | PAGE_WRITE | PAGE_PRESENT);
    cpu->proc->stack = stack;

    return r;
  }

  // Very uggly and very temporary way of signaling the kernel from a user mode process.
  // TODO: REMOVE THIS CRAP!
  if(read_cr2() == 0xBADC0FFEE)
  {
    debug("Bad signal from process %d\n", cpu->proc->pid);
    yield();
    return r;
  }
  PANIC("Page fault in process\n");
  return r;
}
