#include <scheduler.h>
#include <queue.h>
#include <thread.h>

QUEUE_DEFINE(runQ);

void ready(struct thread *th)
{
  queue_add(runQ, th);
}

struct thread *scheduler_next()
{
  return queue_pop(runQ);
}
