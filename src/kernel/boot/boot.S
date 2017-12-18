#include <memory.h>
.intel_syntax noprefix

.section .bss
.align PAGE_SIZE
.skip PAGE_SIZE
BootStack:
.byte 0

.section .text

.code32
error:
  //; Print "ERR!" in upper left corner
  mov [0xb8000], dword ptr 0x4f004f00
  mov [0xb8004], dword ptr 0x4f004f00
  mov [0xb8000], byte ptr 'E'
  mov [0xb8002], byte ptr 'R'
  mov [0xb8004], byte ptr 'R'
  mov [0xb8006], byte ptr '!'
  jmp $

check_cpuid:
  //; Save EFLAGS
  pushfd

  //; Try to modify EFLAGS.ID
  pushfd
  xor dword ptr [esp], 1<<21
  popfd

  //;Read EFLAGS into eax
  pushfd
  pop eax

  //; Restore original EFLAGS
  popfd

  //; Check if EFLAGS.ID was modified
  xor eax, [esp+4]
  and eax, 1<<21
  jz error
  ret

check_longmode:
  //; cpuid 0x8000 0000
  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  jb error

  //; cpuid 0x8000 0001
  mov eax, 0x80000001
  cpuid

  //; Test for long mode flag
  test edx, 1<<29
  jz error
  ret

.global _start
_start:
  cli

  mov esp, offset BootStack

  call check_cpuid
  call check_longmode

  //; Set CR4.PAE
  //; enabling Page Address Extension
  mov eax, cr4
  or eax, 1<<5
  mov cr4, eax

  //; Load a P4 page table
  mov eax, offset BootP4
  mov cr3, eax

  //; Set EFER.LME
  //; enabling Long Mode
  mov ecx, 0x0C0000080
  rdmsr
  or eax, 1<<8
  wrmsr

  //; Set CR0.PG
  //; enabling Paging
  mov eax, cr0
  or eax, 1<<31
  mov cr0, eax

  //; Load a new GDT
  lgdt [BootGDTp]

  //; and update the code selector by a long jump
  jmp 0x8:long_mode_start

.code64
  long_mode_start:

  //; Clear out all other selectors
  mov eax, 0x0
  mov ss, eax
  mov ds, eax
  mov es, eax

  //; Loop infinitely
  jmp $
