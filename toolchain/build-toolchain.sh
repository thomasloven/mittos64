#!/bin/sh -e

# Tools and libraries required to build binutils and gcc
apk --update add build-base
apk add gmp-dev mpfr-dev mpc1-dev

# For making a bootable iso
apk add grub-bios xorriso
# For debugging
apk add gdb valgrind
# We need a later version of qemu than included in the default repo
# due to a bug in the qemu gdb server
apk --update add qemu-system-x86_64 --repository http://dl-cdn.alpinelinux.org/alpine/v3.7/main

rm -rf /var/cache/apk/*

# For now we'll use the default x86_64-elf target
target=x86_64-elf
binutils=binutils-2.29
gcc=gcc-7.2.0


# Configure, make and install binutils
# configured for our target
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

# Configure, make and install gcc and libgcc
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

cd /
# Remove apk cache
rm -rf /opt
