#pragma once
#include <queue.h>
#include <scheduler.h>
#include <stdint.h>
#include <interrupts.h>
#include <cpu.h>

struct process
{
  uint64_t pid;
  void *stack_ptr;
  uint64_t state;

  uint64_t P4;
  uint64_t brk;
  uint64_t stack;
  QUEUE_SPOT(runQ);
};

struct process *new_process(void (*function)(void));
void yield();
void start_scheduler();

void switch_stack(void *old_ptr, void *new_ptr);

uint64_t procmm_brk(struct process *p, void *addr);
