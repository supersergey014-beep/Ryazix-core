#ifndef RYAZIX_TASK_H
#define RYAZIX_TASK_H

#include <stdint.h>

#include "idt.h"

#define TASK_KERNEL_STACK_SIZE (8U * 1024U)

typedef enum task_state_enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_ZOMBIE
} task_state_t;

typedef struct task_struct {
    uint32_t pid;
    task_state_t state;
    registers_t *regs;
    void *page_directory;
    void *kernel_stack;
    struct task_struct *next;
} task_t;

void task_init(void);
task_t *create_task(void (*entry_point)(void));
void task_exit_current(void);

#endif