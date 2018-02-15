#include <thread.h>
#include <scheduler.h>
#include <memory.h>

#define TCB_OFFSET (PAGE_SIZE - sizeof(struct thread))
#define SWTCH_STACK_SIZE (0x8*8)

struct thread_stack
{
  uint8_t stack[TCB_OFFSET-SWTCH_STACK_SIZE];
  uint64_t RBP;
  uint64_t RBX;
  uint64_t R12;
  uint64_t R13;
  uint64_t R14;
  uint64_t R15;
  uint64_t RBP2;
  uint64_t ret;

  struct thread tcb;
}__attribute__((packed));

#define thread_stack(th) (struct thread_stack *)((uintptr_t)th - TCB_OFFSET)

struct thread boot_thread;
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
  switch_stack(&old->stack_ptr, &new->stack_ptr);
}

void yield()
{
  struct thread *old, *new;

  old = current_thread();
  if(old)
    ready(old);
  else
    old = &boot_thread;
  while(!(new = scheduler_next()));

  switch_thread(old, new);
}
