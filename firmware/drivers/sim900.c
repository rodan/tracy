
#include <string.h>

#include "sim900.h"
#include "timer_a0.h"
#include "uart1.h"
#include "sys_messagebus.h"

// XXX
#include <stdio.h>
#include "uart0.h"

uint8_t sm_c; // state machine internal counter
#define SM_DELAY 6500

static void sim900_state_machine(enum sys_message msg)
{
    switch (sim900.cmd) {

        // poweron sequence
        case CMD_ON:
            switch (sim900.next_state) {
                case SIM900_VBAT_ON:
                    LED_ON;
                    SIM900_VBAT_ENABLE;
                    SIM900_PWRKEY_HIGH;
                    SIM900_RTS_HIGH;
                    SIM900_DTR_LOW;
                    sim900.next_state = SIM900_PWRKEY_ACT;
                    timer_a0_delay_noblk_ccr2(16384); // 0.5s
                break;
                case SIM900_PWRKEY_ACT:
                    LED_OFF;
                    SIM900_PWRKEY_LOW;
                    sim900.next_state = SIM900_ON;
                    timer_a0_delay_noblk_ccr2(39321); // 1.2s
                break;
                case SIM900_ON:
                    LED_ON;
                    SIM900_PWRKEY_HIGH;
                    sim900.next_state = SIM900_PRE_IDLE;
                    timer_a0_delay_noblk_ccr2(62259); // 1.9s
                break;
                case SIM900_PRE_IDLE:
                    LED_OFF;
                    SIM900_RTS_LOW;
                    sim900.cmd = CMD_NULL;
                    sim900.next_state = SIM900_IDLE;
                break;
            }
        break;

        // initial setup of the sim900
        case CMD_FIRST_PWRON:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_AT;
                    sim900.rc = RC_NULL;
                    sm_c = 0;
                    timer_a0_delay_noblk_ccr2(32000); // ~1s
                break;
                case SIM900_AT:
                    if (sim900.rc == RC_OK) {
                        sim900_tx_cmd("AT+IPR=9600;+IFC=2,2;E0&W\r", 26);
                        sim900.next_state = SIM900_WAITREPLY;
                        timer_a0_delay_noblk_ccr2(SM_DELAY);
                    } else {
                        sm_c++;
                        sim900_tx_cmd("AT\r",3);
                    }
                    if (sm_c > 15) {
                        // something terribly wrong, stop sim900
                        sim900.cmd = CMD_OFF;
                    }
                    timer_a0_delay_noblk_ccr2(32000); // ~1s
                break;
                case SIM900_WAITREPLY:
                    if (sim900.rc == RC_OK) {
                        // use the wrong password, thus trigger a reset
                        WDTCTL = WDTHOLD; 
                        sim900.cmd = CMD_NULL;
                        sim900.next_state = SIM900_IDLE;
                    } else {
                        sim900.cmd = CMD_OFF;
                        timer_a0_delay_noblk_ccr2(SM_DELAY);
                    }
                break;
            }
        break;

        // poweroff of the sim900
        case CMD_OFF:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_VBAT_OFF;
                    sim900_tx_cmd("AT+CPOWD=1\r", 11);
                    timer_a0_delay_noblk_ccr2(32000); // ~1s
                break;
                case SIM900_VBAT_OFF:
                    sim900.next_state = SIM900_OFF;
                    sim900.rdy = 0;
                    sim900.checks = 0;
                    sim900.cmd = CMD_NULL;
                    P1DIR &= 0xb7; // make RTS, DTR inputs
                    P4SEL &= 0xcf; // make both gprs RX and TX inputs
                    P4DIR &= 0xcf;
                    SIM900_VBAT_DISABLE;
                    SIM900_PWRKEY_HIGH;
                break;
            }
        break;

    }
}

