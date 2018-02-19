#include <stdint.h>
#include <cpu.h>
#include <memory.h>

#define GDT_CODE      (3<<11)
#define GDT_DPL(lvl)  ((lvl)<<13)
#define GDT_PRESENT   (1<<15)
#define GDT_LONG      (1<<21)


struct gdt
{
  uint32_t _;
  uint32_t flags;
}__attribute__((packed));

struct gdtp
{
  uint16_t len;
  struct gdt *gdt;
}__attribute__((packed));


struct gdt BootGDT[] = {
  {0, 0},
  {0, GDT_PRESENT | GDT_DPL(0) | GDT_CODE | GDT_LONG}
};

struct gdtp GDTp = {2*8-1, BootGDT};


void gdt_init(uint64_t *_gdt)
{
  struct gdt *gdt = (struct gdt *)_gdt;
  memcpy(gdt, BootGDT, sizeof(BootGDT));

  GDTp.len = 2*8 - 1;
  GDTp.gdt = gdt;

  load_gdt(&GDTp);
}
