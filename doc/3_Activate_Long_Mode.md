# Chapter 3 - Entering Long Mode

In this chapter, we'll put the processor in long mode with minimal
possible effort.


## Preparation

The AMD64 manual volume 4 outlines what needs to be done in order to
actiave long mode (chapter 14). It says we need:

- An IDT with 64 bit interrupt-gate descriptors *We don't need this as long as
  interrupts are disabled*
- 64-bit interrupt and exception handlers *See above*
- A GDT containing:
  - Any LDT descriptors *We don't have any*
  - A TSS descriptor *Only needed when we want to enter User mode*
  - Code descriptors for long mode code *One is enough for now*
  - Data-segment descriptors for software running in compatibility mode *We
    don't have that*
  - FS and GS data-segment descriptors *We won't be using those*
- A 64-bit TSS *See note about TSS descriptor above*
- The 4-level page translation tables

So if we bring it down to the essentials:

- A GDT with one entry
- A Page Table

Shouldn't be too hard. In fact, for now we can actually pretty much
hardcode those...

## GDT

In long mode, segmentation and the GDT doesn't really fill any
purpose... It's still required, for some reason, but if you read the
AMD manual, you'll see that in long mode, almost all fields of the GDT
entries are ignored.

What's left can be set up like this:

`src/kernel/boot/boot_GDT.S`
```asm
#include <gdt.h>
.intel_syntax noprefix

.section .rodata
.global BootGDT
.global BootGDTp

BootGDT:
  .long 0,0
  .long 0, (GDT_PRESENT | GDT_CODE | GDT_LONG)

BootGDTp:
  .short 2*8-1
  .quad offset BootGDT
```

where
- `GDT_PRESENT = 1<<15`
- `GDT_CODE = 3<<11`
- `GDT_LONG = 1<<21`

The GDT is page aligned, of course, and the GDT pointer is configured in
the same way as in 32 bit mode.


## Page Tables

Paging works pretty much exactly the same way in 64 bit mode as in
32, but with four levels of nested tables instead of two. If you have
trouble wrapping your head around it, chapter 5 *Page Translation and
Protection* of the AMD64 Systems programming manual should help.

The four levels do have names, "Page-Map Level-4 Table", "Page-Directory
Pointer Table", "Page Directory Table" and "Page Table", but I like to
think of them as P4, P3, P2 and P1.

We could make use of the 2 Mb page translation feature, which uses only three
levels. I.e. the entries of P2 points directly at the start of a 2 Mb memory
area rather than at a P1. This is indicated by a special flag in the P2 entry.
Doing so would make the memory management a bit more complicated later, though,
so I won't use that for now.

For now, we'll just identity map the first two megabytes of memory. That should
be enough to get the kernel started.  So we just need a P4 where the first
entry points to a P3 where the first entry points to a P2 where the first entry
points to a P1 filled with 512 entries ranging from 0 to 2 mb.

`src/kernel/boot/boot_PT.S`
```asm
.#include <memory.h>
.intel_syntax noprefix

.section .data
.align PAGE_SIZE
.global BootP4

BootP4:
  .quad offset BootP3 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP3:
  .quad offset BootP2 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP2:
  .quad offset BootP1 + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 1
    .quad 0
  .endr
BootP1:
  .set i, 0
  .rept ENTRIES_PER_PT
    .quad (i << 12) + (PAGE_PRESENT | PAGE_WRITE)
    .set i, (i+1)
  .endr
```

where
- `PAGE_PRESENT = 0x001`
- `PAGE_WRITE = 0x002`
- `ENTRIES_PER_PT = 512`


## Activating Long Mode

Again, consulting the AMD64 manual we find the following steps to
activate long mode:

1. Disable paging *Paging isn't enabled by GRUB, so we're good to go*
2. In any order:
    - Enable PAE by setting CR4.PAE to 1
    - Load CR3 with the address of P4
    - Enable long mode by setting EFER.LME to 1
3. Enable paging

We should then reload the system tables (in our case only GDT) with 64
bit descriptors.

The manual is even kind enough to supply us with some sample code which
also performs some checks to ensure that long mode is available. So
let's go.

`src/kernel/boot/boot.S`

```asm
...
.code32
.global _start
_start:
  cli
  mov esp, offset BootStack
...
```

First we set up a temporary stack for booting. The label BootStack is
defined earlier:

