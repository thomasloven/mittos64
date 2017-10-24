ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

CC := x86_64-elf-gcc

SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))

CFLAGS ?= -Wall -Wextra
CFLAGS += -ffreestanding

all: kernel

kernel: LDFLAGS += -n -nostdlib -T Link.ld
kernel: $(OBJ)
	$(LINK.c) $^ -o $@
	mkdir -p /opt/sysroot
	cp kernel /opt/sysroot/kernel
