#pragma once
#include <queue.h>

#define READYQ readyQ, readyQ_next, struct thread
QUEUE_DECLARE(READYQ);

void ready(struct thread *th);
struct thread *scheduler_next();
