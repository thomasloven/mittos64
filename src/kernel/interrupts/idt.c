#include <stdint.h>
#include <memory.h>
#include <debug.h>

struct int_gate_descriptor
{
  uint16_t base_l;
  uint16_t cs;
  uint8_t ist;
  uint8_t flags;
  uint16_t base_m;
  uint32_t base_h;
  uint32_t ignored;
} __attribute__((packed));

struct idtr
{
  uint16_t len;
  struct int_gate_descriptor *addr;
} __attribute__((packed));

typedef struct
{
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;

  uint64_t int_no;
  uint64_t err_code;

  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} registers;

void load_idt(struct idtr *);

#define NUM_INTERRUPTS 256

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
    idt_set_gate(i, isr_table[i], 0x8, 0, 0x8E);
  }

  idtr.addr = idt;
  idtr.len = sizeof(idt)-1;
  load_idt(&idtr);
}

registers *int_handler(registers *r)
{
  (void)r;
  debug("Interrupt %d\n", r->int_no);
  for(;;);
}
