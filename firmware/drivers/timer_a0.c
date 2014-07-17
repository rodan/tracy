
//   timer a0 handling
//   CCR0 is currently unused
//   CCR1 is currently unused
//   CCR2 is used for timer_a0_delay_noblk_ccr2()
//   CCR3 is used for timer_a0_delay_noblk_ccr3()
//   CCR4 is currently unused
//
//   author:          Petre Rodan <petre.rodan@simplex.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3

#include "timer_a0.h"

void timer_a0_init(void)
{
    __disable_interrupt();
    timer_a0_ovf = 0;

    TA0CTL |= TASSEL__ACLK + MC__CONTINOUS + TACLR;// + TAIE;
    __enable_interrupt();
}


// ticks = microseconds / 30.5175
void timer_a0_delay_noblk_ccr2(uint16_t ticks)
{
    //uint32_t ticks = microseconds / 30.5175;
    TA0CCTL2 &= ~CCIE;
    TA0CCTL2 = 0;
    TA0CCR2 = TA0R + ticks;
    TA0CCTL2 = CCIE;
}

// ticks = microseconds / 30.5175
void timer_a0_delay_noblk_ccr3(uint16_t ticks)
{
    //uint32_t ticks = microseconds / 30.5175;
    TA0CCTL3 &= ~CCIE;
    TA0CCTL3 = 0;
    TA0CCR3 = TA0R + ticks;
    TA0CCTL3 = CCIE;
}

__attribute__ ((interrupt(TIMER0_A1_VECTOR)))
void timer0_A1_ISR(void)
{
    uint16_t iv = TA0IV;
    if (iv == TA0IV_TA0CCR4) {
        // timer used by timer_a0_delay()
        timer_a0_last_event |= TIMER_A0_EVENT_CCR4;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0CCR2) {
        // timer used by timer_a0_delay_noblk()
        // disable interrupt
        TA0CCTL2 &= ~CCIE;
        TA0CCTL2 = 0;
        timer_a0_last_event |= TIMER_A0_EVENT_CCR2;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0CCR3) {
        // timer used by timer_a0_delay_noblk_ccr3()
        // disable interrupt
        TA0CCTL3 &= ~CCIE;
        TA0CCTL3 = 0;
        timer_a0_last_event |= TIMER_A0_EVENT_CCR3;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0IFG) {
        TA0CTL &= ~TAIFG;
        timer_a0_ovf++;
        timer_a0_last_event |= TIMER_A0_EVENT_IFG;
        _BIC_SR_IRQ(LPM3_bits);
    }
}
