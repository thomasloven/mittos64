#pragma once
#include <stdint.h>
struct kernel_boot_data_st
{
  int multiboot_version;
  char *bootloader;
  char *commandline;
};

struct kernel_boot_data_st kernel_boot_data;

struct mboot2_taglist{
  uint32_t total_size;
  uint32_t reserved;
}__attribute__((packed));
struct mboot2_tag {
  uint32_t type;
  uint32_t size;
  uint8_t data[];
}__attribute__((packed));

#define MBOOT2_COMMANDLINE 1
#define MBOOT2_BOOTLOADER 2

int parse_multiboot(uint64_t magic, void *mboot_info);
