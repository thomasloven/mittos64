# Chapter 1 - Setting up a toolchain

In this chapter we'll build a docker image which contains all the tools
we need to build and emulate our OS.

We'll also make some helper scripts to run commands in the Docker
container and for running the emulator and debugger.


## Docker image

### Why docker?

I've heard the name Docker thrown around a lot the last year or two, but
only just recently started to look into it. The idea of using it for
compiling an operating system came to me almost immediately.

Docker lets you run processes inside a well defined, isolated and
portable, linux-based environment. What's there not to like?

So, let's build a Docker image for osdeving.

### What we want

For now, I want the following in the image:

- binutils
- gcc
- make
- grub
- xorriso
- qemu
- gdb

In order to get a known compiler configuration, we will be building
`binutils` and `gcc` from source. At this point, we'll only use a base
configuration, and could therefore probably use the versions that come
with the docker base linux image. Later, however, we'll patch them to
add new targets for compiling native usermode code for our OS, so we
might as well get the practice of compiling.

`Make` is just for simplifying the build process. An indispensable tool,
really, but more on that later.

`Grub` and `xorriso` is used to generate a bootable cdrom ISO with our
kernel. We'll need to make sure we get `grub` with BIOS support, though,
because that's what `qemu` expects.

`Qemu` for emulating. We won't need all of `qemu`, but most package
managers will let you install just one or a few system emulators. In our
case, we want `qemu-system-x86_64` specifically.

Finally `gdb` can attach to qemu and be used to inspect and change
memory, variables, code, registers. It has saved me inumerable times
already.

### Dockerfile

I chose to build my image on top of alpine linux, because that seems to
be generally accepted as best practice.

Alpine also happens to be built on musl c library, which is what I plan
to port to this OS. Not that it matters...

So...

`toolchain/Dockerfile`

```dockerfile
FROM alpine:3.6

ADD build-toolchain.sh /opt/build-toolchain.sh

RUN /opt/build-toolchain.sh

ENV PATH "/opt/toolchain:$PATH"
ENV MITTOS64 "true"
ENV BUILDROOT "/opt/"
WORKDIR /opt
```

This simply copies over the installation script, runs it and then sets a
few environment variables.

The installation script looks like this:

`toolchain/build-toolchain.sh`

```bash
#!/bin/sh -e

apk --update add build-base
apk add gmp-dev mpfr-dev mpc1-dev

apk add make
apk add grub-bios xorriso
apk add gdb
apk --update add qemu-system-x86_64 --repository http://dl-cdn.alpinelinux.org/alpine/v3.7/main

rm -rf /var/cache/apk/*

target=x86_64-elf
binutils=binutils-2.29
gcc=gcc-7.2.0


cd /opt
wget http://ftp.gnu.org/gnu/binutils/${binutils}.tar.gz
tar -xf ${binutils}.tar.gz
mkdir binutils-build && cd binutils-build
../${binutils}/configure \
  --target=${target} \
  --disable-nls \
  --disable-werror \
  --with-sysroot \

make -j 4
make install

cd /opt
wget http://ftp.gnu.org/gnu/gcc/${gcc}/${gcc}.tar.gz
tar -xf ${gcc}.tar.gz
mkdir gcc-build && cd gcc-build
../${gcc}/configure \
  --target=${target} \
  --disable-nls \
  --enable-languages=c \
  --without-headers \

make all-gcc all-target-libgcc -j 4
make install-gcc install-target-libgcc

apk del build-base

cd /
rm -rf /opt
```

First we use the alpine package manager `apk` to install the things we need for
compiling, `build-base` - which is compilers and stuff, and some libraries
needed to compile `gcc`. Then we install the packages discussed above and
finally download, configure, make and install binutils and gcc.  Note that make
is installed specifically, even though it's included in build-base. This is so
that we can uninstall build-base to save room, and still have make available.

