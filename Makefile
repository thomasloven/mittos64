ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

CC = x86_64-elf-gcc

all: kernel

kernel:
ifeq ($(shell make -sqC src/kernel || echo 1), 1)
	CC=$(CC) $(MAKE) -C src/kernel install
endif

clean:
	$(MAKE) -C src/kernel clean
	rm -rf sysroot

.PHONY: kernel clean
