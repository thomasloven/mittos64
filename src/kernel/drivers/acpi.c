#include <smp.h>
#include <memory.h>
#include <debug.h>

struct rsdp
{
  uint8_t signature[8];
  uint8_t checksum;
  uint8_t OEMID[6];
  uint8_t revision;
  uint32_t rsdt;
  uint32_t length;
  uint64_t xsdt;
  uint8_t checksum2;
  uint8_t _[3];
}__attribute__((packed));

struct sdt
{
  uint8_t signature[4];
  uint32_t len;
  uint8_t revision;
  uint8_t checksum;
  uint8_t OEMID[6];
  uint8_t table_ID[8];
  uint32_t OEM_revision;
  uint32_t creator;
  uint32_t creator_rev;
  uint8_t data[];
}__attribute__((packed));

static struct rsdp *find_rsdp()
{
  uintptr_t ebda_start = *(uint16_t *)P2V(0x40e);
  uintptr_t ebda_end = ebda_start + 1024;

  void *p = P2V(ebda_start);
  while(p < P2V(ebda_end))
  {
    if(!memcmp(p, "RSD PTR ", 8))
    {
      debug_info("RSDP found at:%x\n", p);
      return p;
    }
    p = incptr(p, 16);
  }

  p = P2V(0xE0000);
  while(p < P2V(0xFFFFF))
  {
    if(!memcmp(p, "RSD PTR ", 8))
    {
      debug_info("RSDP found at:%x\n", p);
      return p;
    }
    p = incptr(p, 16);
  }

  return 0;
}

static void parse_sdt(struct sdt *sdt, uint8_t revision)
{
  uint32_t *p32 = (void *)sdt->data;
  uint64_t *p64 = (void *)sdt->data;
  int entries = (sdt->len - sizeof(struct sdt)) / (revision ? 8 : 4);
  for(int i = 0; i < entries; i++)
  {
    struct sdt *table = P2V(revision ? p64[i] : p32[i]);
    debug_info("Found table: ");
    debug_putsn((char *)table->signature, 4);
    debug_printf("\n");
  }
}

void acpi_init()
{
  struct rsdp *rsdp = find_rsdp();
  struct sdt *s = P2V(rsdp->revision ? rsdp->xsdt : rsdp->rsdt);
  parse_sdt(s, rsdp->revision);
}
