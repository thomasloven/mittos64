#pragma once
#include <queue.h>
#include <scheduler.h>
#include <stdint.h>
#include <interrupts.h>

struct process
{
  uint64_t pid;
  void *stack_ptr;
  uint64_t state;
  uint64_t P4;
  QUEUE_SPOT(runQ);
  uint8_t stack[];
};

struct process *process();

struct process *new_process(void (*function)(void));
void yield();
void start_scheduler();

void switch_stack(void *old_ptr, void *new_ptr);
