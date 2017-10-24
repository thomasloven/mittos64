ifeq ($(MITTOS64),)
$(error Unsupported environment! See README)
endif

.PHONY: kernel clean

all: kernel

kernel:
ifeq ($(shell make -sqC src/kernel || echo 1), 1)
	$(MAKE) -C src/kernel install
endif

clean:
	$(MAKE) -C src/kernel clean
	rm -rf sysroot

