
#include "sim900.h"
#include "timer_a0.h"
#include "uart1.h"
#include "sys_messagebus.h"

static void sim900_sm(enum sys_message msg)
{
    switch (sim900.cmd) {
        case CSIM900_ON:
            switch (sim900.next_state) {
                case SIM900_VBAT_ON:
                    LED_SWITCH; // on
                    SIM900_VBAT_ENABLE;
                    SIM900_PWRKEY_HIGH;
                    //SIM900_RTS_HIGH;
                    SIM900_DTR_LOW;
                    sim900.next_state = SIM900_PWRKEY_ACT;
                    //timer_a0_delay_noblk(16384); // 0.5s
                    timer_a0_delay_noblk(65000);
                break;
                case SIM900_PWRKEY_ACT:
                    LED_SWITCH; // off
                    SIM900_PWRKEY_LOW;
                    sim900.next_state = SIM900_ON;
                    timer_a0_delay_noblk(39321); // 1.2s
                break;
                case SIM900_ON:
                    LED_SWITCH; // on
                    SIM900_PWRKEY_HIGH;
                    sim900.next_state = SIM900_PRE_IDLE;
                    timer_a0_delay_noblk(62259); // 1.9s
                break;
                case SIM900_PRE_IDLE:
                    LED_SWITCH; // off
                    sim900.cmd = CSIM900_NULL;
                    sim900.next_state = SIM900_IDLE;
                break;
            }
        break;
        case CSIM900_OFF:
        break;
        case CSIM900_NULL:
        break;
    }
}

uint16_t sim900_tx_str(char *str, const uint16_t size)
{
    uint16_t p = 0;
    //SIM900_RTS_LOW;
    while (p < size) {
        while (!(SIM900_UCAIFG & UCTXIFG)) ;  // TX buffer ready?
        if (!(SIM900_CTS_IN)) {
            SIM900_UCATXBUF = str[p];
            p++;
        }
    }
    //SIM900_RTS_HIGH;
    return p;
}


// call only ONCE
void sim900_init(void)
{
    sim900.cmd = CSIM900_ON;
    sys_messagebus_register(&sim900_sm, SYS_MSG_TIMER0_CRR2);
    sim900.next_state = SIM900_VBAT_ON;
    timer_a0_delay_noblk(16384); // 0.5s

    /*
    // IRQ triggers on high-to-low edge
    P1IES |= SIM900_CTS;
    P1IFG &= ~SIM900_CTS;
    P1IE |= SIM900_CTS;
    */

}

void sim900_setup(void)
{
    uint8_t i;
    for (i=0;i<10;i++) {
        sim900_tx_str("AT\r", 4);
        timer_a0_delay(32000);
    }

    sim900_tx_str("AT+IPR=9600\r", 13);
    timer_a0_delay(7000);
    sim900_tx_str("ATE0\r", 13);
    timer_a0_delay(7000);
    sim900_tx_str("AT&W\r", 6);
}

/*
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    uint16_t iv = P1IV;

    if (iv == P1IV_P1IFG5) {
        if (P1IFG & SIM900_CTS) {
            P1IFG &= ~SIM900_CTS;
            if ((SIM900_CTS_IN) == 0) {
                SIM900_RTS_LOW;
            } else {
                SIM900_RTS_HIGH;
            }
        }
    }
}
*/

