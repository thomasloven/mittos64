#include <cpu.h>
#include <interrupts.h>

uint64_t gdt[5];
uint8_t tss[104];
void gdt_init(uint64_t *, void *);

void cpu_init()
{
  interrupt_init();
  gdt_init(gdt, tss);
}
