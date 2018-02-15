#include <scheduler.h>
#include <queue.h>
#include <thread.h>

QUEUE_DEFINE(READYQ);

void ready(struct thread *th)
{
  queue_add(READYQ, th);
}

struct thread *scheduler_next()
{
  struct thread *th = queue_peek(READYQ);
  queue_drop(READYQ);
  return th;
}
