#include <stdint.h>

#include "gdt.h"
#include "idt.h"
#include "initrd.h"
#include "keyboard.h"
#include "kheap.h"
#include "multiboot.h"
#include "pic.h"
#include "pmm.h"
#include "shell.h"
#include "scheduler.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"
#include "vga.h"
#include "vmm.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static void task_a(void)
{
    for (;;) {
        uint32_t start = timer_ticks;
        (void)syscall(SYS_WRITE, (uint32_t)"Task A running\n", 0);
        while (timer_ticks - start < 100U) {
            __asm__ volatile("hlt");
        }
    }
}

static void task_b(void)
{
    for (;;) {
        uint32_t start = timer_ticks;
        (void)syscall(SYS_WRITE, (uint32_t)"Task B running\n", 0);
        while (timer_ticks - start < 100U) {
            __asm__ volatile("hlt");
        }
    }
}

static volatile uint16_t *const vga_buffer = (volatile uint16_t *)0xB8000;
static uint8_t cursor_row;
static uint8_t cursor_column;

static void vga_scroll(void)
{
    uint32_t index;

    for (index = 0; index < (VGA_HEIGHT - 1) * VGA_WIDTH; ++index) {
        vga_buffer[index] = vga_buffer[index + VGA_WIDTH];
    }

    for (; index < VGA_HEIGHT * VGA_WIDTH; ++index) {
        vga_buffer[index] = (uint16_t)0x0720;
    }

    cursor_row = VGA_HEIGHT - 1;
}

void vga_clear(void)
{
    uint32_t index;

    for (index = 0; index < VGA_WIDTH * VGA_HEIGHT; ++index) {
        vga_buffer[index] = (uint16_t)0x0720;
    }

    cursor_row = 0;
    cursor_column = 0;
}

void vga_putc(char character, uint8_t color)
{
    if (character == '\n') {
        cursor_column = 0;
        ++cursor_row;
    } else if (character == '\b') {
        if (cursor_column != 0) {
            --cursor_column;
            vga_buffer[(uint32_t)cursor_row * VGA_WIDTH + cursor_column] =
                (uint16_t)0x0720;
        }
    } else {
        uint16_t entry = (uint16_t)character | ((uint16_t)color << 8);
        vga_buffer[(uint32_t)cursor_row * VGA_WIDTH + cursor_column] = entry;
        ++cursor_column;

        if (cursor_column == VGA_WIDTH) {
            cursor_column = 0;
            ++cursor_row;
        }
    }

    if (cursor_row == VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_puts(const char *str, uint8_t color)
{
    while (*str != '\0') {
        vga_putc(*str, color);
        ++str;
    }
}

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_address)
{
    const uint8_t color = 0x0F;
    uint32_t memory_size = 0;
    uint32_t initrd_location = 0;
    uint32_t initrd_size = 0;
    void *heap_test;

    if (multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC &&
        multiboot_info_address != 0) {
        const multiboot_info_t *multiboot_info =
            (const multiboot_info_t *)multiboot_info_address;

        if ((multiboot_info->flags & MULTIBOOT_INFO_MEMORY) != 0) {
            uint32_t upper_memory = multiboot_info->mem_upper;
            uint32_t max_upper_memory = UINT32_MAX / 1024U - 1024U;

            if (upper_memory > max_upper_memory) {
                memory_size = UINT32_MAX & ~(PMM_BLOCK_SIZE - 1U);
            } else {
                memory_size = (upper_memory + 1024U) * 1024U;
            }
        }
        if ((multiboot_info->flags & MULTIBOOT_INFO_MODS) != 0 &&
            multiboot_info->mods_count != 0 && multiboot_info->mods_addr != 0) {
            const multiboot_module_t *module =
                (const multiboot_module_t *)multiboot_info->mods_addr;
            if (module->mod_end >= module->mod_start) {
                initrd_location = module->mod_start;
                initrd_size = module->mod_end - module->mod_start;
            }
        }
    }

    vga_clear();
    gdt_init();
    idt_init();
    pic_remap();
    timer_init(100);
    keyboard_init();
    pmm_init(memory_size);
    vmm_init();
    kheap_init();
    vfs_init();
    if (initrd_location != 0 && initrd_size != 0) {
        (void)initialise_initrd_range(initrd_location, initrd_size);
    }
    task_init();
    syscall_init();
    create_task(task_a);
    create_task(task_b);
    vga_puts("==========================================\n", color);
    vga_puts("           Welcome to Ryazix OS           \n", color);
    vga_puts("==========================================\n", color);
    vga_puts("Phase 1: Boot sequence complete.", color);
    vga_puts("\nGDT and IDT initialized successfully.", color);
    vga_puts("\nHardware interrupts and devices initialized.", color);
    heap_test = kmalloc(64);
    if (heap_test != 0) {
        ((uint8_t *)heap_test)[0] = 0x5A;
        kfree(heap_test);
        vga_puts("\nMemory management and Paging active.", color);
    } else {
        vga_puts("\nKernel heap allocation failed.", 0x0C);
    }
    vga_puts("\nMultitasking and Syscall vector 0x80 active.", color);
    vga_puts("\nWelcome to the Ryazix shell.\n", color);

    __asm__ volatile("sti");
    shell_run();
}