#pragma once
#include <stdint.h>

void cpu_init();

extern uint8_t tss[];
void set_tss_rsp0(void *tss, void *rsp0);


void load_idt(void *);
void load_gdt(void *);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
void write_cr3(uint64_t);
uint64_t read_cr4();
