#pragma once
#include <stdint.h>
struct kernel_boot_data_st
{
  int multiboot_version;
  char *bootloader;
  char *commandline;
};

extern struct kernel_boot_data_st kernel_boot_data;

int multiboot_init(uint64_t magic, void *mboot_info);
