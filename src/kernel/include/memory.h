#pragma once
#define KERNEL_OFFSET 0xFFFFFF8000000000

#ifdef __ASSEMBLER__
#define V2P(a) ((a) - KERNEL_OFFSET)
#define P2V(a) ((a) + KERNEL_OFFSET)
#else
#include <stdint.h>
#define V2P(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define P2V(a) ((void *)((uintptr_t)(a) | KERNEL_OFFSET))
#define incptr(p, n) ((void *)(((uintptr_t)(p)) + (n)))
#endif

#define P1_OFFSET(a) (((a)>>12) & 0x1FF)
#define P2_OFFSET(a) (((a)>>21) & 0x1FF)
#define P3_OFFSET(a) (((a)>>30) & 0x1FF)
#define P4_OFFSET(a) (((a)>>39) & 0x1FF)

#define PAGE_PRESENT      0x001
#define PAGE_WRITE        0x002
#define PAGE_HUGE         0x080

#define PAGE_SIZE       0x1000
#define ENTRIES_PER_PT  512

#ifndef __ASSEMBLER__
#include <stddef.h>
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);

void pmm_free(uintptr_t page);
uintptr_t pmm_alloc();
uintptr_t pmm_calloc();

uintptr_t vmm_get_page(void *P4, uintptr_t addr);
int vmm_set_page(void *P4, uintptr_t addr, uintptr_t page, uint16_t flags);
int touch_page(void *P4, uintptr_t addr, uint16_t flags);
void free_page(void *P4, uintptr_t addr, int free);

extern union PTE BootP4;
extern int kernel_start, kernel_end;
#endif
