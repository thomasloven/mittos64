#pragma once
#include <stdint.h>

void cpu_init();

extern uint64_t global_gdt[];
extern uint8_t global_tss[];
#define GDT() ((void *)global_gdt)
#define TSS() ((void *)global_tss)

void interrupt_stack(void *rsp0);


void load_idt(void *);
void load_gdt(void *);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
void write_cr3(uint64_t);
uint64_t read_cr4();
