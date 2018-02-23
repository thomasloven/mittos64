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

struct madt
{
  uint32_t lic_address;
  uint32_t flags;
  uint8_t data[];
}__attribute__((packed));
struct madt_entry
{
  uint8_t type;
  uint8_t len;
  union{
    struct {
      uint8_t id;
      uint8_t apic;
      uint32_t flags;
    }__attribute__((packed)) lapic;
    struct {
      uint8_t id;
      uint8_t _;
      uint32_t addr;
      uint32_t base;
    }__attribute__((packed)) ioapic;
    struct {
      uint8_t bus;
      uint8_t source;
      uint32_t target;
      uint16_t flags;
    }__attribute__((packed)) interrupt;
  };
}__attribute__((packed));

#define MADT_CPU 0
#define MADT_IOAPIC 1
#define MADT_INT 2

struct acpi_info acpi_info = {0};

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

static void parse_madt(struct madt *madt, uint32_t len)
{
  uintptr_t end = (uintptr_t)madt + len;
  struct madt_entry *e = (void *)madt->data;
  debug_info("Local Interrupt Controller: %x\n", madt->lic_address);
  while((uintptr_t)e < end)
  {
    int i;
    switch(e->type)
    {
      case MADT_CPU:
        i = acpi_info.num_cpus;
        acpi_info.cpu[i].id = e->lapic.id;
        acpi_info.cpu[i].apic = e->lapic.apic;
        acpi_info.num_cpus++;
        break;
      case MADT_IOAPIC:
        i = acpi_info.num_ioapic;
        acpi_info.ioapic[i].id = e->ioapic.id;
        acpi_info.ioapic[i].addr = e->ioapic.addr;
        acpi_info.ioapic[i].base = e->ioapic.base;
        acpi_info.num_ioapic++;
        break;
      case MADT_INT:
        acpi_info.int_map[e->interrupt.source] = e->interrupt.target;
        break;
    }
    debug_info(" MADT: type:%d len:%d\n", e->type, e->len);
    e = incptr(e, e->len);
  }
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
    if(!memcmp(table->signature, "APIC", 4))
      parse_madt((void *)table->data, table->len);
  }
}

void acpi_init()
{
  struct rsdp *rsdp = find_rsdp();
  struct sdt *s = P2V(rsdp->revision ? rsdp->xsdt : rsdp->rsdt);
  parse_sdt(s, rsdp->revision);
}
