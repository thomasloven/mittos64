# Multiboot Data

In this chapter we'll parse the multiboot information structure passed
to the kernel from the bootloader.

## Passing the data to the kernel
When a multiboot compliant bootloader passes controll to the kernel, the
registers `eax` and `ebx` contains a magic number and a pointer to the
multiboot information structure respectively.

If we wish to pass those on to the kernel main function, we can do that
according to the System V ABI calling convention. According to this,
the first two arguments to a function should be passed in the `rdi` and
`rsi` registers for x86\_64. Those obviously correspond to `edi` and
`esi` in 32-bit mode, and luckily those registers are unused throughout
the entire long mode transition process, untill we call `kmain`.

So everything we need to to do preserve the information is to move it
into those registers. Might as well do that as soon as possible.

`src/kernel/boot/boot.S`
```asm
...
_start:
  cli

  mov edi, eax
  mov esi, ebx

...
```

And `kmain` is updated accordingly:
`src/kernel/boot/kmain.c`
```c
...
void kmain(uint64_t multiboot_magic, void *multiboot_data)
{
...
```

## Reading the multiboot information

Reading the data passed from the bootloader is actually pretty straight
forward. It's stored as a list of "tags" with a 32 bit type and 32 bit
size followed by the data.

I just step through the tag list and save the data I want into a global
structure. The only possible pitfall is that tags are 8 byte alligned,
so finding the next tag can look like:

```c
int padded_size = tag->size + ((tag->size % 8)?(8-(tag->size%8)):0);
tag = incptr(tag, padded_size);
```

`incptr` is just a macro I made to move a pointer a number of bytes
forward or backward. Quite useful.

```c
#define incptr(p, n) ((void *)(((uintptr_t)(p)) + (n)))
```

For now, I only save the bootloader name (type 2) and kernel commandline
(type 1), and that's just for testing purposes. Later we'll also want to
save and parse the memory map (type 6) and any modules (type 3).
