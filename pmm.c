#include "pmm.h"

#define PMM_MAX_BLOCKS (1U << 20)
#define PMM_BITMAP_SIZE (PMM_MAX_BLOCKS / 8U)
#define PMM_RESERVED_LOW_MEMORY (1024U * 1024U)

extern uint8_t _kernel_end;

static uint8_t frame_bitmap[PMM_BITMAP_SIZE];
static uint32_t total_blocks;
static uint32_t used_blocks;
static uint32_t first_usable_block;

static uint8_t frame_is_used(uint32_t block)
{
    return (uint8_t)(frame_bitmap[block >> 3] & (uint8_t)(1U << (block & 7)));
}

static void set_frame_used(uint32_t block)
{
    frame_bitmap[block >> 3] |= (uint8_t)(1U << (block & 7));
}

static void set_frame_free(uint32_t block)
{
    frame_bitmap[block >> 3] &= (uint8_t)~(1U << (block & 7));
}

void pmm_init(uint32_t mem_size)
{
    uint32_t index;
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    uint32_t reserved_end = kernel_end;

    if (reserved_end < PMM_RESERVED_LOW_MEMORY) {
        reserved_end = PMM_RESERVED_LOW_MEMORY;
    }
    if (reserved_end > UINT32_MAX - (PMM_BLOCK_SIZE - 1U)) {
        first_usable_block = PMM_MAX_BLOCKS;
    } else {
        first_usable_block =
            (reserved_end + PMM_BLOCK_SIZE - 1U) / PMM_BLOCK_SIZE;
    }

    total_blocks = mem_size / PMM_BLOCK_SIZE;
    if (total_blocks > PMM_MAX_BLOCKS) {
        total_blocks = PMM_MAX_BLOCKS;
    }
    if (first_usable_block > total_blocks) {
        first_usable_block = total_blocks;
    }

    for (index = 0; index < PMM_BITMAP_SIZE; ++index) {
        frame_bitmap[index] = 0xFF;
    }
    used_blocks = total_blocks;

    for (index = first_usable_block; index < total_blocks; ++index) {
        set_frame_free(index);
        --used_blocks;
    }
}

void *pmm_alloc_block(void)
{
    uint32_t block;

    for (block = first_usable_block; block < total_blocks; ++block) {
        if (!frame_is_used(block)) {
            set_frame_used(block);
            ++used_blocks;
            return (void *)(block * PMM_BLOCK_SIZE);
        }
    }

    return 0;
}

void pmm_free_block(void *addr)
{
    uint32_t address = (uint32_t)addr;
    uint32_t block;

    if ((address & (PMM_BLOCK_SIZE - 1U)) != 0) {
        return;
    }

    block = address / PMM_BLOCK_SIZE;
    if (block < first_usable_block || block >= total_blocks ||
        !frame_is_used(block)) {
        return;
    }

    set_frame_free(block);
    --used_blocks;
}