#ifndef RYAZIX_VMM_H
#define RYAZIX_VMM_H

#include <stdint.h>

#define VMM_PAGE_SIZE 4096U
#define VMM_PAGE_ENTRIES 1024U

typedef struct page_table_struct {
    uint32_t entries[VMM_PAGE_ENTRIES];
} __attribute__((aligned(VMM_PAGE_SIZE))) page_table_t;

typedef struct page_directory_struct {
    uint32_t entries[VMM_PAGE_ENTRIES];
} __attribute__((aligned(VMM_PAGE_SIZE))) page_directory_t;

void vmm_init(void);
void *vmm_get_page_directory(void);

#endif