#pragma once
#include <stdint.h>
#include <stddef.h>
struct kernel_boot_data_st
{
  int multiboot_version;
  char *bootloader;
  char *commandline;
  size_t mmap_size;
  void *mmap;
};

#define MMAP_FREE 1

extern struct kernel_boot_data_st kernel_boot_data;

int multiboot_init(uint64_t magic, void *mboot_info);
int multiboot_get_memory_area(size_t count, uintptr_t *start, uintptr_t *end, uint32_t *type);