static void sim900_console_timing(enum sys_message msg)
{
    if (sim900.console == TTY_RX_WAIT) {
        // this point is reached REPLY_TMOUT ticks after a command was sent
        // it also means SIM900 failed to reply in that time period
        sim900.rc = RC_TMOUT;
        sim900.console = TTY_NULL;
    } else if (sim900.console == TTY_RX_PENDING) {
        // this point is reached RXBUF_TMOUT ticks after the first reply byte is received
        uart1_rx_enable = false;
        sim900.console = TTY_NULL;
        uart1_last_event |= UART1_EV_RX;
    }
}

uint16_t sim900_tx_str(char *str, const uint16_t size)
{
    uint16_t p = 0;
    while (p < size) {
        while (!(SIM900_UCAIFG & UCTXIFG)) ;  // TX buffer ready?
        if (!(SIM900_CTS_IN)) {
            SIM900_UCATXBUF = str[p];
            p++;
        }
    }
    return p;
}

uint8_t sim900_tx_cmd(char *str, const uint16_t size)
{
    uint16_t p = 0;

    if (sim900.console != TTY_NULL) {
        return EXIT_FAILURE;
    }

    sim900.cmd_type = CMD_SOLICITED;
    sim900.console = TTY_RX_WAIT;
    // set up timer that will end the wait for a reply
    timer_a0_delay_noblk_ccr3(REPLY_TMOUT);

    while (p < size) {
        while (!(SIM900_UCAIFG & UCTXIFG)) ;  // TX buffer ready?
        if (!(SIM900_CTS_IN)) { // hw flow control allows TX?
            SIM900_UCATXBUF = str[p];
            p++;
        }
    }

    return EXIT_SUCCESS;
}

uint8_t sim900_parse_rx(char *s, const uint16_t size)
{
    if (sim900.cmd_type == CMD_SOLICITED) {
        if (strstr(s, "OK")) {
            sim900.rc = RC_OK;
        } else if (strstr(s, "ERROR")) {
            sim900.rc = RC_ERROR;
        } else {
            // here be dragons
            sim900.rc = RC_NULL;
        }
    } else {
        // unsolicited messages
        if (strstr(s, "RDY")) {
            sim900.rdy = true;
        }
        sim900.rc = RC_NULL;
    }

    // XXX
    //snprintf(str_temp, STR_LEN, "prx %d %d\r\n", sim900.cmd_type, sim900.rc);
    //uart0_tx_str(str_temp, strlen(str_temp));

    // signal that we are ready to receive more
    SIM900_RTS_LOW;

    uart1_p = 0;
    uart1_rx_enable = true;  

    sim900.cmd_type = CMD_UNSOLICITED;
    return EXIT_SUCCESS;
}

void sim900_init(void)
{
    sim900.checks = 0;
    sim900.rdy = false;
    sim900.cmd_type = CMD_UNSOLICITED;
    sim900.cmd = CMD_ON;
    sim900.next_state = SIM900_VBAT_ON;

    PMAPPWD = 0x02D52;
    P4MAP4 = PM_UCA1TXD;
    P4MAP5 = PM_UCA1RXD;
    P4SEL |= 0x30;
    PMAPPWD = 0;
    P1DIR |= 0x48;
    uart1_init(9600);
    timer_a0_delay_noblk_ccr2(16384); // 0.5s
}

void sim900_init_messagebus(void)
{
    sys_messagebus_register(&sim900_state_machine, SYS_MSG_TIMER0_CRR2);
    sys_messagebus_register(&sim900_console_timing, SYS_MSG_TIMER0_CRR3);
}

void sim900_first_pwron(void)
{
    // 2400bps is better detected by sim900's autobauding
    uart1_init(2400);

    sim900.cmd = CMD_FIRST_PWRON;
    sim900.next_state = SIM900_IDLE;
    timer_a0_delay_noblk_ccr2(SM_DELAY);
}

void sim900_halt(void)
{
    sim900.cmd = CMD_OFF;
    timer_a0_delay_noblk_ccr2(SM_DELAY);
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

