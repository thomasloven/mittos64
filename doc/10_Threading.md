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

So the next step would be to have a structured and reliable way of keeping
track of stacks and stack pointers. The stack pointer is easy enough, and we
might as well store some more thread related stuff at the same place while
we're at it.

`src/kernel/include/thread.h`
```c
struct thread
{
  uintptr_t stack_ptr;
  uint64_t tid;
  uint64_t state;
};
```

## Queueing
