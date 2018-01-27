#include <scheduler.h>
#include <queue.h>
#include <thread.h>

QUEUE_HEAD_EMPTY(READYQ);

void ready(struct thread *th)
{
  QUEUE_ADD(READYQ, th);
}

struct thread *scheduler_next()
{
  struct thread *th = QUEUE_PEEK(READYQ);
  QUEUE_DROP(READYQ);
  return th;
}
