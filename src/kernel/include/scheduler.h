#pragma once
#include <queue.h>

#define READYQ readyQ, readyQ_next, struct thread
QUEUE_DECL(READYQ);
QUEUE_HEAD(READYQ);

void ready(struct thread *th);
struct thread *scheduler_next();
