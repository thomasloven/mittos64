#pragma once
#include <stdint.h>

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

#define debug_print_registers(r) \
debug("RAX=%016x RBX=%016x RCX=%016x RDX=%016x\n", r->rax, r->rbx, r->rcx, r->rdx); \
debug("RSI=%016x RDI=%016x RBP=%016x RSP=%016x\n", r->rsi, r->rdi, r->rbp, r->rsp); \
debug("R8 =%016x R9 =%016x R10=%016x R11=%016x\n", r->r8, r->r9, r->r10, r->r11); \
debug("R12=%016x R13=%016x R14=%016x R15=%016x\n", r->r12, r->r13, r->r14, r->r15); \
debug("RIP=%016x RFL=%016x\n", r->rip, r->rflags); \
debug("CS=%016x SS=%016x\n", r->cs, r->ss);


void load_idt(struct idtr *);

#define NUM_INTERRUPTS 256
void interrupt_init();
