#pragma once
#include <stdint.h>
#include <process.h>

struct cpu
{
  void *cpu;
  uint64_t gdt[6];
  uint8_t tss[104];
  struct process *proc;
  struct process *scheduler;
};

extern struct cpu __seg_gs *cpu;

void cpu_init();

void interrupt_stack(void *rsp0);

void load_idt(void *);
void load_gdt(void *);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
void write_cr3(uint64_t);
uint64_t read_cr4();

// Model Specific Registers
// Functions defined in cpu/registers.S
void write_msr(uint64_t reg, uint64_t value);
uint64_t read_msr(uint64_t reg);
#define KERNEL_GS_BASE 0xC0000102
