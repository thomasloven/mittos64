#include <cpu.h>
#include <interrupts.h>
#include <memory.h>

void gdt_init(struct cpu *cpu);

struct cpu *cpu;

void cpu_init()
{
  cpu = P2V(pmm_calloc());
  interrupt_init();
  gdt_init(cpu);
}

