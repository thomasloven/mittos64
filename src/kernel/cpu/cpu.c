#include <cpu.h>
#include <interrupts.h>

void cpu_init()
{
  interrupt_init();
}
