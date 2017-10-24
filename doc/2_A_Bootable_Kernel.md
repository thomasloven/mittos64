# Chapter 2 - Booting a Kernel

In this chapter we'll create a minimal kernel that can be loaded by a Multiboot compatible bootloader.

## Makefile

Gnu Make is a powerful tool. I don't think most people realize just how powerful it is. It's really ridiculously smart, if you trust it.

As an example, I'm currently going through the [Build Your Own Text Editor](http://viewsourcecode.org/snaptoken/kilo/) booklet. Here's my Makefile for the project:
```make
CFLAGS := -Wall -Wextra -pedantic -std=c99

kilo:
```

And Make takes care of the rest. Of course, the makefile for a kernel
will be a bit more complex, but we'll trust Make as far as possible.

Just for fun, you can run `make -p` to see all the built-in rules
of Make, some of which we will make use of, such as `$(LINK.C)` or
`$(COMPILE.S)`

But let's get into it. First we create a directory for our sourc files,
and the kernel sources specifically. In it, we place a Makefile:

`src/kernel/Makefile`
```make
ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

CC := x86_64-elf-gcc

SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))

CFLAGS := -Wall -Wextra -pedantic -ffreestanding
CFLAGS += -ggdb -O0
ASFLAGS += -ggdb
CPPFLAGS += -I include
LDFLAGS := -n -nostdlib -lgcc -T Link.ld

kernel: $(OBJ)
	$(LINK.c) $^ -o $@


DEP := $(OBJ:.o=.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d
$(OBJ): CPPFLAGS += $(DEPFLAGS)
%.d: ;

DESTDIR ?= $(BUILDROOT)sysroot

# Copy kernel to sysroot
$(DESTDIR)$(PREFIX)/kernel: kernel
	install -D kernel $(DESTDIR)$(PREFIX)/kernel

install: $(DESTDIR)$(PREFIX)/kernel

clean:
	rm -rf $(OBJ) $(DEP) kernel

.PHONY: install

include $(DEP)
```

I think there are a few things here that needs explanation.

First of all, there's the environment check

```make
ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif
```

This is just as discussed in the previous chapter, to make sure the
makefile is only run inside the Docker container.

Then we set `CC := x86_64-elf-gcc` to make sure the kernel is build
using our cross compiler. If you look through the built in Make
rules, you'll see that `CC` is used for compiling .c files,
compiling .cpp files, compiling .S files and linking it all
together.

The next three lines

```make
SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))
```

scan the source directory and all subdirectories for `.c` or `.S` files, and find the names of the corresponding object files.

Then we set some compiler and linker options

```make
CFLAGS := -Wall -Wextra -pedantic -ffreestanding
CFLAGS += -ggdb -O0
ASFLAGS += -ggdb
CPPFLAGS += -I include
LDFLAGS := -n -nostdlib -lgcc -T Link.ld
```

- We want to compile with `-Wall -Wextra and -pedantic` to help keep the
code quality high.
- `-ffreestanding` stops the compiler from assuming that there's a
standard c library present.
- `-ggdb and -O0` simplifies debugging by including gdb specific debug
symbols in the compiled object code and turning off all optimizations
- and we also want `-ggdb` for assembly files.
-The c preprocessor should look for include files in
`src/kernel/include`, that's what the `-I include` option does.
- And finally, there's the linker options. `-nostdlib` because we don't
want any libraries but `libgcc` to be linked with our kernel. `-n` tells
the linker that we don't necessarily want the sections page aligned.
`-T Link.ld` tells the linker that the file `src/kernel/Link.ld`
contains more information about how we want the file structured.

The next two lines:

```make
kernel: $(OBJ)
	$(LINK.c) $^ -o $@
```

tells `make` that the file `kernel` depends on all our objects and how
to build it. Here we're using the built in `$(LINK.c)` rule.

And that's really all we need to build the kernel. 15 lines. The rest of
the file is just for convenience.

Like the lines:

```make
DEP := $(OBJ:.o=.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d
$(OBJ): CPPFLAGS += $(DEPFLAGS)
%.d: ;
[...]
include $(DEP)
```

which deserve some explanation.

If you update a source file and run make, it will know which files were
updated and need to be rebuilt. But what about included header files? If
they update, you'd want to rebuild all files that include them.

The book `Managing Projects with GNU Make` by Robert Meclenburg suggests
a solution that uses the `-M` option of gcc to generate a list of
dependencies from a source file, coupled with a mess of temporary files
and `sed` commands to generate make rules that you can import. I've seen
this method used in lots of projects.

I am sorry to say I can't remember where I picked this up - I certainly
didn't come up with it myself - but it turns out that through the use
of a bunch of more `gcc` options, the rules can be generated exactly as
we want them. `-MT $@` for example changes the name of the generated
rule to the input file - which is what Meclenburg does with `sed
's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$ > $@;`.

I just think this is a very neat solution.

The part of the Makefile I skipped is for "installing" the kernel, i.e.
copying it to the sysroot directory and for cleaning up.

### Bonus
For convenience, I added a makefile to the project root directory which
contains the following rule

```make
kernel:
ifeq ($(shell make -sqC src/kernel || echo 1), 1)
	$(MAKE) -C src/kernel install
endif
```

## Linking

As described above, the linker gets passed a linker script. This is as
basic as possible for now.

`src/kernel/Link.ld`

```Linker Script
ENTRY(_start)

SECTIONS
{
  .text :
  {
    *(.multiboot)
    *(.text)
  }
}
```

What's important here is that the `*(.multiboot)` statement is first
inside the `.text` section. The multiboot headers (described in the next
section) need to be near the beginning of the kernel executable, or the
bootloader won't be able to find them. As the kernel grows, this will
ensure they can still be found.

