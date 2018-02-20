#pragma once
#include <queue.h>

#define runQ readyQ, readyQ_next, struct process
QUEUE_DECLARE(runQ);

void ready(struct process *proc);
struct process *scheduler_next();
