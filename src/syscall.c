#include "syscall.h"

#include "scheduler.h"
#include "task.h"
#include "vga.h"

#define IDT_USER_INTERRUPT_GATE 0xEE

extern void syscall_stub(void);

void syscall_init(void)
{
    idt_register_handler(0x80, syscall_stub, IDT_USER_INTERRUPT_GATE);
}

uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2)
{
    uint32_t result;

    __asm__ volatile("int $0x80"
                     : "=a"(result)
                     : "a"(num), "b"(arg1), "c"(arg2)
                     : "memory", "cc");
    return result;
}

registers_t *syscall_handler(registers_t *regs)
{
    switch (regs->eax) {
    case SYS_WRITE:
        if (regs->ebx != 0) {
            vga_puts((const char *)regs->ebx, 0x0F);
            regs->eax = 0;
        } else {
            regs->eax = (uint32_t)-1;
        }
        return regs;
    case SYS_EXIT:
        task_exit_current();
        return schedule(regs);
    default:
        regs->eax = (uint32_t)-1;
        return regs;
    }
}