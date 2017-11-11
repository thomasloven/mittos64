.intel_syntax noprefix

.section .bss
.align 0x1000
.skip 0x1000
BootStack:
.byte 0

.section .text

.code32
error:
  // Print "ERR!" in upper left corner
  mov [0xb8000], dword ptr 0x4f524f45
  mov [0xb8004], dword ptr 0x4f214f52
  jmp $

check_cpuid:
  pushfd
  pushfd
  xor dword ptr[esp], 1<<21
  popfd
  pushfd
  pop eax
  xor eax, [esp]
  popfd
  and eax, 1<<21
  jz error
  ret

check_longmode:
  // cpuid 0x8000 0000
  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  jb error

  // cpuid 0x8000 0001
  mov eax, 0x80000001
  cpuid

  // Test for long mode flag
  test edx, 1<<29
  jz error
  ret

.global _start
_start:
cli

mov esp, offset BootStack

call check_cpuid
call check_longmode

jmp $
