#include <multiboot.h>
#include <memory.h>
#include <debug.h>

#define MBOOT_REPLY 0x36D76289

struct taglist{
  uint32_t total_size;
  uint32_t reserved;
}__attribute__((packed));

struct tag {
  uint32_t type;
  uint32_t size;
  uint8_t data[];
}__attribute__((packed));

#define MBOOT2_COMMANDLINE 1
#define MBOOT2_BOOTLOADER 2

struct kernel_boot_data_st kernel_boot_data;

int parse_multiboot2(struct taglist *tags)
{
  struct tag *tag = incptr(tags, sizeof(struct taglist));
  while(tag->type)
  {
    switch(tag->type)
    {
      case MBOOT2_BOOTLOADER:
        kernel_boot_data.bootloader = (char *)tag->data;
        break;
      case MBOOT2_COMMANDLINE:
        kernel_boot_data.commandline = (char *)tag->data;
        break;
      default:
        debug_warning("Unknown multiboot tag type:%d \n", tag->type);
    }
    int padded_size = tag->size + ((tag->size % 8)?(8-(tag->size%8)):0);
    tag = incptr(tag, padded_size);
  }
  return 0;
}

int multiboot_init(uint64_t magic, void *mboot_info)
{
  if(magic == MBOOT_REPLY)
  {
    kernel_boot_data.multiboot_version = 2;
    parse_multiboot2(mboot_info);
  }
  else
    return 1;

  return 0;
}
