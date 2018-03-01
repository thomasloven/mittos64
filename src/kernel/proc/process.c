#include <process.h>
#include <scheduler.h>
#include <memory.h>
#include <cpu.h>

struct swtch_stack
{
  uint64_t RBP;
  uint64_t RBX;
  uint64_t R12;
  uint64_t R13;
  uint64_t R14;
  uint64_t R15;
  uint64_t isr_return_arg;
  uint64_t RBP2;
  uint64_t ret;
  registers r;
}__attribute__((packed));

struct process *sched_proc = 0;
struct process *_proc = 0;

void procmm_init(struct process *p);

uint64_t next_pid = 1;
struct process *new_process(void (*function)(void))
{
  struct process *proc = P2V(pmm_calloc());
  proc->pid = next_pid++;
  proc->stack_ptr = incptr(proc, PAGE_SIZE - sizeof(struct swtch_stack));
  procmm_init(proc);

  struct swtch_stack *stk = proc->stack_ptr;
  stk->RBP = (uint64_t)&stk->RBP2;

  stk->ret = (uint64_t)isr_return;
  stk->isr_return_arg = (uint64_t)&stk->r;

  stk->r.rip = (uint64_t)function;
  stk->r.cs = 0x10 | 3;
  stk->r.ss = 0x18 | 3;
  stk->r.rflags = 3<<12;
  stk->r.rsp = KERNEL_OFFSET;

  return proc;
}

struct process *process()
{
  return _proc;
}

void yield()
{
  switch_stack(&_proc->stack_ptr, &sched_proc->stack_ptr);
}

void scheduler()
{
  while(1)
  {
    struct process *new = 0;
    while(!(new = scheduler_next()));

    _proc = new;
    write_cr3(new->P4);
    interrupt_stack(incptr(new, PAGE_SIZE));
    switch_stack(&sched_proc->stack_ptr, &new->stack_ptr);

    ready(_proc);
    _proc = 0;
  }
}

void start_scheduler()
{
  sched_proc = P2V(pmm_calloc());
  sched_proc->pid = (uint64_t)-1;
  sched_proc->stack_ptr = incptr(sched_proc, PAGE_SIZE - sizeof(struct swtch_stack) + sizeof(registers));
  sched_proc->P4 = kernel_P4;

  struct swtch_stack *stk = sched_proc->stack_ptr;
  stk->RBP = (uint64_t)&stk->RBP2;

  stk->ret = (uint64_t)scheduler;

  uint64_t stack;
  switch_stack(&stack, &sched_proc->stack_ptr);
}
