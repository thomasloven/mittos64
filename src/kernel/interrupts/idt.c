#include <interrupts.h>
#include <stdint.h>
#include <memory.h>
#include <debug.h>

struct int_gate_descriptor idt[NUM_INTERRUPTS];
struct idtr idtr;

extern uintptr_t isr_table[];

void idt_set_gate(uint32_t num, uintptr_t vector, uint16_t cs, uint8_t ist, uint8_t flags)
{
  idt[num].base_l = vector & 0xFFFF;
  idt[num].base_m = (vector >> 16) & 0xFFFF;
  idt[num].base_h = (vector >> 32) & 0xFFFFFFFF;
  idt[num].cs = cs;
  idt[num].ist = ist;
  idt[num].flags = flags;
}

void interrupt_init()
{
  memset(idt, 0, sizeof(idt));

  for(int i=0; i < NUM_INTERRUPTS; i++)
  {
    idt_set_gate(i, isr_table[i], 0x8, 0, IDT_PRESENT | IDT_DPL0 | IDT_INTERRUPT);
  }

  idtr.addr = idt;
  idtr.len = sizeof(idt)-1;
  load_idt(&idtr);
}

registers *int_handler(registers *r)
{
  (void)r;
  debug("Unhandled interrupt occurred\n");
  debug("Interrupt number: %d Error code: %d\n", r->int_no, r->err_code);
  debug_print_registers(r);
  for(;;);
}
