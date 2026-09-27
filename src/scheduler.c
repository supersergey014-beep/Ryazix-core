#include "scheduler.h"

#include "vmm.h"

static task_t bootstrap_task;
static task_t *task_list;
static task_t *current_task;

void scheduler_init(void)
{
    bootstrap_task.pid = 0;
    bootstrap_task.state = TASK_RUNNING;
    bootstrap_task.regs = 0;
    bootstrap_task.page_directory = vmm_get_page_directory();
    bootstrap_task.kernel_stack = 0;
    bootstrap_task.next = &bootstrap_task;
    task_list = &bootstrap_task;
    current_task = &bootstrap_task;
}

void scheduler_add_task(task_t *task)
{
    task_t *tail;

    if (task == 0) {
        return;
    }
    if (task_list == 0) {
        scheduler_init();
    }

    tail = task_list;
    while (tail->next != task_list) {
        tail = tail->next;
    }
    task->next = task_list;
    tail->next = task;
}

task_t *scheduler_current_task(void)
{
    return current_task;
}

registers_t *schedule(registers_t *current_regs)
{
    task_t *candidate;
    task_t *start;

    if (current_task == 0 || task_list == 0) {
        return current_regs;
    }

    current_task->regs = current_regs;
    if (current_task->state != TASK_ZOMBIE) {
        current_task->state = TASK_READY;
    }

    start = current_task->next;
    candidate = start;
    do {
        if (candidate->state == TASK_READY && candidate->regs != 0) {
            current_task = candidate;
            current_task->state = TASK_RUNNING;
            return current_task->regs;
        }
        candidate = candidate->next;
    } while (candidate != start);

    if (current_task->state != TASK_ZOMBIE) {
        current_task->state = TASK_RUNNING;
        return current_regs;
    }

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}