Note that you can't do this:

```Linker Script
SECTIONS
{
  .multiboot : {*(.multiboot)}
  .text : {*(.text)}
}
```

Oh, how I wish this worked. But for some reason, the linker will put the
`.text` section first in the output file, no matter what.

The linker script will soon grow, but for now, this is enough.

## Multiboot Headers

There are currently two actively used versions of the multiboot
specification, "Multiboot 1" (latest version is 0.6.96) and "Multiboot
2" (latest version is 1.6). Since Multiboot 2 has better support for 64
bit kernels, that's what I'll use.

In order for the bootloader to be able to recognize a multiboot kernel,
there has to be a special header near the start of the executable. Via
our linker file trick above, we can place the header at the very start
(actually after the program headers, but that's OK) by making sure it's
in the `.multiboot` section.

`src/kernel/boot/multiboot_header.S`

```asm
#include <multiboot.h>
.section .multiboot

.align 0x8
Multiboot2Header:
.long MBOOT2_MAGIC
.long MBOOT2_ARCH
.long MBOOT2_LENGTH
.long MBOOT2_CHECKSUM

.short 0
.short 0
.long 8
Multiboot2HeaderEnd:
```

The Multiboot 2 header is followed by a list of tags to specify further
options. We only have one; the list termination tag.

The constants above are defined in `multiboot.h`.

`src/kernel/include/multiboot.h`

```c
#pragma once

#define MBOOT2_MAGIC 0xE85250D6
#define MBOOT2_REPLY 0x36D76289
#define MBOOT2_ARCH 0
#define MBOOT2_LENGTH (Multiboot2HeaderEnd - Multiboot2Header)
#define MBOOT2_CHECKSUM -(MBOOT2_MAGIC + MBOOT2_ARCH + MBOOT2_LENGTH)
```


This is just for the simplest possible multiboot headers. No frills or extra
functions.

For more information, see [Multiboot 1
specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
or [Multiboot 2
specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html).

## The kernel code

Ladies and gentlemen, I present to you: The Simplest Kernel

`src/kernel/boot/boot.S`

```asm
.intel_syntax noprefix
.section .text
.global _start
.code32
_start:
cli
jmp $
```

That's all you need to make sure the
booting procedure works as it should.

## Testing it out

Let's go.

```bash
# compile
$ d make
make -C src/kernel install
make[1]: Entering directory '/opt/src/kernel'
cc -ggdb -I include -MT boot/multiboot_header.o -MMD -MP -MF boot/multiboot_header.d  -c -o boot/multiboot_header.o boot/multiboot_header.S
cc -ggdb -I include -MT boot/boot.o -MMD -MP -MF boot/boot.d  -c -o boot/boot.o boot/boot.S
cc -Wall -Wextra -pedantic -ffreestanding -ggdb -O0 -I include -n -nostdlib -lgcc -T Link.ld  boot/multiboot_header.o boot/boot.o -o kernel
mkdir -p /opt/sysroot
cp kernel /opt/sysroot/kernel
make[1]: Leaving directory '/opt/src/kernel'
```

Then run the emulator as before with `d emul` and also start the debugger in
another terminal window with `d gdb`.

Now you can load the kernel debug symbols into gdb with the command

    (gdb) file sysroot/kernel
    A program is being debugged already
    Are you sure you want to change the file? (y or n) y
    Reading symbols from sysroot/kernel...done.

A small problem at this point is that GRUB will start the kernel in 32 bit
protected mode, while gdb assumes we're in 64 bit mode. You can see this by
running

    (gdb) show architecture
    The target architecture is set automatically (currently i386:x86-64)

So, to get the addresses and stuff correct, you can run

    (gdb) set architecture i386
    The target architecture is assumed to be i386

And finally, we're ready to start the emulator

    (gdb) c
    Continuing.

This will make the familliar `grub>` prompt show up in the emulator.

It's time to load our kernel

    grub> multiboot2 /kernel
    grub> boot

And... nothing should happen, that you can see...

But switch back to gdb and press ctrl+C to interrupt the emulator and you should see

    Program received signal SIGINT, Interrupt.
    _start () at boot/boot.S:9
    9      jmp $
    (gdb)

So, apparently, the processor is running our "kernel" and is stuck at the
infinite loop on line 9. Just as we expected!

The multiboot specification says that the bootloader should leave a magic
number in the `EAX` register after boot. Let's check it.

    (gdb) p/x $eax
    $1 = 0x36d76289

Exactly according to the specification. Great!

And with that everything seems to work!

## Some extra fancy stuff

You really don't want to keep typing all those grub and gdb
instructions in manually each time you test your OS, right?

The gdb commands can be added to `toolchain/gdbinit`. In the name of
cinsistency, the debug symbols should not be loaded from `sysroot/kernel`,
but from `/opt/sysroot/kernel`, or rather yet `${BUILDROOT}sysroot/kernel`.
Using environment variables in a gdb script is a bit tricky, but can be
done via the `python` function:

```gdb
python
import os
gdb.execute('file ' + os.environ['BUILDROOT'] + 'sysroot/kernel')
end
```

In order to make grub load the kernel automatically, you need to add a file
called `grub.cfg` at `/boot/grub` in the sysroot. I suggest you add this
file to `sysroot` while building the iso in `toolchain/mkiso`.

I did it this way

```bash
cat > ${sysroot}/boot/grub/grub.cfg << EOF
set timeout=1
set default=0

menuentry "mittos64" {
  multiboot2 /kernel
}

EOF
```

See the [grub 0.97 manual](https://www.gnu.org/software/grub/manual/legacy/grub.html) for more information.
