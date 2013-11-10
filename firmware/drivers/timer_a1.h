#ifndef __TIMER_A1_H__
#define __TIMER_A1_H__

#include "proj.h"

void timer_a1_init(void);
void timer_a1_halt(void);

enum timer_a1_event {
    TIMER1_EVENT_CCR0 = BIT0,
    TIMER1_EVENT_CCR1 = BIT1,
    TIMER1_EVENT_CCR2 = BIT2
};

volatile enum timer_a1_event timer_a1_last_event;

#endif
