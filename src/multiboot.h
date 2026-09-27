#ifndef RYAZIX_MULTIBOOT_H
#define RYAZIX_MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
#define MULTIBOOT_INFO_MEMORY 0x00000001
#define MULTIBOOT_INFO_MODS 0x00000008

typedef struct multiboot_info_struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
} __attribute__((packed)) multiboot_info_t;

typedef struct multiboot_module_struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} __attribute__((packed)) multiboot_module_t;

#endif