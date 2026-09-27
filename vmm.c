#include "vmm.h"

#define VMM_PRESENT 0x001U
#define VMM_WRITABLE 0x002U
#define VMM_IDENTITY_TABLES 4U

static page_directory_t page_directory;
static page_table_t page_tables[VMM_IDENTITY_TABLES];

void vmm_init(void)
{
    uint32_t directory_index;
    uint32_t table_index;

    for (directory_index = 0; directory_index < VMM_PAGE_ENTRIES;
         ++directory_index) {
        page_directory.entries[directory_index] = 0;
    }

    for (directory_index = 0; directory_index < VMM_IDENTITY_TABLES;
         ++directory_index) {
        for (table_index = 0; table_index < VMM_PAGE_ENTRIES; ++table_index) {
            uint32_t address =
                (directory_index * VMM_PAGE_ENTRIES + table_index) * VMM_PAGE_SIZE;
            page_tables[directory_index].entries[table_index] =
                address | VMM_PRESENT | VMM_WRITABLE;
        }

        page_directory.entries[directory_index] =
            ((uint32_t)&page_tables[directory_index]) |
            VMM_PRESENT | VMM_WRITABLE;
    }

    __asm__ volatile("movl %0, %%cr3" : : "r"(&page_directory) : "memory");

    {
        uint32_t cr0;
        __asm__ volatile("movl %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000U;
        __asm__ volatile("movl %0, %%cr0" : : "r"(cr0) : "memory");
    }
}

void *vmm_get_page_directory(void)
{
    return &page_directory;
}