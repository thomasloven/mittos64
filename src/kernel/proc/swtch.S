.intel_syntax noprefix

.global swtch

swtch:
  push rbp
  mov rbp, rsp
  push r15
  push r14
  push r13
  push r12
  push rbx
  push rbp

  mov [rdi], rsp
  mov rsp, [rsi]

  pop rbp
  pop rbx
  pop r12
  pop r13
  pop r14
  pop r15
  leaveq
  ret
