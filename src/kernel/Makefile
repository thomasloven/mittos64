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


# Copy kernel to sysroot
$(BUILDROOT)sysroot/kernel: kernel
	mkdir -p $(BUILDROOT)sysroot
	cp kernel $(BUILDROOT)sysroot/kernel

install: $(BUILDROOT)sysroot/kernel

clean:
	rm -rf $(OBJ) $(DEP) kernel

.PHONY: install

include $(DEP)
