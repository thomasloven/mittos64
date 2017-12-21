# Exceptions and Interrupts

Sometimes, things go wrong. When they do, we want to fail gracefully - or even
recover. That's the point of exceptions.

## Interrupt Service Routines

The x86 interrupt handling method is, for historical reasons I assume, messy.
The x86\_64 architecture saw a slight improvement in that the stack pointer and
segment are always pushed, even if the cpu was running in ring 0 when the
interrupt happened. Still, though, some exceptions push an error code, and
others do not. And no data is provided to determine which interrupt occurred,
besides which interrupt service routine was called.

If all interrupts pushed a dummy error code and an identifying number, a single
ISR would be enough, and the rest could be done in software.

Anyway. Let's play with the cards we're dealt.

The most common way of solving this discrepancy is by having a number of short
ISRs in the form

```asm
isr1:
  push 0 //; Dummy error code
  push 1 //; Interrupt number
  jmp isr_common //; The rest is the same for all interrupts
```

You may want up to 256 ISRs, so let's do some finger warmup exercises!

Or rather yet, let's generate the ISRs automatically. With python!

`src/kernel/cpu/isr.S.py`
```python
#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function

num_isr = 256
pushes_error = [8, 10, 11, 12, 13, 14, 17]

print('''
.intel_syntax noprefix
.extern isr_common
''')


print('// Interrupt Service Routines')
for i in range(num_isr):
    print('''isr{0}:
    cli
    {1}
    push {0}
    jmp isr_common '''.format(i,
        'push 0' if i not in pushes_error else 'nop'))

print('')
print('''
// Vector table

.section .data
.global isr_table
isr_table:''')

for i in range(num_isr):
    print('  .quad isr{}'.format(i))
```

This outputs an assembly file with 256 ISRs like the one above, except numbers
8, 10, 11, 12, 13, 14 and 17, which has an `nop` instruction instead of pushing
a bogus error code.

It's written for python 2 because that's what's included in the alpine version
the build docker image is based on - despite it being 2018.  The encoding is
utf-8, and I import the print function from \_\_future\_\_, because it's 2018.

It also makes a table with pointers to each ISR, which makes it easy to set up
the Interrupt Descriptor Table later:

`src/kernel/cpu/interrupts.c`
```c
...
struct idt
{
  uint16_t base_l;
  uint16_t cs;
  uint8_t ist;
  uint8_t flags;
  uint16_t base_m;
  uint32t base_h;
  uint32_t _;
}__attribute__((packed)) idt[NUM_INTERRUPTS];

extern uintptr_t isr_table[]

void interrupt_init()
{
  memset(idt, 0, sizeof(idt));
  for(int i=0; i < NUM_INTERRUPTS; i++)
  {
    idt[i].base_l = isr_table[i] & 0xFFFF;
    idt[i].base_m = (isr_table[i] >> 16) & 0xFFFF;
    idt[i].base_h = (isr_table[i] >> 32) & 0xFFFFFFFF;
    idt[i].cs = 0x8;
    idt[i].ist = 0;
    idt[i].flags = IDT_PRESENT | IDT_DPL0 | IDT_INTERRUPT;
  }
...
```

`isr_common` pushes all registers to the stack (one by one, there's no `pusha`
instruction in x86\_64) and passes controll to a c interrupt handler. Note that
for x86\_64 the arguments to a function is not primarily passed on the stack,
  but in registers. So the last thing it does before calling the c function is
  move the stack pointer into `rdi`. In case the handler returns, `isr_common`
  restores the stack pointer from `rax` - which is the function return value,
  pops all values again, and performs an `iretq` instruction, which is pretty
  much a backwards interrupt.

`src/kernel/cpu/isr_common.S`
```asm
...
isr_common:
  push r15
  push r14
...
  push rbx
  push rax
  mov rdi, rsp
  call int_handler

  mov rdi, rax
isr_return:
  mov rsp, rdi
  pop rax
  pop rbx
...
  pop r14
  pop r15
  add rsp, 0x10
  iretq
```

But what's the deal with passing `rax` to `rsp` via `rdi`? Doing it this way
will allow us to call `isr_return` as a function, with a faked interrupt stack.
We'll use this later to get into user mode.

## Building isr.S.py

But back to the ISRs. In order to build this, we need some changes in the
kernel makefile.
First of all, the lines

```make
SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))
```

need to be updated to allow more file extensions:

