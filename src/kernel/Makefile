ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

CC := x86_64-elf-gcc

SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))
DEP := $(OBJ:.o=.d)

CFLAGS ?= -Wall -Wextra
CFLAGS += -ffreestanding -ggdb -O0
ASFLAGS += -ggdb
CPPFLAGS += -I include

all: kernel

kernel: LDFLAGS += -n -nostdlib -T Link.ld
kernel: $(OBJ)
	$(LINK.c) $^ -o $@

DEPFLAGS = -MT $@ -MMD -MP -MF $*.d
%.o: %.c $.d
	$(COMPILE.c) $(DEPFLAGS) -o $@ $<
%.o: %.S %.d
	$(COMPILE.S) $(DEPFLAGS) -o $@ $<
%.d: ;


# Copy kernel to sysroot
$(BUILDROOT)sysroot/kernel: kernel
	mkdir -p $(BUILDROOT)sysroot
	cp kernel $(BUILDROOT)sysroot/kernel

install: $(BUILDROOT)sysroot/kernel

clean:
	rm -rf $(OBJ) $(DEP) kernel

.PHONY: install

include $(DEP)
