#include <stdint.h>
#include <cpu.h>
#include <memory.h>

#define GDT_CODE      (3<<11)
#define GDT_DPL(lvl)  ((lvl)<<13)
#define GDT_PRESENT   (1<<15)
#define GDT_LONG      (1<<21)
#define GDT_TSS       (1<<8)

struct gdt
{
  uint32_t addr;
  uint32_t flags;
}__attribute__((packed));

struct gdtp
{
  uint16_t len;
  struct gdt *gdt;
}__attribute__((packed));

struct tss
{
  uint32_t r1;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t r2;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t r3;
  uint16_t r4;
  uint16_t io_mba;
}__attribute__((packed));


struct gdt BootGDT[] = {
  {0, 0},
  {0, GDT_PRESENT | GDT_DPL(0) | GDT_CODE | GDT_LONG},
  {0, GDT_PRESENT | GDT_DPL(3) | GDT_CODE | GDT_LONG},
  {0, 0},
  {0, 0},
};

struct gdtp GDTp = {2*8-1, BootGDT};


void gdt_init(uint64_t *_gdt, void *_tss)
{
  struct gdt *gdt = (struct gdt *)_gdt;
  memcpy(gdt, BootGDT, sizeof(BootGDT));

  uint32_t tss_limit = sizeof(struct tss);
  uint64_t tss_base = (uint64_t)_tss;
  gdt[3].flags = GDT_PRESENT | GDT_TSS;
  gdt[3].flags |= (((tss_base >> 24) & 0xFF) << 24);
  gdt[3].flags |= ((tss_base >> 16) & 0xFF);
  gdt[3].flags |= (((tss_limit >> 16) & 0xF) << 16);
  gdt[3].addr = ((tss_base & 0xFFFF) << 16) | (tss_limit & 0xFFFF);
  gdt[4].flags = 0;
  gdt[4].addr = ((tss_base >> 32) & 0xFFFF);

  GDTp.len = 5*8 - 1;
  GDTp.gdt = gdt;

  load_gdt(&GDTp);
}
