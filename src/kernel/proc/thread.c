#include <thread.h>
#include <scheduler.h>

struct thread dummy;
struct thread *_current_thread = 0;

uint64_t next_tid = 1;
struct thread *new_thread(void (*function)(void))
{
  struct thread_stack *stk = P2V(pmm_alloc());
  struct thread *th = &stk->tcb;

  th->tid = next_tid++;
  stk->RBP = (uint64_t)&stk->RBP2;
  stk->ret = (uint64_t)function;
  th->stack_ptr = (uint64_t)&stk->RBP;

  return th;
}

struct thread *current_thread()
{
  return _current_thread;
}

void set_current_thread(struct thread *th)
{
  _current_thread = th;
}

void switch_thread(struct thread *old, struct thread *new)
{
  set_current_thread(new);
  swtch(&old->stack_ptr, &new->stack_ptr);
}

void yield()
{
  struct thread *old, *new;

  old = current_thread();
  if(old)
    ready(old);
  else
    old = &dummy;
  while(!(new = scheduler_next()));

  switch_thread(old, new);
}

int get_tid()
{
  return current_thread()->tid;
}
