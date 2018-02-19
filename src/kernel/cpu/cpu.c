#include <cpu.h>
#include <interrupts.h>

uint64_t gdt[2];
void gdt_init(uint64_t *);

void cpu_init()
{
  interrupt_init();
  gdt_init(gdt);
}
