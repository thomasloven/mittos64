#pragma once
#include <memory.h>


struct tcb
{
  uintptr_t stack_ptr;
  uint64_t tid;
  uint64_t state;
};

#define TCB_OFFSET (PAGE_SIZE - sizeof(struct tcb))
#define SWTCH_STACK_SIZE (0x8*8)

struct thread
{
  union {
    uint8_t stack[TCB_OFFSET];
    struct {
      uint8_t _stck[TCB_OFFSET-SWTCH_STACK_SIZE];
      uint64_t RBP;
      uint64_t RBX;
      uint64_t R12;
      uint64_t R13;
      uint64_t R14;
      uint64_t R15;
      uint64_t RBP2;
      uint64_t ret;
    };
  };
  struct tcb tcb;
};

extern struct thread *threads[];
int current_tid;

#define CURRENT_THREAD() (threads[current_tid])

struct thread *new_thread(void (*function)());
void yield();
int get_tid();

void swtch(void *, void *);
