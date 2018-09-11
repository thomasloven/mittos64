# Threading

In this chapter we'll implement context switching and a simple scheduler.

## Context switching

Switching from one thread to another is actually really easy. All you need to
do is replace everything in every processor register at the same time, and
you're good to go. Ok... that doesn't sound so simple, but we get some help.

In the conventions used by our gcc cross compiler (known as [System V
ABI](https://wiki.osdev.org/System_V_ABI)), when calling a function only a few
registers are guaranteed to be preserved. Those are `rbx`, `rsp`, `rbp`, `r12`,
`r13`, `r14` and `r15`. The rest can not be assumed to retain the same value.

So, if we make our context switch out to look like a function call, we only
need to replace those seven registers. This can easily be done with a small asm
routine:

`src/kernel/proc/swtch.S`
```asm
.intel_syntax noprefix

.global switch_stack
switch_stack:
  push rbp
  mov rbp, rsp

  push r15
  push r14
  push r13
  push r12
  push rbx
  push rbp

  mov [rdi], rsp
  mov rsp, [rsi]

  pop rbp
  pop rbx
  pop r12
  pop r13
  pop r14
  pop r15

  leaveq
  ret
```

This pushes all registers, writes the stack pointer to the address passed as
the first argument, reads a new stack pointer from the address in the second
argument, and pops all registers. Since the return address is on the stack, the
`ret` instruction will return to the new thread.

> #### A note on credit
>
> Not everything I present here is my own original ideas. In fact, most of it
> probably isn't. I've been itterating my kernel design from the ground up a
> dozen times or more through the last ten years, and where I picked up methods
> and ideas have gotten lost along the way.
>
> This method of switching threads, though, I know I got from XV6, where it may
> or may not have originated.
>
> I'm sorry I can't always give proper and detailed credit to the giants on
> whose shoulders I stand, but for a list of my most significant sources of
> inspiration through the years, see Chapter 0.

## Threads

Ok, so now switching between two threads of execution only requires:

```c
switch_stack(&old_stack_ptr, &new_stack_ptr);
```

So the next step would be a structured and reliable way of keeping track of new
and old stack pointers, and to allocate the stacks themselves. We might also
want some extra information relating to each thread. A struct would be ideal
for this.

`src/kernel/include/thread.h`
```c
struct thread
{
  uint64_t tid;
  void *stack_ptr;
  uint64_t state;
};
```

This will grow with a lot of more information later.

But where should we put this struct? you may remember from an earlier chapter
that we don't have a `malloc` implementation to assign storage. We could use
`pmm_alloc` to get some memory, but that would give us an entire page, and this
struct is only 24 bytes. So what should we do with the rest of the space?

How about using it for the thread stack?
Allocating a new thread would then look something like this:

`src/kernel/proc/thread.c`
```c
uint64_t next_tid = 1;
struct thread *new_thread()
{
  struct thread *th = P2V(pmm_calloc());
  th->tid = next_tid++;
  th->stack_ptr = incptr(th, PAGE_SIZE);

  return th;
}
```

Of course, this thread can't be run. If we try to switch to it, the
`switch_stack` function won't get a propper stack to start from, so we need to
mock that up first:

`src/kernel/proc/thread.c`
```c
struct swtch_stack
{
  uint64_t RBP;
  uint64_t RBX;
  uint64_t R12;
  uint64_t R13;
  uint64_t R14;
  uint64_t R15;
  uint64_t RBP2;
  uint64_t ret;
};

uint64_t next_tid = 1;
struct thread *new_thread(void (*function)(void))
{
  struct thread *th = P2V(pmm_calloc());
  th->tid = next_tid++;
  th->stack_ptr = incptr(th, PAGE_SIZE - sizeof(struct swtch_stack));

  struct swtch_stack *stk = th->stack_ptr;
  stk->RBP = (uint64_t)&stk->RBP2;
  stk->ret = (uint64_t)function;

  return th;
}
```

That's all you need to do to set up a new thread with it's own stack, which
will start running the function you pass to `new_thread`.

If you wish, you can try this out now. Here's a quick (untested) mockup on how
it might be done:

`src/kernel/boot/kmain.c`
```c

struct thread *current, *next;

void thread_function()
{
  int thread_id = current->tid;

  while(1)
  {
    debug("Thread %d\n", thread_id);
    struct thread *_next = next;
    struct thread *_current = current;

    // Update "scheduler"
    next = current;
    current = _next;

    // Switch thread
    switch_stack(&_current->stack_ptr, &_next->stack_ptr)
  }
}

void kmain(...)
{
  ...

  current = new_thread(thread_function);
  next = new_thread(thread_function);

  uint64_t dummy_stack_ptr;
  switch_stack(&dummy_stack_ptr, &current->stack_ptr);

  ...
}

```

This implements a simple "scheduler" which keeps track of two threads and
switches between them.

Switching in the first thread requires a dummy variable to store the old stack
pointer. This value is thrown away, because we will never switch back into
`kmain`.

If you run this, the screen should fill with alternating lines of "Thread 1"
and "Thread 2". Note that the variable `thread_id` is function local, and thus
stored on the stack. If you're not convinced, you can make the threads run
different functions instead.

I don't get why tutorial writers make this look so hard...

Now it's time to make it scaleable.

## Queueing

Ok, so now we can create and switch between threads, but we still need some way
of keeping track of them.

Threads can be in one of several states. They are either *running*, *waiting to
run*, or *waiting for something else*. This might seem obvious, kind of like
how everything is either *a banana* or *not a banana*, but those three states
are kind of important and decide how we keep track of the thread.

For the running threads it's easy. Only one thread per cpu core can be running
at a time, so it makes sense to keep a global variable which points to the
running thread (one per cpu core).

Threads that are waiting to run will be kept in the scheduler queue. When a
running thread has finished running for one reason or another, the sheduler
will pick the next one to run from the run queue based on various conditions
and, if necessary, put the previously running thread back in the queue.

Where the threads waiting for something else are kept depends on what they are
waiting for. For example, a thread waiting for a disk read to finish will
probably be kept track of by the disk driver or simmilar. Once the read
completes, the driver will hand the thread over to the scheduler to be put in
the run queue.

Either way, almost all waiting threads will be kept in a queue somewhere.

I won't go into the pros and cons of different queueing methods here, but just
use a simple linked list, where the queue header keeps track of the first and
last item, and each item in the queue keeps track of the next one. Something
like this:

```c
struct {
  struct thread *first;
  struct thread *last;
} run_queue;


struct thread
{
  ...
  struct thread *run_queue_next;
  ..
};

void init_queue()
{
  run_queue.first = 0;
  run_queue.last = 0;
}

void queue_add(struct thread *th)
{
  if(!run_queue.last)
    run_queue.first = th;
  else
    run_queue.last->run_queue_next = th;
  run_queue.last = th;
  th->run_queue_next = 0;
}

thread *queue_pop()
{
  thread *ret = run_queue.first;
  if(run_queue.first && !(run_queue.first = run_queue.first->run_queue_next))
    run_queue.last = 0;
  return ret;
}
```

That's a full FIFO queue setup. Simple, but not very fun.

Let's generalize!

There are three things which are unique for each queue.

- The name of the queue header struct
- The name of the pointer to the next item in the item struct
- The type of item in the queue

With some macro magic, we can condense this into a single symbol:

```c
#define RunQ run_queue, run_queue_next, struct thread
```

The plan is that `RunQ` should be used every time we want to do or define
something with the run queue. Such as `queue_add(RunQ, my_thread)` or
`queue_pop(RunQ)`, or:

```c
struct thread
{
  ...
  QUEUE_SPOT(RunQ);
  ...
};
```

You probably see by now that `queue_add` would need to be a macro. You can't
pass a type to a function, and the example above would expand to
`queue_add(run_queue, run_queue_next, struct thread, my_thread)`.

But if `queue_add` is a macro, `RunQ` won't be expanded...

This can be solved with an extra layer of indirection. We define a variadic
wrapper macro which expands all arguments and pass them to another macro. This
will make the preprocessor realize it needs to make another run of the code.

```c
#define _QUEUE_ADD(queue, entry, type, item) \
  if(!queue.last) \
    queue.first = (item) \
  else
    queue.last->entry = (item); \
  queue.last = (item); \
  (item)->entry = 0;
#define queue_add(...) _QUEUE_ADD(__VA_ARGS__)
```

Then we do the same thing with any other queue operations we might need, as
well as declaring and defining the queue head and next item pointer.

```c
#define _QUEUE_DECL(queue, entry, type) \
  struct queue{ \
    type *first; \
    type *last; \
  } queue;
#define QUEUE_DECLARE(...) _QUEUE_DECL(__VA_ARGS__)

#define _QUEUE_HEAD(queue, entry, type) \
  struct queue queue = {0, 0};
#define QUEUE_DEFINE(...) _QUEUE_HEAD(__VA_ARGS__)

#define _QUEUE_SPOT(queue, entry, type) \
  type *entry
#define QUEUE_SPOT(...) _QUEUE_SPOT(__VA_ARGS__)

#define _QUEUE_POP(queue, entry, type) \
  __extension__({ \
    type *_ret = _(queue.first); \
    if(queue.first && !(queue.first = queue.first->entry)) \
      queue.last = 0; \
    _ret; \
  })
#define queue_pop(...) _QUEUE_POP(__VA_ARGS__)
```

> The `__extension__` thing is a workaround to make gcc accept a macro with a
> return value without warnings.  For some reason I get warnings that this is
> valid ANSI c even when compiling with `-std=gnu11`...
