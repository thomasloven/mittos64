#pragma once
#include <queue.h>
#include <scheduler.h>
#include <stdint.h>

struct thread
{
  uintptr_t stack_ptr;
  uint64_t tid;
  uint64_t state;
  QUEUE_SPOT(READYQ);
};


struct thread *new_thread(void (*function)(void));
void yield();
struct thread *current_thread();

void switch_stack(void *old_ptr, void *new_ptr);
