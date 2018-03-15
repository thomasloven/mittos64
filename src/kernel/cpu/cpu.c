#include <cpu.h>
#include <interrupts.h>
#include <memory.h>
#include <debug.h>

void gdt_init(struct cpu *cpu);

struct cpu __seg_gs *cpu = 0;

void cpu_init()
{
  struct cpu *c = P2V(pmm_calloc());
  c->cpu = c;
  write_msr(0xc0000102, (uint64_t)c);
  asm("swapgs");

  interrupt_init();
  gdt_init(cpu->cpu);
}

