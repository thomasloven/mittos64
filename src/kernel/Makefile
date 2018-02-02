ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

SRC := $(wildcard **/*.[cS])
OBJ := $(patsubst %, %.o, $(basename $(SRC)))

CFLAGS ?= -Wall -Wextra -pedantic
CFLAGS += -ffreestanding
CFLAGS += -ggdb -O0
ASFLAGS += -ggdb
CPPFLAGS += -I include
LDFLAGS := -n -nostdlib -lgcc -T Link.ld

kernel: $(OBJ)
	$(LINK.c) $^ -o $@

# Automatically generate dependency files
# Those keep track of which header files are used by which source files
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

.PHONY: install clean

# Include generated dependency files
include $(DEP)
