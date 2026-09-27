#ifndef RYAZIX_GDT_H
#define RYAZIX_GDT_H

#include <stdint.h>

typedef struct gdt_entry_struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_struct;

typedef struct gdt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_struct;

void gdt_init(void);

#endif