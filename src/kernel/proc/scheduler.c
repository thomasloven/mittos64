#include <scheduler.h>
#include <queue.h>
#include <process.h>

QUEUE_DEFINE(runQ);

void ready(struct process *proc)
{
  queue_add(runQ, proc);
}

struct process *scheduler_next()
{
  return queue_pop(runQ);
}
