#ifndef RYAZIX_TIMER_H
#define RYAZIX_TIMER_H

#include <stdint.h>

extern volatile uint32_t timer_ticks;

void timer_init(uint32_t frequency);

#endif