
//   timer a1 handling
//   CCR0 is used in decoding infrared remote control sequences
//   CCR1 is currently unused
//   CCR2 is currently unused
//
//   author:          Petre Rodan <petre.rodan@simplex.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3

#include "timer_a1.h"
#include "ir_remote.h"
#include "proj.h"

void timer_a1_init(void)
{
    __disable_interrupt();
    TA1CTL |= TASSEL__SMCLK + MC__UP;
    TA1CCR0 = IR_CCR;
    TA1R = 0;
    TA1CCTL0 |= CCIE;           // enable interrupt
    __enable_interrupt();
}

void timer_a1_halt(void)
{
    TA1CTL = 0;
    TA1CTL &= ~TAIE;
    TA1CCTL0 &= ~CCIE;          // disable interrupt
}

__attribute__ ((interrupt(TIMER1_A0_VECTOR)))
void timer1_A0_ISR(void)
{
    if (TA1CCR0 == IR_CCR) {
        timer_a1_last_event |= TIMER1_EVENT_CCR0;
        goto exit_lpm3;
    }
    return;

 exit_lpm3:
    // exit from LPM3, give execution back to mainloop
    _BIC_SR_IRQ(LPM3_bits);
}

/*
__attribute__ ((interrupt(TIMER1_A1_VECTOR)))
void timer1_A1_ISR(void)
{
    uint16_t iv = TA1IV;
    if (iv == TA1IV_TA1CCR1) {
        timer_a1_last_event |= TIMER1_EVENT_CCR1;
    } else if (iv == TA1IV_TA1CCR2) {
        timer_a1_last_event |= TIMER1_EVENT_CCR2;
    }
}
*/
