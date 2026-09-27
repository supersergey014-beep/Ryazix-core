#ifndef RYAZIX_SYSCALL_H
#define RYAZIX_SYSCALL_H

#include <stdint.h>

#include "idt.h"

#define SYS_WRITE 0U
#define SYS_EXIT 1U

void syscall_init(void);
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2);
registers_t *syscall_handler(registers_t *regs);

#endif