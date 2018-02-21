.intel_syntax noprefix

.global load_idt
load_idt:
  lidt [rdi]
  ret

.global load_gdt
load_gdt:
  lgdt [rdi]
  mov ax, 0x0
  mov ss, ax
  mov ds, ax
  mov es, ax
  movabs rax, offset .load_gdt
  pushq 0x8
  push rax
  retfq
.load_gdt:
  mov rax, 0x18
  ltr ax
  ret

.global read_cr0
read_cr0:
  mov rax, cr0
  ret
.global read_cr2
read_cr2:
  mov rax, cr2
  ret
.global read_cr3
read_cr3:
  mov rax, cr3
  ret
.global write_cr3
write_cr3:
  mov cr3, rdi
  ret
.global read_cr4
read_cr4:
  mov rax, cr4
  ret
