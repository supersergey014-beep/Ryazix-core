#ifndef RYAZIX_KHEAP_H
#define RYAZIX_KHEAP_H

#include <stdint.h>

void kheap_init(void);
void *kmalloc(uint32_t size);
void kfree(void *ptr);

#endif