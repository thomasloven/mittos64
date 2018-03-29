#include <thread.h>
#include <scheduler.h>
#include <memory.h>

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

struct thread *sched_th = 0;
struct thread *_thread = 0;

uint64_t next_tid = 1;
struct thread *new_thread(void (*function)(void))
{
  struct thread *th = P2V(pmm_calloc());
  th->tid = next_tid++;
  th->stack_ptr = incptr(th, PAGE_SIZE - sizeof(struct swtch_stack));

  struct swtch_stack *stk = th->stack_ptr;
  stk->RBP = (uint64_t)&stk->RBP2;
  stk->ret = (uint64_t)function;

  return th;
}

struct thread *thread()
{
  return _thread;
}

void yield()
{
  switch_stack(&_thread->stack_ptr, &sched_th->stack_ptr);
}

void scheduler()
{
  while(1)
  {
    struct thread *new = 0;
    while(!(new = scheduler_next()));

    _thread = new;
    switch_stack(&sched_th->stack_ptr, &new->stack_ptr);

    ready(_thread);
    _thread = 0;
  }
}

void start_scheduler()
{
  sched_th = new_thread(scheduler);
  sched_th->tid = (uint64_t)-1;

  uint64_t stack;
  switch_stack(&stack, &sched_th->stack_ptr);
}
