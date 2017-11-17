# Chapter 4 - "Higher Half" Kernel

In this chapter we'll make our kernel run in the top of memory - well out of
the way of user programs and memory mapped devices.

## What is a higher half kernel

Some arguments for a higher half kernel can be found at [the osdev
wiki](http://wiki.osdev.org/Higher_Half_Kernel).  There are arguments against
as well, such as it being pointless with modern memory management routines.  My
main argument for using it is that it makes things simpler.

I chose to put the split at `0xFFFFFF8000000000`, which corresponds to the last
entry in P4.

> A note about the address. If you go through the calculations, you'll find
> that the addresses mapped from the last entry in P4 actually starts at
> `0xFF8000000000` or `0x0000FF8000000000`. However, due to limitations in the
> hardware, the X86 architecture requires the most significant 13 bits of an
> address to be equal; thus `0xFFFFFF8000000000`. This address format is called
> 'canonical'.

## Higher half linking

I want to have the kernel running at address `0xFFFFFF8000000000` and above.
However, when we're booted from GRUB, paging is not enabled, so
we're limited to physical RAM.

The solution to this problem is to tell GRUB to load the kernel into a low
memory position, but make the kernel think it's loaded at a high position. This
can be done with a linker script trick.

We change the linker script from Chapter 2 to something like this:

`src/kernel/Link.ld`
```
ENTRY(_start)

KERNEL_OFFSET = 0xFFFFFF8000000000;
KERNEL_START = 0x10000;

SECTIONS
{
  . = KERNEL_START + KERNEL_OFFSET;
  .text : AT(ADDR(.text.) - KERNEL_OFFSET)
  {
    *(.multiboot)
    *(.text)
  }
}
```

What this does is tell the linker to assume the code starts at
`0xFFFFFF8000010000` when calculating addresses for things like function calls
and jumps and such, but to generate an ELF file with headers saying the `.text`
section should be *loaded* at an address `0xFFFFFF8000000000` below that, i.e.
`0x10000` which is within the physical RAM limits.

This also means that GRUB will jump to address `0x10000` after loading the
kernel, and from there we can set up paging and jump to above
`0xFFFFFF8000000000`. We just need to take care at all memory references, since
we can't trust the linker to sort them out before paging is setup.

Oh, and you will also want to do the same with the other default elf sections
in the linker file, such as `.rodata`, `.data` and `.bss`.

## Fixing memory references

In order to make sure all memory references are correct, we'll define some
helpful macros.

`src/kernel/include/memory.h`
```c
#define KERNEL_OFFSET 0xFFFFFF8000000000

#ifdef __ASSEMBLER__
#define V2P(a) ((a) - KERNEL_OFFSET)
#define P2V(a) ((a) + KERNEL_OFFSET)
#else
#include <stdint.h>
#define V2P(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define P2V(a) ((uintptr_t)(a) | KERNEL_OFFSET))
#endif
...
```

I define two versions of the macros, one for use in assembly and one for c.
`__ASSEMBLER__` is set by gcc when compiling a `.S` file.  The c version uses
bit operations which means you can run `V2P(VP2(address))` without any problems
due to the format of `KERNEL_OFFSET`. The proof of this is left as an exercise
to the reader.

Note that although we don't have access to a standard c library at this point,
`stdint.h` (which defines `uintptr_t`) can still be used since it's included in
`libgcc`, which we built and installed in the docker image together with the
compiler.

Then we need to go through our code and make sure all memory references are
corrected.

`src/kernel/boot/boot.S`
```asm
#include <memory.h>
.intel_syntax noprefix

...
_start:
  cli
  mov esp, offset V2P(BootStack)

  ...

  mov eax, offset V2P(BootP4)
  mov cr3, eax

  ...

  lgdt [V2P(BootGDTp)]

  ...

  jmp 0x8:V2P(long_mode_start)
  ...
```

Note that `call` instructions don't have to be modified, since `call` uses
relative addressing.

And don't forget about the memory references in the page tables:

`src/kernel/boot/boot_PT.S`
```asm
...
BootP4:
  .quad offset V2P(BootP3) + (PAGE_PRESENT | PAGE_WRITE)
...
```

Note also that the GDT pointer does not require to be redirected to `V2P(BootGDT)` as one would assume. But why is that? Because of pure luck and coincidence.

Before starting long mode the `lgdt` instruction will expect a 32 bit gdt pointer. If there's any more data, the top bits will just be truncated (due to the small-endian nature of the processor). As luck would have it, the only difference between `BootGDT` and `V2P(BootGDT)` lies in the top 32 bits. This also means that when it's time to load a 64 bit GDT, we can use the same pointer. Neat!

At this point, it would be a good idea to check that the kernel still boots.
However, gdb won't be able to tell you anything about the code since we're
running outside of the linked addresses. You can still use it to inspect
registers and such, though. You can also set breakpoints by modifying the
address manually: `(gdb) break *(long_mode_start - 0xFFFFFF8000000000)`.

## Jumping to higher half

The final piece of setup we need to do before we can start running in the
higher half is update the page table.

We'll do this by adding a pointer to the same P3 we set up earlier at the end
of the BootP4.

`src/kernel/boot/boot_PT.S`
```asm
...
BootP4:
  .quad offset V2P(BootP3) + (PAGE_PRESENT | PAGE_WRITE)
  .rept ENTRIES_PER_PT - 2
    .quad 0
  .endr
  .quad offset V2P(BootP3) + (PAGE_PRESENT | PAGE_WRITE)
...
```

If you start up the emulator now you can check that the higher half is mapped

```
(gdb) mmap
0000000000000000-0000000040000000 0000000040000000 -rw
0000ff8000000000-0000ff8040000000 0000000040000000 -rw
```

Note that qemu doesn't report the addresses in canonical mode.

Anyway. It should now be safe to jump to higher half code:

`src/kernel/boot/boot.S`
```asm
...
.code64
long_mode_start:
  mov eax, 0x0
  mov ss, eax
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax

  movabs rax, offset upper_memory
  jmp rax

upper_memory:

  jmp $
```

By loading the address of the `upper_memory` into a register and jumping to it
we force the assembler to make a non-relative jump.

If you run this, you'll find that gdb will be able to track where in the code
you are again (after passing `upper_memory:`, or you could check the `RIP`
register.

```
upper_memory () at boot/boot.S:116
116       jmp $
(gdb) reg RIP
RIP=ffffff800001019f
```

Great! Now we can do some cleanup

Move the stack pointer to higher half memory:
```asm
...
upper_memory:
  mov rax, KERNEL_OFFSET
  add rsp, rax
...
```

and unmap the identity mapping of the first gigabyte and reload the page table:

```asm
...
  mov rax, 0
  movabs [BootP4], rax

  mov rax, cr3
  mov cr3, rax
...
```

Run it all again, and check that the low memory is unmapped:

```
(gdb) mmap
0000ff8000000000-0000ff8040000000 0000000040000000 -rw
```

Finally, we also need to reload the GDT. In long mode, the GDT
register points to the physical address of the GDT, and we just
unmapped that...

So we need to

- reload the GDT and update the data selectors:
```asm
...
  lgdt[rax]
  mov rax, 0x0
  mov ss, rax
  mod ds, rax
  mov es, rax
...
```
- and reload the code selector. There are no long jumps in long mode,
  so instead we'll use the `retfq` instruction which pops a return
  address and code segment selector off the stack:
```asm
...
  movabs rax, offset .reload_cs
  pushq 0x8
  push rax
  retfq
.reload_cs:
```

## Running c code

Now that the instruction pointer is safely within our linked memory, we can
trust c code to run.

Calling a c function is simple enough:

`src/kernel/boot/boot.S`
```asm
...
.reload_cs:

.extern kmain
  movabs rax, offset kmain
  call rax

  hlt
  jmp $
```

And the c source file:
`src/kernel/boot/kmain.c`
```c
#include <memory.h>

void clear_screen()
{
  unsigned char *vidmem = P2V(0xB8000);
  for(int i=0; i < 80*24*2; i++)
    *vidmem++ = 0;
}

void print_string(char *str)
{
  unsigned char *vidmem = P2V(0xB8000);
  while(*str)
  {
    *vidmem++ = *str++;
    *vidmem++ = 0x7;
  }
}

void kmain()
{
  clear_screen();
  print_string("Hello from c, world!");
  for(;;);
}
```

... which will clear the screen and print "Hello from c, world!".
Things are so much simpler in c...

But what now? This doesn't compile!

You'll probably get an error about "relocation truncated to fit: R_X86_64_32
against \`.rodata\`"

This is because gcc assumes your code will be running at a lower memory
address, and optimizes it as such. The solution is to tell gcc to make no
assumption about addresses by adding the switch `-mcmodel=large` to `CFLAGS` in
your makefile.
