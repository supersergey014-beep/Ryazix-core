#ifndef RYAZIX_SCHEDULER_H
#define RYAZIX_SCHEDULER_H

#include "idt.h"
#include "task.h"

void scheduler_init(void);
void scheduler_add_task(task_t *task);
task_t *scheduler_current_task(void);
registers_t *schedule(registers_t *current_regs);

#endif