#include <thread.h>

struct thread *threads[8];
int current_tid = -1;
int tid = 0;

struct thread dummy;

struct thread *new_thread(void (*function)())
{
  struct thread *th = threads[tid] = P2V(pmm_alloc());

  th->tcb.tid = tid++;
  th->RBP = (uint64_t)&th->RBP2;
  th->ret = (uint64_t)function;
  th->tcb.stack_ptr = (uint64_t)&th->RBP;

  return th;
}

void switch_thread(struct thread *old, struct thread *new)
{
  swtch(&old->tcb.stack_ptr, &new->tcb.stack_ptr);
}

void yield()
{
  struct thread *old, *new;
  if(current_tid == -1)
    old = &dummy;
  else
    old = threads[current_tid];

  current_tid++;
  if(current_tid == tid)
    current_tid = 0;

  new = threads[current_tid];

  switch_thread(old, new);
}

int get_tid()
{
  return CURRENT_THREAD()->tcb.tid;
}