> #### A note about qemu versions
> You'll note that qemu is installed from a different repository than the rest
> of the packages. This is because of a problem with the gdb server in some
> qemu versions. For some versions, gdb can't follow when qemu switches
> processor mode, e.g. from 32 to 64 bit execution. You will then get some
> error message about "g packet size" and some numbers. The way to solve this
> is to disconnect from the remote debugging, change architecture manually, and
> then reconnect:

> ```
> (gdb) disconnect
> (gdb) set architecture i386:x86_64:intel
> (gdb) target remote :1234
> ```

> Or you could make sure you're running qemu version 2.10 or later, which seems
> to have fixed this problem.

> At the time of writing, the lates alpine docker image is version 3.6, which
> installs qemu version 2.8 by default. Therefore, we manually choose the
> repository for alpine 3.7 instead.

The configuration flags are well described in the [GCC
Cross-Compiler](http://wiki.osdev.org/GCC_Cross-Compiler) article over at
[osdev.org](http://osdev.org), so I recommend you read that if you didn't
already. This also gives us a good base for later adding a custom build target.

The image can be built with `docker build -t mittos64 toolchain/.` and
when done, you can run a command inside it to test that it works:

```
$ docker run --rm mittos64 x86_64-elf-gcc --version
x86_64-elf-gcc (GCC) 7.2.0
Copyright (C) 2017 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

## Docker helper script

In order to compile our code inside Docker, we need to mount our source
directory to the container. This can be done with the -v flag, but at
this point things are starting to look messy, so let's write a script
for it.

I simply named it `d` and put it in the project root directory.

`d`

```bash
#!/usr/bin/env bash
imagename=mittos64
buildroot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

if [[ $(docker ps -q -f name=${imagename}-run) ]]; then
  docker exec -it -u $(id -u):$(id -g) ${imagename}-run "$@"
else
  docker run -it --rm -v ${buildroot}:/opt --name ${imagename}-run -u $(id -u):$(id -g) ${imagename} "$@"
fi
```

This will run any command inside the docker container as the calling user:

    $ ./d qemu-system-x86_64 --version
    QEMU emulator version 2.8.1
    Copyright (c) 2003-2016 Fabrice Bellard and the QEMU Project developers

Furthermore. If a command is already running in the container, the next
invocation of `d` will not launch a new container, but instead connect
to the currently running one. This means you can e.g. run `qemu` and
`gdb` inside the same container, so that they may talk to each other.

The command will mount the directory the `d` script resides in to `/opt`
in the container, which also is the default working directory, so we'll
have direct access to all our source.


## Making a bootable ISO file

When the kernel is compiled, we need to get it to a computer or emulator
somehow. `Qemu` can actually load a MultiBoot 1 compatible kernel
directly, but I believe making a bootable ISO is a more robust way to
go. This is obviously something we will need to do a lot of times, so
it's scripting time.

`toolchain/mkiso`

```bash
#!/bin/sh -e
if [ -z ${MITTOS64+x} ]; then >&2 echo "Unsupported environment! See README"; exit 1; fi

sysroot=${BUILDROOT}sysroot
iso=${BUILDROOT}mittos64.iso

mkdir -p ${sysroot}

grub-mkrescue -o ${iso} ${sysroot}
```

The first two lines need some explanation. First of all, I normally
write all my scripts for `bash` Alpine linux, however, does not include
`ash`by default. So instead we are going for `sh`.

The second line checks if the $MITTOS64 environment variable is set. If
it's not, execution stops immediately. This will be a feature of most
scripts within the project. This is just to avoid messing anything up
on your own computer. Remember that this variable was defined by the
Dockerfile.

`BUILDROOT` was also defined in the Dockerfile.

The rest of the script builds a `sysroot` directory (that is where our
boot filesystem will live, for now it's empty...) and then turns it all
into an ISO with grub installed.


## Running the emulator

We'll test the kernel using `qemu`.

`Qemu` has a lot of command line flags.

We don't want to type those in all the time.

Script time:

`toolchain/emul`

```bash
#!/bin/sh -e
if [ -z ${MITTOS64+x} ]; then >&2 echo "Unsupported environment! See README"; exit 1; fi

iso=${BUILDROOT}mittos64.iso

${BUILDROOT}toolchain/mkiso

qemu-system-x86_64 -s -S -cdrom ${iso} -curses
```

This should be simple enough. After the environment check, it runs the
`mkiso` script and then starts `qemu` with the options:

- -s to start a gdb server at telnet port 1234
- -S to freeze the cpu at startup and wait for a command to continue
- -cdrom ${iso} to mount our ISO as a cd
- -curses to output the screen (in VGA text mode) to the terminal

Later we'll add stuff like multiple cpus and VNC output to be able to
use graphics modes, but this is good enough for now.


## Debugger

Finally, we add a script to start `gdb` with some initial settings

`toolchain/gdb`

```bash
#!/bin/sh -e
if [ -z ${MITTOS64+x} ]; then >&2 echo "Unsupported environment! See README"; exit 1; fi

/usr/bin/gdb -q -x ${BUILDROOT}toolchain/gdbinit
```

This just runs `gdb` and tells it to read and execute
`toolchain/gdbinit`. Normally, you'd probably use a `.gdbinit` either
in your home directory, or in the project root, but I wanted it to be
visible (filenames starting with `.` are hidden in UNIX-like systems)
and together with the rest of the toolchain stuff. Hence the script -
which overloads `gdb` since it will come earlier in $PATH.

So, the important stuff in this section is actually the `gdbinit` file.

`toolchain/gdbinit`

```gdb
set prompt \033[31m(gdb) \033[0m
set disassembly-flavor intel

target remote :1234

define q
monitor quit
end

define reg
monitor info registers
end

define reset
monitor system_reset
end
```

This script does the following:

- Colors the gdb prompt read for improved visibility
- Makes gdb use intel assembly syntax, rather that AT&T. Personal preference.
- Connects to the `qemu` gdb server at port 1234
- redefines the `q` command to stop the emulator (this will kill `gdb`
as well). If you want to, you can still use `(gdb) quit` to quit just
the debugger.
- Defines a `reg` command which pulls in the register information from
`qemu`. This is more detailed than the `gdb` register output.
- Defines a `reset` command to reboot the emulator.


## Trying it all out

### Emulator

To make sure everything works, open up a terminal window and run

    $ d emul

After a second or two, you should get a blank screen with the text `VGA Blank mode`.
That means qemu is paused and waiting for a command to start.

You could start it by switching to the qemu monitor with `META+2` (if
you don't have a meta key, press and release `ESC` immediately followed
by `2`) and running

    (qemu) c

Then you can switch back to the VGA output with `META+1`.

Once the emulator is running, you should soon see the GRUB start screen:
`GNU GRUB version 2.02` followed by some text about tab completion and
then a grub prompt

    grub>

You can exit the emulator by going to the monitor and issuing

    (qemu) q

### Debugger

To check that the debugger works, start the emulator again with

    $ d emul

Then open another terminal window and run

    $ d gdb

You should se some text and then a `(gdb)` prompt. The emulator window
should still show `VGA Blank mode`.

Now run

    (gdb) c

and the emulator should start running and bring you to the `grub` prompt.
When you're there, pause executing with `CTRL+c` (in gdb), which brings
back the prompt.

You can now inspect the processor registers with

    (gdb) reg
    EAX=00000000 EBX=00000000 ECX=07fa0880 EDX=00000031
    ESI=00000000 edI=07fa0880 EBX)00001ff0 ESP=00001ff4
    [...]
    XMM06=00000000000000000000000000000000 XMM07=00000000000000000000000000000000

Finally, run

    (gdb) q

and notice that both the emulator and gdb stops.
