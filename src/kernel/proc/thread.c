#include <thread.h>
#include <scheduler.h>

struct thread dummy;
struct thread *current_thread = 0;

uint64_t tid = 1;
struct thread *new_thread(void (*function)(void))
{
  struct thread *th = P2V(pmm_alloc());

  th->tcb.tid = tid++;
  th->RBP = (uint64_t)&th->RBP2;
  th->ret = (uint64_t)function;
  th->tcb.stack_ptr = (uint64_t)&th->RBP;

  return th;
}

void switch_thread(struct thread *old, struct thread *new)
{
  current_thread = new;
  swtch(&old->tcb.stack_ptr, &new->tcb.stack_ptr);
}

void yield()
{
  struct thread *old, *new;

  old = current_thread;
  if(old)
    ready(old);
  else
    old = &dummy;
  while(!(new = scheduler_next()));

  switch_thread(old, new);
}

int get_tid()
{
  return CURRENT_THREAD()->tcb.tid;
}
