
#include "sim900.h"
#include "timer_a0.h"
#include "sys_messagebus.h"

static void sim900_sm(enum sys_message msg)
{
    switch (sim900.cmd) {
        case CSIM900_ON:
            switch (sim900.next_state) {
                case SIM900_OFF:
                    LED_SWITCH;
                    sim900.next_state = SIM900_VBAT_ON;
                    timer_a0_delay_noblk(500000);
                break;
                case SIM900_VBAT_ON:
                    LED_SWITCH;
                    SIM900_VBAT_ENABLE;
                    SIM900_PWRKEY_HIGH;
                    SIM900_DTR_LOW;
                    sim900.next_state = SIM900_PWRKEY_ACT;
                    timer_a0_delay_noblk(500000);
                break;
                case SIM900_PWRKEY_ACT:
                    LED_SWITCH;
                    SIM900_PWRKEY_LOW;
                    sim900.next_state = SIM900_ON;
                    timer_a0_delay_noblk(1600000);
                break;
                case SIM900_ON:
                    LED_SWITCH;
                    SIM900_PWRKEY_HIGH;
                    sim900.next_state = SIM900_PRE_IDLE;
                    timer_a0_delay_noblk(1900000);
                break;
                case SIM900_PRE_IDLE:
                    LED_SWITCH;
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

// call only ONCE
void sim900_init(void)
{
    sim900.cmd = CSIM900_ON;
    sim900.next_state = SIM900_OFF;
    sys_messagebus_register(&sim900_sm, SYS_MSG_TIMER0_CRR2);
}


