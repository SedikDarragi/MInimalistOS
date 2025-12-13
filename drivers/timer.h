#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "../include/idt.h"
#include "../kernel/io.h"

void timer_init(void);
void timer_wait(uint32_t ticks);
uint32_t timer_get_ticks(void);
void timer_interrupt_handler(struct regs* r);

#endif
