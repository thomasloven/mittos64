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
  uint64_t RBP2;
  uint64_t ret;
}__attribute__((packed));

struct process *sched_proc = 0;
struct process *_proc = 0;

uint64_t next_pid = 1;
struct process *new_process(void (*function)(void))
{
  struct process *proc = P2V(pmm_calloc());
  proc->pid = next_pid++;
  proc->stack_ptr = incptr(proc, PAGE_SIZE - sizeof(struct swtch_stack));
  proc->P4 = new_P4();

  struct swtch_stack *stk = proc->stack_ptr;
  stk->RBP = (uint64_t)&stk->RBP2;
  stk->ret = (uint64_t)function;

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
    switch_stack(&sched_proc->stack_ptr, &new->stack_ptr);

    ready(_proc);
    _proc = 0;
  }
}

void start_scheduler()
{
  sched_proc = new_process(scheduler);
  sched_proc->pid = (uint64_t)-1;

  uint64_t stack;
  switch_stack(&stack, &sched_proc->stack_ptr);
}
