# Memory Management

Normally, I think of the memory management of the kernel as four parts.

- Physical Memory Manager
- Virtual Memory Manager
- Kernel Heap
- Process Memory Manager

We'll go through them in order, but first a radical design decision.

## Mapping the entire physical memory into kernel space

A 32-bit processor can address about 4 Gb of memory. Near the end of the 32-bit
era, even home desktop computers were actually running into this limit.

In chapter 4, I mentioned that te top 16 address bits must be the same due to
hardware limitations, in what's called canonical addressing. This leaves 48
bits, which lets us address 256 Tb.

I also mentioned that I reserve a 512th of this for the kernel (virtual
addresses above 0xFFFFFF8000000000), which is about 512 Gb.

512 Gb - to paraphrase something Bill Gates probably never actually said -
ought to be enough for anybody.

This means we could - for any forseeable future, and definitely longer than the
lifespan of my hobby kernel designed for running in an emulated environment
with a couple of Mb of RAM - map the entirety of RAM right into the kernel
address space.

The main advantage of this is that we will never have to map anything into
kernel space temporarily, say to modifiy the page tables of a process. It also
means that we can use unused pages for temporary storage (more on this in a
minute).

The main disadvantage is that the entirety of memory is mapped into the kernels
virtual memory at all times. At the time of writing this, the Spectre and
Meltdown hardware bugs affecting almost all modern Intel CPUs have recently
proved that anything mapped into memory is insecure.

There are ways to remedy this, but they come at a performance cost, and -
honestly - I don't feel like implementing them right now. Perhaps I will later,
or perhaps I'll never have to because Intel decides that maybe they should fix
the issue.

Anyway. The entirety of physical RAM will be mapped into kernel space. That
means a physical page with address `addr` can be accessed at `P2V(addr)`, where
`P2V` is one of the two macros defined in Chapter 4. This explains their name
P2V - Physical to Virtual, and V2P - Virtual to Physical. We'll use that a lot.

## The physical memory manager - PMM

The PMM keeps track of available physical memory and hands out free pages when
requested.

Thanks to virtual address translation, we almost never need to care where a
physical page is located, and therefor the PMM can be made very simple.

In reality, we sometimes do need a number of continuous physical pages - for
example when reading from disk using Direct Memory Access, but we'll save that
for another day.

The entire PMM has only one single state variable. A pointer to the last freed
page. This page, in turn, holds a pointer to the one that was freed before, and
so on, so when we free a page, we write the pointer to it, and update the free
pointer:

`src/kernel/memory/pmm.c`
```c
...
uint64_t next = 0;

void pmm_free(uint64_t page)
{
  *(uint64_t *)P2V(page) = next;
  next = (uint64_t)P2V(page);
}
...
```

That's all. Allocating a page is equally simple:

```c
...
uint64_t pmm_alloc()
{
  if(!next) return 0;
  uint64_t page = next;
  next = *(uint64_t *)page;
  return V2P(page);
}
...
```

Feeling comfortable about dereferencing a pointer cast will help you when
debugging your VMM in gdb, by the way.

I also define a `pmm_calloc()` function, which allocates and zeros a page.

## The virtual memory manager - VMM

The VMM handles setting up page tables, and separating user and kernel space.

As mentioned earlier, x86\_64 uses four levels of page tables, each containing
512 64 bit entries. The 52 most significant bits of each entry is a pointer to
the next level of page table, or the page itself in the case of the last page
table (P1 in my inofficial nomenclature). This pointer is obviously always page
alligned.

The 12 least significant bits of each entry are for various flags. In order to
find the next level, they need to be masked out. So let's start with some
macros:

`src/kernel/memory/vmm.c`
```c
...
#define FLAGS_MASK (PAGE_SIZE - 1)
#define MASK_FLAGS(addr) ((uint64_t)addr & ~FLAGS_MASK)
...
```

Now it's easy to go from a page table entry to something we can parse:

```c
...
#define PT(ptr) ((uint64_t *)P2V(MASK_FLAGS(ptr)))
...
```

Page table entries are physical addresses, so we need to go through `P2V` to
access them. To access a certain page table entry, you just do e.g.
`PT(P4)[num]`.I chose to define some macros to access the entries for a certain
address in each page table (P4 trough P1). They look like this:

```c
...
#define P4E (PT(P4)[P4_OFFSET(addr)])
#define P3E PT(P4E)[P3_OFFSET(addr)])
#define P2E PT(P3E)[P2_OFFSET(addr)])
#define P1E PT(P2E)[P1_OFFSET(addr)])
...
```

This assumes that there's a variable named `addr` which
contains the virtual address whose page table entries you
want, and a variable named `P4` which points to the top
level page directory. The `OFFSET` macros finds the
correct entry for an address for each page table level
(`#define P4_OFFSET(a) (((a)>>39 & 0x1FF)` and so on).

Now getting the physical page for a virtual address is very easy:

```c
...
uint64_t vmm_get_page(uint64_t P4, uint64_t addr)
{
  if(P4 && PRESENT(P4E) && PRESENT(P3E) && PRESENT(P2E))
    return P1E;
  return -1;
}
...
```

Where the `PRESENT` macro just checks for the `PAGE_PRESENT` bit being set.

Setting the physical page for a virtal address is also very easy:

```c
...
int vmm_set_page(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags)
{
  ...

  if(!PRESENT(P4E) && !(P4E = pmm_calloc()))
    return -1;
  P4E |= flags | PAGE_PRESENT;

  // Do the same thing for P3E and P2E
  ...

  P1E = page | flags;
  return 0;
}
...
```

The first three lines checks if the P4 entry is set, and if not, allocates a P3
and sets the entry to point to it. If the allocation fails, `vmm_set_page`
fails as well. The same is then done with P2 and P1, and finally the correct P1
entry is set with the page address and flags.

I also wrote a function `void vmm_clear_page(uint64_t P4, uint64_t addr, int
free)` which zeros the P1 entry, and - if `free` is true - frees P1, P2 and P3
if they are empty. This is left as an exercise to the reader.

## The kernel heap

The kernel heap keeps track of and hands out small chunks of memory for
temporary storage. This corresponds to the `malloc()`-family of functions.

In this case, I actually chose to forego the heap, and try to make do with
hard-coded global variables and structures using entire pages.

For example, when setting up a new process, I might normally do something like

```c
struct process *new_process = kmalloc(sizeof(struct process));
setup_process(new_process);
```

Now, I'll instead do:

```c
struct process *new_process = (void *)pmm_alloc();
setup_process(new_process);
```

This will reqire me to think a bit more carefully of how I define my various
data structures in order to keep wasted space to a minimum. It'll be an
interesting experiment, but we'll see - perhaps I'll end up implementing a heap
later...

## The process memory manager - PROCMM

The final memory related part of the kernel - the procmm - handles user space
memory. Setting up and cloning process memory spaces, replacing them with new
executables, and handling the user stack and `brk()`  calls are some of its
tasks.

Since there are no processes yet, having a process memory manager doesn't really make sense, so I'll save this for later...

## A remark about the git history

For this chapter, I went a bit crazy with the TDD and made one git commit every
time I wrote and passed a test. Perhaps that would make sense if I had a
finished API to conform agains. Now, it just got a bit messy... If you explore
the git history - I'm sorry.
