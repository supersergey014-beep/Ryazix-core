#include "task.h"

#include "kheap.h"
#include "scheduler.h"
#include "syscall.h"
#include "vmm.h"

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define TASK_INITIAL_EFLAGS 0x202

static uint32_t next_pid = 1;

static void task_return_trampoline(void)
{
    (void)syscall(SYS_EXIT, 0, 0);
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void task_init(void)
{
    next_pid = 1;
    scheduler_init();
}

task_t *create_task(void (*entry_point)(void))
{
    task_t *task;
    uint8_t *stack_top;
    uint32_t *return_address;

    if (entry_point == 0) {
        return 0;
    }

    task = (task_t *)kmalloc((uint32_t)sizeof(task_t));
    if (task == 0) {
        return 0;
    }

    task->kernel_stack = kmalloc(TASK_KERNEL_STACK_SIZE);
    if (task->kernel_stack == 0) {
        kfree(task);
        return 0;
    }

    stack_top = (uint8_t *)task->kernel_stack + TASK_KERNEL_STACK_SIZE;
    return_address = (uint32_t *)(stack_top - sizeof(uint32_t));
    *return_address = (uint32_t)task_return_trampoline;

    task->regs = (registers_t *)(stack_top - sizeof(uint32_t) -
                                 sizeof(registers_t));
    task->regs->ds = KERNEL_DATA_SELECTOR;
    task->regs->edi = 0;
    task->regs->esi = 0;
    task->regs->ebp = 0;
    task->regs->esp = 0;
    task->regs->ebx = 0;
    task->regs->edx = 0;
    task->regs->ecx = 0;
    task->regs->eax = 0;
    task->regs->int_no = 0;
    task->regs->error_code = 0;
    task->regs->eip = (uint32_t)entry_point;
    task->regs->cs = KERNEL_CODE_SELECTOR;
    task->regs->eflags = TASK_INITIAL_EFLAGS;
    task->pid = next_pid++;
    task->state = TASK_READY;
    task->page_directory = vmm_get_page_directory();

    scheduler_add_task(task);
    return task;
}

void task_exit_current(void)
{
    task_t *current = scheduler_current_task();

    if (current != 0 && current->pid != 0) {
        current->state = TASK_ZOMBIE;
    }
}