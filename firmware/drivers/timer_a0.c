
//   timer a0 handling
//   CCR0 is currently unused
//   CCR1 is currently unused
//   CCR2 is currently unused
//   CCR3 is currently unused
//   CCR4 is used for timer_a0_delay()
//
//   author:          Petre Rodan <petre.rodan@simplex.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3


#include "timer_a0.h"

void timer_a0_init(void)
{
    __disable_interrupt();
    TA0CTL |= TASSEL__ACLK + MC__CONTINOUS;
    TA0R = 0;
    //TA0CCTL0 |= CCIE;
    __enable_interrupt();
}

// microseconds must be a value between 31 and 1999964
void timer_a0_delay(uint32_t microseconds)
{
    // one tick of ACLK is 1/32768 s
    /*
    if (microseconds < 31) {
        microseconds = 31;
    } else if (microseconds > 1999964) {
        microseconds = 1999964;
    }
    */

    uint32_t ticks = microseconds / 30.5175;
    __disable_interrupt();
    TA0CCR4 = TA0R + ticks;
    TA0CCTL4 = CCIE;
    __enable_interrupt();
    timer_a0_last_event &= ~TIMER_A0_EVENT_CCR4;
    while (1) {
        __no_operation();
#ifdef USE_WATCHDOG
        // reset watchdog
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        if (timer_a0_last_event & TIMER_A0_EVENT_CCR4)
            break;
    }
    __disable_interrupt();
    //TA0CCTL4 &= ~CCIE;
    TA0CCTL4 = 0;
    __enable_interrupt();
    timer_a0_last_event &= ~TIMER_A0_EVENT_CCR4;
}

__attribute__ ((interrupt(TIMER0_A1_VECTOR)))
void timer0_A1_ISR(void)
{
    uint16_t iv = TA0IV;
    // timer used by timer_a0_delay()
    if (iv == TA0IV_TA0CCR4) {
        timer_a0_last_event |= TIMER_A0_EVENT_CCR4;
        goto exit_lpm3;
    }
    return;
 exit_lpm3:
    /* exit from LPM3, give execution back to mainloop */
    _BIC_SR_IRQ(LPM3_bits);
}

