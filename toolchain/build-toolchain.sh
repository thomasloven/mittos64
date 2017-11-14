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
