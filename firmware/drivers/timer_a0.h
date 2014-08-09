#ifndef __TIMER_A0_H__
#define __TIMER_A0_H__

#include "proj.h"

//#define _10ms           41UL       // ~10ms
#define _10ms           5UL       // ~10ms
#define _500ms          _10ms * 50
#define _1200ms         _10ms * 120
//#define _1s             4096UL
#define _1s             512UL
#define _2s             _1s * 2
#define _3s             _1s * 3
#define _3sp            _3s + SM_STEP_DELAY
#define _5s             _1s * 5
#define _5sp            _5s + SM_STEP_DELAY
#define _6s             _1s * 6
#define _6sp            _6s + SM_STEP_DELAY
#define _10s            _1s * 10
#define _10sp           _10s + SM_STEP_DELAY
#define _14s            _1s * 14


enum timer_a0_event {
    TIMER_A0_EVENT_CCR0 = BIT0,
    TIMER_A0_EVENT_CCR1 = BIT1,
    TIMER_A0_EVENT_CCR2 = BIT2,
    TIMER_A0_EVENT_CCR3 = BIT3,
    TIMER_A0_EVENT_CCR4 = BIT4,
    TIMER_A0_EVENT_IFG = BIT5,
};

volatile enum timer_a0_event timer_a0_last_event;
volatile uint16_t timer_a0_ovf;

void timer_a0_init(void);
void timer_a0_halt(void);
void timer_a0_delay_noblk_ccr1(uint16_t ticks);
void timer_a0_delay_noblk_ccr2(uint16_t ticks);
void timer_a0_delay_noblk_ccr3(uint16_t ticks);
void timer_a0_delay_ccr4(uint16_t ticks);

#endif
