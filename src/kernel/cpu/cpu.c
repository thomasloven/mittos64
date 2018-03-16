#include <cpu.h>
#include <interrupts.h>
#include <memory.h>
#include <debug.h>

void gdt_init(struct cpu *cpu);

struct cpu __seg_gs *cpu = 0;

void cpu_init()
{
  // Set up cpu struct
  struct cpu *c = P2V(pmm_calloc());
  c->cpu = c;
  write_msr(KERNEL_GS_BASE, (uint64_t)c);
  asm("swapgs");

  interrupt_init();
  gdt_init(cpu->cpu);
}

