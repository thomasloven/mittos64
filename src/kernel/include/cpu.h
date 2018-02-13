#pragma once
#include <stdint.h>

void cpu_init();


void load_idt(void *);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
uint64_t read_cr4();
