#pragma once
#include <queue.h>
#include <scheduler.h>
#include <stdint.h>

struct thread
{
  uint64_t tid;
  void *stack_ptr;
  uint64_t state;
  QUEUE_SPOT(runQ);
  uint8_t stack[];
};

struct thread *thread();

struct thread *new_thread(void (*function)(void));
void yield();
void start_scheduler();

void switch_stack(void *old_ptr, void *new_ptr);
