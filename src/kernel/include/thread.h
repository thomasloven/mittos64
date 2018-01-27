#pragma once
#include <queue.h>
#include <scheduler.h>
#include <memory.h>

struct thread
{
  uintptr_t stack_ptr;
  uint64_t tid;
  uint64_t state;
  QUEUE_SPOT(READYQ);
};

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

struct thread *new_thread(void (*function)(void));
void yield();
struct thread *current_thread();

void switch_stack(void *old_ptr, void *new_ptr);
