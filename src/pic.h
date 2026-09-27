#ifndef RYAZIX_PIC_H
#define RYAZIX_PIC_H

#include <stdint.h>

#define PIC_MASTER_OFFSET 0x20
#define PIC_SLAVE_OFFSET 0x28

void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

#endif