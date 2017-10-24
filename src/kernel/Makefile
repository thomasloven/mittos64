ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

CC := x86_64-elf-gcc

SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))

CFLAGS ?= -Wall -Wextra
CFLAGS += -ffreestanding
CPPFLAGS += -I include

all: kernel

kernel: LDFLAGS += -n -nostdlib -T Link.ld
kernel: $(OBJ)
	$(LINK.c) $^ -o $@


# Copy kernel to sysroot
/opt/sysroot/kernel: kernel
	mkdir -p /opt/sysroot
	cp kernel /opt/sysroot/kernel

install: /opt/sysroot/kernel

clean:
	rm -rf $(OBJ) kernel

.PHONY: install
