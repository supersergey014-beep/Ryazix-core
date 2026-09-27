#include "pic.h"

#include "io.h"

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_COMMAND 0xA0
#define PIC_SLAVE_DATA 0xA1
#define PIC_ICW1_INIT 0x10
#define PIC_ICW1_ICW4 0x01
#define PIC_ICW4_8086 0x01
#define PIC_EOI 0x20

void pic_remap(void)
{
    uint8_t master_mask = inb(PIC_MASTER_DATA);
    uint8_t slave_mask = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();
    outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    outb(PIC_MASTER_DATA, PIC_MASTER_OFFSET);
    io_wait();
    outb(PIC_SLAVE_DATA, PIC_SLAVE_OFFSET);
    io_wait();

    outb(PIC_MASTER_DATA, 0x04);
    io_wait();
    outb(PIC_SLAVE_DATA, 0x02);
    io_wait();

    outb(PIC_MASTER_DATA, PIC_ICW4_8086);
    io_wait();
    outb(PIC_SLAVE_DATA, PIC_ICW4_8086);
    io_wait();

    (void)master_mask;
    (void)slave_mask;
    outb(PIC_MASTER_DATA, 0xFF);
    outb(PIC_SLAVE_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 16) {
        return;
    }

    if (irq >= 8) {
        outb(PIC_SLAVE_COMMAND, PIC_EOI);
    }
    outb(PIC_MASTER_COMMAND, PIC_EOI);
}

void pic_unmask_irq(uint8_t irq)
{
    uint16_t data_port;
    uint8_t mask;
    uint8_t line;

    if (irq >= 16) {
        return;
    }

    if (irq < 8) {
        data_port = PIC_MASTER_DATA;
        line = irq;
    } else {
        data_port = PIC_SLAVE_DATA;
        line = irq - 8;
        pic_unmask_irq(2);
    }

    mask = inb(data_port);
    outb(data_port, (uint8_t)(mask & (uint8_t)~(1U << line)));
}