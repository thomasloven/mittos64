.intel_syntax noprefix

.extern int_handler
.global isr_common
.global isr_return

isr_common:
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rbp
  push rdi
  push rsi
  push rdx
  push rcx
  push rbx
  push rax

  mov rax, [rsp + 152]
  and rax, (3<<12)
  jz .kernel_interrupt
  swapgs

.kernel_interrupt:
  mov rdi, rsp
  call int_handler

  mov rdi, rax
isr_return:
  mov rsp, rdi

  mov rax, [rsp + 152]
  and rax, (3<<12)
  jz .kernel_return
  swapgs

.kernel_return:
  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  pop rbp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  add rsp, 0x10
  iretq
