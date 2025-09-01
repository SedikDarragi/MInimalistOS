#ifndef TIMER_H
#define TIMER_H

#include "../kernel/kernel.h"

void timer_init(void);
void timer_wait(uint32_t ticks);
uint32_t timer_get_ticks(void);

#endif
