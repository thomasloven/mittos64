#include <thread.h>

struct {
  struct thread *first;
  struct thread *last;
} readyQ = {0,0};

void ready(struct thread *th)
{
  if(!readyQ.last)
  {
    th->tcb.next = 0;
    readyQ.first = readyQ.last = th;
  } else {
    readyQ.last->tcb.next = th;
    readyQ.last = th;
  }
}

struct thread *scheduler_next()
{
  struct thread *th = readyQ.first;
  if(!(readyQ.first = th->tcb.next))
    readyQ.last = 0;
  return th;
}
