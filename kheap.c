#include "kheap.h"

#define KHEAP_SIZE (64U * 1024U)
#define KHEAP_ALIGNMENT 16U

typedef struct heap_block_struct {
    uint32_t size;
    uint32_t free;
    struct heap_block_struct *next;
} __attribute__((aligned(KHEAP_ALIGNMENT))) heap_block_t;

static uint8_t heap_area[KHEAP_SIZE] __attribute__((aligned(KHEAP_ALIGNMENT)));
static heap_block_t *heap_first_block;
static uint8_t heap_initialized;

static uint32_t align_size(uint32_t size)
{
    return (size + KHEAP_ALIGNMENT - 1U) & ~(KHEAP_ALIGNMENT - 1U);
}

void kheap_init(void)
{
    heap_first_block = (heap_block_t *)heap_area;
    heap_first_block->size = KHEAP_SIZE - (uint32_t)sizeof(heap_block_t);
    heap_first_block->free = 1;
    heap_first_block->next = 0;
    heap_initialized = 1;
}

void *kmalloc(uint32_t size)
{
    heap_block_t *block;
    uint32_t aligned_size;

    if (size == 0 || size > UINT32_MAX - (KHEAP_ALIGNMENT - 1U)) {
        return 0;
    }
    if (!heap_initialized) {
        kheap_init();
    }
    aligned_size = align_size(size);

    for (block = heap_first_block; block != 0; block = block->next) {
        if (!block->free || block->size < aligned_size) {
            continue;
        }

        if (block->size - aligned_size >=
            sizeof(heap_block_t) + KHEAP_ALIGNMENT) {
            heap_block_t *remainder = (heap_block_t *)
                ((uint8_t *)(block + 1) + aligned_size);
            remainder->size = block->size - aligned_size -
                              (uint32_t)sizeof(heap_block_t);
            remainder->free = 1;
            remainder->next = block->next;
            block->next = remainder;
            block->size = aligned_size;
        }

        block->free = 0;
        return (void *)(block + 1);
    }

    return 0;
}

void kfree(void *ptr)
{
    heap_block_t *block;
    heap_block_t *previous = 0;

    if (ptr == 0 || !heap_initialized) {
        return;
    }

    for (block = heap_first_block; block != 0; previous = block, block = block->next) {
        if ((void *)(block + 1) == ptr) {
            if (block->free) {
                return;
            }

            block->free = 1;
            if (block->next != 0 && block->next->free) {
                block->size += (uint32_t)sizeof(heap_block_t) + block->next->size;
                block->next = block->next->next;
            }
            if (previous != 0 && previous->free) {
                previous->size += (uint32_t)sizeof(heap_block_t) + block->size;
                previous->next = block->next;
            }
            return;
        }
    }
}