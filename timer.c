#include "timer.h"

#include "idt.h"
#include "io.h"
#include "pic.h"
#include "scheduler.h"

#define PIT_INPUT_FREQUENCY 1193182U
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_CHANNEL0 0x00
#define PIT_ACCESS_LO_HI 0x30
#define PIT_MODE_SQUARE_WAVE 0x06

volatile uint32_t timer_ticks;

static registers_t *timer_irq_handler(registers_t *regs)
{
    ++timer_ticks;
    return schedule(regs);
}

void timer_init(uint32_t frequency)
{
    uint32_t divisor;

    if (frequency == 0) {
        return;
    }

    divisor = PIT_INPUT_FREQUENCY / frequency;
    if (divisor == 0) {
        divisor = 1;
    } else if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }

    idt_register_irq(0);
    irq_register_handler(0, timer_irq_handler);

    outb(PIT_COMMAND_PORT,
         PIT_COMMAND_CHANNEL0 | PIT_ACCESS_LO_HI | PIT_MODE_SQUARE_WAVE);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    pic_unmask_irq(0);
}