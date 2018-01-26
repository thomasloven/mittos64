#pragma once
#include <memory.h>


struct tcb
{
  uintptr_t stack_ptr;
  uint64_t tid;
  uint64_t state;
  struct thread *next;
};

#define TCB_OFFSET (PAGE_SIZE - sizeof(struct tcb))
#define SWTCH_STACK_SIZE (0x8*8)

struct thread
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

  struct tcb tcb;
}__attribute__((packed));

struct thread *new_thread(void (*function)(void));
void yield();
int get_tid();

void swtch(void *, void *);
