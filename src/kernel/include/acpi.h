#pragma once
#include <stdint.h>

#define MAX_CPUS 16
#define MAX_IOAPIC 4

struct acpi_info
{
  int num_cpus;
  struct {
    uint8_t id;
    uint8_t apic;
  } cpu[MAX_CPUS];

  int num_ioapic;
  struct {
    uint8_t id;
    uint32_t addr;
    uint32_t base;
  } ioapic[MAX_IOAPIC];

  uint32_t int_map[256];
};
extern struct acpi_info acpi_info;

void acpi_init();
int get_cpu();
