#include "gdt.h"

static gdt_entry_struct gdt_entries[5];
static gdt_ptr_struct gdt_ptr;

extern void gdt_flush(const gdt_ptr_struct *ptr);

static void gdt_set_gate(uint8_t index, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t granularity)
{
    gdt_entries[index].base_low = (uint16_t)(base & 0xFFFF);
    gdt_entries[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt_entries[index].base_high = (uint8_t)((base >> 24) & 0xFF);
    gdt_entries[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt_entries[index].granularity =
        (uint8_t)(((limit >> 16) & 0x0F) | (granularity & 0xF0));
    gdt_entries[index].access = access;
}

void gdt_init(void)
{
    gdt_ptr.limit = (uint16_t)(sizeof(gdt_entries) - 1);
    gdt_ptr.base = (uint32_t)&gdt_entries[0];

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush(&gdt_ptr);
}