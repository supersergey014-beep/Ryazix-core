#include "idt.h"

#include "pic.h"
#include "vga.h"

#define IDT_ENTRY_COUNT 256
#define ISR_COUNT 32
#define KERNEL_CODE_SELECTOR 0x08
#define IDT_INTERRUPT_GATE 0x8E

static idt_entry_struct idt_entries[IDT_ENTRY_COUNT];
static idt_ptr_struct idt_ptr;
static irq_handler_t irq_handlers[16];

#define DECLARE_ISR(number) extern void isr##number(void)
DECLARE_ISR(0);
DECLARE_ISR(1);
DECLARE_ISR(2);
DECLARE_ISR(3);
DECLARE_ISR(4);
DECLARE_ISR(5);
DECLARE_ISR(6);
DECLARE_ISR(7);
DECLARE_ISR(8);
DECLARE_ISR(9);
DECLARE_ISR(10);
DECLARE_ISR(11);
DECLARE_ISR(12);
DECLARE_ISR(13);
DECLARE_ISR(14);
DECLARE_ISR(15);
DECLARE_ISR(16);
DECLARE_ISR(17);
DECLARE_ISR(18);
DECLARE_ISR(19);
DECLARE_ISR(20);
DECLARE_ISR(21);
DECLARE_ISR(22);
DECLARE_ISR(23);
DECLARE_ISR(24);
DECLARE_ISR(25);
DECLARE_ISR(26);
DECLARE_ISR(27);
DECLARE_ISR(28);
DECLARE_ISR(29);
DECLARE_ISR(30);
DECLARE_ISR(31);
#undef DECLARE_ISR

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

extern void isr_default(void);

static void (*const exception_stubs[ISR_COUNT])(void) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

static void (*const irq_stubs[16])(void) = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
};

static const char *const exception_messages[ISR_COUNT] = {
    "Division By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved Exception",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved Exception",
    "Reserved Exception",
    "Reserved Exception",
    "Reserved Exception",
    "Reserved Exception",
    "Reserved Exception",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved Exception"
};

static void idt_set_gate(uint8_t index, uint32_t base, uint16_t selector,
                         uint8_t flags)
{
    idt_entries[index].base_low = (uint16_t)(base & 0xFFFF);
    idt_entries[index].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    idt_entries[index].selector = selector;
    idt_entries[index].always_zero = 0;
    idt_entries[index].flags = flags;
}

void idt_init(void)
{
    uint16_t index;
    uint32_t default_stub = (uint32_t)isr_default;

    idt_ptr.limit = (uint16_t)(sizeof(idt_entries) - 1);
    idt_ptr.base = (uint32_t)&idt_entries[0];

    for (index = 0; index < IDT_ENTRY_COUNT; ++index) {
        idt_set_gate((uint8_t)index, default_stub, KERNEL_CODE_SELECTOR,
                     IDT_INTERRUPT_GATE);
    }

    for (index = 0; index < ISR_COUNT; ++index) {
        idt_set_gate((uint8_t)index, (uint32_t)exception_stubs[index],
                     KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    }

    __asm__ volatile("lidt (%0)" : : "r"(&idt_ptr) : "memory");
}

void idt_register_irq(uint8_t irq)
{
    if (irq < 16) {
        idt_set_gate((uint8_t)(PIC_MASTER_OFFSET + irq),
                     (uint32_t)irq_stubs[irq], KERNEL_CODE_SELECTOR,
                     IDT_INTERRUPT_GATE);
    }
}

void idt_register_handler(uint8_t vector, void (*handler)(void), uint8_t flags)
{
    idt_set_gate(vector, (uint32_t)handler, KERNEL_CODE_SELECTOR, flags);
}

void irq_register_handler(uint8_t irq, irq_handler_t handler)
{
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

static void print_hex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    int shift;

    for (shift = 28; shift >= 0; shift -= 4) {
        vga_putc(digits[(value >> shift) & 0x0F], 0x0C);
    }
}

void isr_handler(registers_t regs)
{
    vga_puts("CPU exception: ", 0x0C);
    if (regs.int_no < ISR_COUNT) {
        vga_puts(exception_messages[regs.int_no], 0x0C);
    } else {
        vga_puts("Unhandled interrupt", 0x0C);
    }

    vga_puts("\nError code: 0x", 0x0C);
    print_hex(regs.error_code);
    vga_puts("\n", 0x0C);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

registers_t *irq_handler(registers_t *regs)
{
    uint32_t irq = regs->int_no - PIC_MASTER_OFFSET;

    if (irq < 16 && irq_handlers[irq] != 0) {
        regs = irq_handlers[irq](regs);
    }
    if (irq < 16) {
        pic_send_eoi((uint8_t)irq);
    }
    return regs;
}