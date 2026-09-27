#ifndef RYAZIX_IDT_H
#define RYAZIX_IDT_H

#include <stdint.h>

typedef struct idt_entry_struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always_zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_struct;

typedef struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_struct;

typedef struct registers_struct {
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} registers_t;

typedef registers_t *(*irq_handler_t)(registers_t *regs);

void idt_init(void);
void idt_register_irq(uint8_t irq);
void idt_register_handler(uint8_t vector, void (*handler)(void), uint8_t flags);
void irq_register_handler(uint8_t irq, irq_handler_t handler);
void isr_handler(registers_t regs);
registers_t *irq_handler(registers_t *regs);

#endif