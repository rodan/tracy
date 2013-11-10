#ifndef __TIMER_A0_H__
#define __TIMER_A0_H__

#include "proj.h"

void timer_a0_init(void);
void timer_a0_halt(void);

enum timer_a0_event {
    TIMER_A0_EVENT_CCR0 = BIT0,
    TIMER_A0_EVENT_CCR1 = BIT1,
    TIMER_A0_EVENT_CCR2 = BIT2,
    TIMER_A0_EVENT_CCR3 = BIT3,
    TIMER_A0_EVENT_CCR4 = BIT4,
};

volatile enum timer_a0_event timer_a0_last_event;

void timer_a0_delay(uint32_t microseconds);




#endif