```asm
.section .bss
.align PAGE_SIZE
.skip PAGE_SIZE
BootStack:
```

Note that the label is after the reserved memory, since the stack grows upwards.

If you wish to make things The Right Way, you should probably check if the
processor supports long mode before going further.  This can be done through
the `cpuid` instruction and the process is described in the AMD64 manual. I
opted to skip this check, and just fail in an uncontrolled manner in the
unlikely event that the code is run on 32 bit processor.

Ok. Let's get to the meat of it

`src/kernel/boot/boot.S`
```asm
...
  //; Set CR4.PAE
  //; enabling Page Address Extension
  mov eax, cr4
  or eax, 1<<5
  mov cr4, eax

  //; Load a P4 page table
  mov eax, offset BootP4
  mov cr3, eax

  //; Set EFER.LME
  //; enabling Long Mode
  mov ecx, 0x0C0000080
  rdmsr
  or eax, 1<<8
  wrmsr

  //; Set CR0.PG
  //; enabling Paging
  mov eax, cr0
  or eax, 1<<31
  mov cr0, eax
...
```

I think the comments explain this well enough. It's just following the
list of actions from the AMD manual anyway.

> Speaking of comments, I apologize for the unconventional comment style `//;`.
> Normally GAS assembly is commented by a `;`, but I run all my files through
> the gcc preprocessor, which interprets semicolon as the end of a line.
> Instead, I have to use c-style comments (`//` or `/* */`). Those are,
> however, not recognized by the github markdown syntax coloring engine, and
> the results look messy with weird colors all over the place. That's why I use
> the combination.

The only step that's left is reloading the system tables. This is done
in exactly the same way as when going to protected mode, by loading a
GDT, loading selectors, and performing a long jump to load CS.

`src/kernel/boot/boot.S`
```asm
...
  //; Load a new GDT
  lgdt [BootGDTp]

  //; and update the code selector by a long jump
  jmp 0x8:long_mode_start

.code64
  long_mode_start:

  //; Clear out all other selectors
  mov eax, 0x0
  mov ss, eax
  mov ds, eax
  mov es, eax

  //; Loop infinitely
  jmp $
```

And that's all!

## Testing it out

Fire up the emulator, make sure the kernel is loaded into gdb, and let's go!

Let's step through the entire boot process

    (gdb) b _start
    Breakpoint 1 at 0x91: file boot/boot.S, line 63
    (gdb) c
    Continuing.

    Breakpoint 1, _start () at boot/boot.S:63
    64        cli
    (gdb)

The first thing that happens is that we set the stack pointer. You can
see that this happens by printing `esp`.

    (gdb)p/x $esp
    $1 = 0x7ff00
    (gdb)si
    65        mov esp, offset BootStack
    (gdb)si
    67        call check_cpuid
    (gdb)p/x $esp
    $2 = 0x5000

So things seem to work so far.

The next thing that happens is that the two functions are called to
check cpuid and long mode availability. You can step through those and
inspect values as you wish. I'll just skip to after returning from
check\_longmode.

You can't print the contents of `CR4` in gdb, but you can read it from
the qemu monitor command `info registers` which can be called from gdb
by the `monitor command`.

    _start () at boot/boot.S:72
    72        mov eax, cr4
    (gdb) monitor info registers
    EAX=00000664 EBX=00000000 ECX=00000005 EDX=2193fbfd
    ...
    CR0=00000011 CR2=00000000 CR3=00000000 CR4=00000000
    ...
    XMM06=00000000000000000000000000000000 XMM07=00000000000000000000000000000000
    (gdb)

To simplify things, I wrote a python function to extract individual
registers and put it in `toolchain/gdbinit` - see next section. This
let's you run `reg cr4` to se the register contents.

    (gdb) reg cr4
    CR4=00000000
    (gdb) n
    73        or eax, 1<<5
    (gdb)
    74        mov cr4, eax
    (gdb)
    77        mov eax, offset BootP4
    (gdb) reg cr4
    CR4=00000020
    (gdb)

No surprises there, really. Page address extension should now be enabled.

The next step is loading the page table. This shouldn't actually matter,
since paging is disabled, so just step through it and make sure the
value loaded into `CR3` is page aligned.

    82        mov ecx, 0x8C000080
    (gdb) reg cr3
    CR3=00002000

