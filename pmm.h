#ifndef RYAZIX_PMM_H
#define RYAZIX_PMM_H

#include <stdint.h>

#define PMM_BLOCK_SIZE 4096U

void pmm_init(uint32_t mem_size);
void *pmm_alloc_block(void);
void pmm_free_block(void *addr);

#endif