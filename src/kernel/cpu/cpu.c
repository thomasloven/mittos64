#include <cpu.h>
#include <interrupts.h>

uint64_t global_gdt[6];
uint8_t global_tss[104];
void gdt_init();

void cpu_init()
{
  interrupt_init();
  gdt_init();
}