The same goes for setting the Long Mode Enable bit of the EFER register.
Make sure to remember the value of EFER, though ...

    89        mov eax, cr0
    (gdb) reg efer
    EFER=0000000000000100
    (gdb) monitor info mem
    PG disabled
    (gdb)

... because after we enable paging by setting the Paging bit in CR0 ...

    (gdb) reg cr0
    CR0=00000011
    (gdb) n
    90       or eax, 1<<31
    (gdb)
    91       mov cr0, eax
    (gdb)
    94       lgdt [BootGDTp]
    (gdb) reg cr0
    CR0=80000011
    (gdb)

... the Long Mode Active bit (bit 10) should also be set.

    (gdb) reg efer
    EFER=0000000000000500
    (gdb) monitor info mem
    0000000000000000-0000000040000000 0000000040000000 -rw
    (gdb)

This means the processor is in Long Mode!

You'll also see that the command `monitor info mem` (which I mapped to
`mmap` in my gdbinit - see next section) show that paging is enabled
and that the first Gb of virtual memory is mapped. Also note that the
virtual address space expects addresses of 64 bits now.

But we're still running code in Legacy Mode. That's why we load the GDT
and reload the segment selectors next.

    (gdb) n
    97        jmp 0x8:long_mode_start
    (gdb)
    long_mode_start () at boot/boot.S:103
    103       mov eax, 0x0
    (gdb) reg
    RAX=0000000080000011 RBX=0000000000000000 RCX=00000000c0000080 RDX=0000000000000000
    ...
    CS =0008 0000000000000000 00000000 00209800 DPL=0 CS64 [---]
    ...
    (gdb)

You'll note that the `reg` command now outputs registers with the R
prefix (`RAX` instead of `EAX` etc.) and that they are twice as big.

We are now running 64 bit code!

## Bonus

I mentioned two custom gdb commands in the previous section.

`mmap` which shows the memory map from qemu, and `reg` which prints the
value of a register. Those are defined in `toolchain/gdbinit`:

```
define mmap
monitor info mem
end

python

import re

class Reg(gdb.Command):

  def __init__(self):
    super(Reg, self).__init__("reg", gdb.COMMAND_USER)

  def invoke(self, arg, from_tty):
    regs = gdb.execute('monitor info registers', False, True)

    if not arg:
    # If no argument was given, print the output from qemu
      print regs
      return

    if arg.upper() in ['CS', 'DS', 'ES', 'FS', 'GS', 'SS']:
    # Code selectors may contain equals signs
      for l in regs.splitlines():
        if l.startswith(arg.upper()):
          print l
    elif arg.upper() in ['EFL', 'RFL']:
    # The xFLAGS registers contains equals signs
      for l in regs.splitlines():
        if arg.upper() in l:
          print ' '.join(l.split()[1:])
          # The xFLAGS register is the second one on the line
    else:
    # Split at any word followed by and equals sign
    # Clean up both sides of the split and put into a dictionary
    # then print the requested register value
      regex = re.compile("[A-Z0-9]+\s?=")
      names = [v[:-1].strip() for v in regex.findall(regs)]
      values = [v.strip() for v in regex.split(regs)][1:]
      regs = dict(zip(names, values))
      print "%s=%s" % (arg.upper(), regs[arg.upper()])


Reg()

end
```

The `mmap` command is obvious enough, but the `reg` one is a bit tougher.
A bit of information on the syntax of python commands in gdb can be
found [here](https://sourceware.org/gdb/onlinedocs/gdb/Python-API.html).

The rest is some rather messy python, but the basic flow is this

- Get the register output from grub by running `monitor info registers`
- If no argument was given, print the output we got and end
- Look for text in the format `SOMETHING=SOMETHINGELSE` and split it into `SOMETHING` and `SOMETHINGELSE`
- Put `SOMETHING` and `SOMETHINGELSE` back together in a way that's more useful for python
- Print the value we want

Then there are some special cases for things like `EFL` which contains
equals signs in the output. E.g. displaying bit flags `EFL=0000002
[-------] CPL=0 II=0 A20=1 SMM=0 HLT=0`. Note that I'm not catching all
such cases, but only the ones I think might be interesting.

Note also that gdb doesn't require the entire command name, but only enough to
make it unambiguous. As such, you can run `(gdb) mm` instead of `(gdb) mmap` if
you'd like. Just a heads up...