```make
SRC := $(wildcard **/*.[cS]*)
OBJ := $(patsubst %, %.o, $(basename $(basename $(SRC))))
```

We also need a special rule to generate .o files from .S.py:

`src/kernel/Makefile`
```asm
%.o: %.S.py
	python $^ | $(COMPILE.S) $(DEPFLAGS) -x assembler-with-cpp - -o $@
```

In theory, it should be enough with a rule of the form

```make
%.S: %.S.py
	python $^ > $@
```

However, this generates the dependency tree .o <- .s <- .S <- .py rather than
.o <- .S <- .py, which uses `as` to compile, and causes some other trouble as
well with intermediate files that are removed once, but not if you run make
again, and stuff...

Some of this can be solved with an `.INTERMEDIATE:` rule, but that's not very
elegant. The big problem's probably with me rather than make.


## The Interrupt Handler

The c interrupt handler routine is a simple thing. Its default modus operandi
is to print an error message and hang.

However, before doing this, it checks a table of other interrupt handlers, and
if one exists for the current interrupt, it passes execution over to that.

`src/kernel/cpu/interrupts.c`
```c
registers *int_handler(registers *r)
{
  if(int_handlers[r->int_no])
    return int_handlers[r->int_no](r);

  debug("Unhandled interrupt occurred\n");
  debug("Interrupt number: %d Error code: %d\n", r->int_no, r->err_code);
  debug_print_registers(r);

  PANIC("Unhandled interrupt occurred");
  for(;;);
}
```

## Final Note

For tidyness sake, I wrapped the call to `interrupt_init` inside a function
called `cpu_init`, which in turn is called from `kmain`. For now, that's all it
is, but it will soon grow more important.

## Bonus: Debugging Interrupts

There's a small problem with the way interrupts are handled by the processor;
they don't follow the calling convention.

This means that when an interrupt occurs, and the debugger breaks in the
`PANIC` macro, it has lost all context, and we can't see what happened.

But wait. The entire context is saved. It was pushed to the stack and passed to
the interrupt handler. And by using gdbs ability to set the value of registers
in qemu, we can bring it back into scope.

I put the following function in `toolchain/gdbinit`

```gdb
define restore_env
set $name = $arg0
python

registers = {r: gdb.parse_and_eval('$name->' + r) for r in
['rax', 'rbx', 'rcx', 'rdx', 'rsi', 'rdi', 'rbp', 'rsp', 'r8', 'r9', 'r10',
'r11', 'r12', 'r13', 'r14', 'r15', 'rip']}

for r in registers.items():
  gdb.parse_and_eval('$%s=%s' % r)
gdb.execute('frame 0')
end
end
```

And it's used like this:

```
(gdb) c
Continuing.

Thread 1 hit Breakpoint 2, int_handler (r=0xffffff8000019f10) at cpu/interrupts.c:74
74        PANIC("Unhandled interrupt occurred");
(gdb) restore_env r
#0  0xffffff8000010caa in divide_two_numbers (divisor=0, dividend=0) at boot/kmain.c:18
18        return dividend/divisor;
(gdb) bt
#0  0xffffff8000010caa in divide_two_numbers (divisor=0, dividend=0) at boot/kmain.c:18
#1  0xffffff8000010dbd in kmain (multiboot_magic=920085129, multiboot_data=0x105fa0) at boot/kmain.c:33
#2  0xffffff8000010efd in .reload_cs () at boot/boot.S:96
#3  0x0000000000000007 in ?? ()
#4  0x0000000000000730 in ?? ()
#5  0x0000000000000000 in ?? ()
(gdb) list
13        for(;;);
14      }
15
16      int divide_two_numbers(int divisor, int dividend)
17      {
18        return dividend/divisor;
19      }
20
21      void kmain(uint64_t multiboot_magic, void *multiboot_data)
22      {
(gdb) p divisor
$1 = 0
(gdb) p divident
$2 = 5
(gdb) frame 1
#1  0xffffff8000010dbd in kmain (multiboot_magic=920085129, multiboot_data=0x105fa0) at boot/kmain.c:33
33        divide_two_numbers(0,5); // Calculate 0/5 and discard the results
(gdb)
```

By restoring the processor to the state stored in `r`, we can debug from where
the interrupt occurred as normal. By backtracing and inspecting variables we
find that whoever wrote line 33 in `kmain.c` got the divisor and divident mixed
up, which resulted in a divide by zero exception.
