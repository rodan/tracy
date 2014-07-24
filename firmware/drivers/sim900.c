
#include <string.h>

#include "sim900.h"
#include "timer_a0.h"
#include "uart1.h"
#include "sys_messagebus.h"

// XXX
#include <stdio.h>
#include "uart0.h"
#include "nmea_parse.h"
#include "rtc.h"

uint8_t sm_c; // state machine internal counter

char eom[2] = { 0x1a, 0x0 };

static void sim900_state_machine(enum sys_message msg)
{
    switch (sim900.cmd) {

        ///////////////////////////////////
        //
        // poweron sequence
        //

        case CMD_ON:
            switch (sim900.next_state) {
                case SIM900_VBAT_ON:
                    LED_ON;
                    SIM900_VBAT_ENABLE;
                    SIM900_PWRKEY_HIGH;
                    SIM900_RTS_HIGH;
                    SIM900_DTR_LOW;
                    sim900.next_state = SIM900_PWRKEY_ACT;
                    timer_a0_delay_noblk_ccr2(2048); // 0.5s
                break;
                case SIM900_PWRKEY_ACT:
                    LED_OFF;
                    SIM900_PWRKEY_LOW;
                    sim900.next_state = SIM900_ON;
                    timer_a0_delay_noblk_ccr2(4915); // 1.2s
                break;
                case SIM900_ON:
                    LED_ON;
                    SIM900_PWRKEY_HIGH;
                    sim900.next_state = SIM900_PRE_IDLE;
                    timer_a0_delay_noblk_ccr2(7782); // 1.9s
                break;
                case SIM900_PRE_IDLE:
                    LED_OFF;
                    SIM900_RTS_LOW;
                    sim900.cmd = CMD_NULL;
                    sim900.next_state = SIM900_IDLE;
                break;
            }
        break;

        ///////////////////////////////////
        //
        // get modem IMEI
        //

        case CMD_GET_IMEI:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_GET_IMEI;
                    sim900_tx_cmd("AT+GSN\r", 7, REPLY_TMOUT);
                    sim900.cmd_type = CMD_SOLICITED_GSN;
                    timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                break;
                case SIM900_GET_IMEI:
                    sim900.next_state = SIM900_IDLE;
                    sim900.cmd = CMD_NULL;
                break;
            }
        break;

        ///////////////////////////////////
        //
        // initial setup of the sim900
        //

        case CMD_FIRST_PWRON:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_AT;
                    sim900.rc = RC_NULL;
                    sm_c = 0;
                    timer_a0_delay_noblk_ccr2(4096); // ~1s
                break;
                case SIM900_AT:
                    if (sim900.rc == RC_OK) {
                        sim900_tx_cmd("AT+IPR=9600;+IFC=2,2;E0&W\r", 26, REPLY_TMOUT);
                        sim900.next_state = SIM900_WAITREPLY;
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    } else {
                        sm_c++;
                        sim900_tx_cmd("AT\r", 3, REPLY_TMOUT);
                    }
                    if (sm_c > 15) {
                        // something terribly wrong, stop sim900
                        sim900.cmd = CMD_OFF;
                    }
                    timer_a0_delay_noblk_ccr2(SM_R_DELAY);
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

        ///////////////////////////////////
        //
        // poweroff of the sim900
        //

        case CMD_OFF:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_VBAT_OFF;
                    sim900_tx_cmd("AT+CPOWD=1\r", 11, REPLY_TMOUT);
                    timer_a0_delay_noblk_ccr2(SM_R_DELAY);
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

        ///////////////////////////////////
        //
        // send location via GPRS
        //
        
        case CMD_SEND_GPRS:
            switch (sim900.next_state) {
                case SIM900_IP_INITIAL:
                    sim900.next_state = SIM900_IP_START;
                    sim900_tx_cmd("AT+CGDCONT=1,\"IP\",\"live.vodafone.com\";+CIPSTATUS\r", 49, REPLY_TMOUT);
                    timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                break;
                case SIM900_IP_START:
                    if (sim900.rc == RC_STATE_IP_INITIAL) {
                        sim900.next_state = SIM900_IP_GPRSACT;
                        sim900_tx_cmd("AT+CSTT=\"live.vodafone.com\",\"live\",\"vodafone\";+CIPSTATUS\r", 57, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
                case SIM900_IP_GPRSACT:
                    if (sim900.rc == RC_STATE_IP_START) {
                        sim900.next_state = SIM900_IP_STATUS;
                        sim900_tx_cmd("AT+CIICR;+CIPSTATUS\r", 20, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
                case SIM900_IP_STATUS:
                    if (sim900.rc == RC_STATE_IP_GPRSACT) {
                        sim900.next_state = SIM900_IP_CONNECT;
                        sim900_tx_cmd("AT+CIFSR;+CIPHEAD=1;+CIPSTATUS\r", 31, 12288); // ~3s
                        timer_a0_delay_noblk_ccr2(12388);
                    }
                break;
                case SIM900_IP_CONNECT:
                    if (sim900.rc == RC_STATE_IP_STATUS) {
                        sim900.next_state = SIM900_IP_CONNECT_OK;
                        sim900_tx_cmd("AT+CIPSTART=\"TCP\",\"www.simplex.ro\",\"80\"\r", 40, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
                case SIM900_IP_CONNECT_OK:
                    if (sim900.rc == RC_OK) {
                        // CIPSTART sends a quick OK and then a CONNECT OK a couple seconds later
                        sim900.next_state = SIM900_IP_SEND;
                        
                        sim900.cmd_type = CMD_SOLICITED;
                        sim900.rc = RC_NULL;
                        sim900.console = TTY_RX_WAIT;
                        timer_a0_delay_noblk_ccr3(12288); // ~3s
                        timer_a0_delay_noblk_ccr2(12388); // ~>3s
                    }
                break;
                case SIM900_IP_SEND:
                    if (sim900.rc == RC_STATE_IP_CONNECT) {
                        sim900.next_state = SIM900_IP_PUT;
                        sim900_tx_cmd("AT+CIPSEND\r", 11, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
                case SIM900_IP_PUT:
                    if (sim900.rc == RC_TEXT_INPUT) {
                        sim900.next_state = SIM900_IP_CLOSE;
                        sim900_tx_str("GET /scripts/t?i=", 17);
                        sim900_tx_str(sim900.imei, 15);
                        sim900_tx_str("&l=", 3);
                        if (mc_f.fix) {
                            snprintf(str_temp, STR_LEN, "%d %d.%04d%c %d %d.%04d%c&f=%ld",
                            mc_f.lat_deg, mc_f.lat_min, mc_f.lat_fr, mc_f.lat_suffix,
                            mc_f.lon_deg, mc_f.lon_min, mc_f.lon_fr, mc_f.lon_suffix,
                            rtca_time.sys - mc_f.fixtime);
                            sim900_tx_str(str_temp, strlen(str_temp));
                        } else {
                            sim900_tx_str("no_fix", 6);
                        }
                        sim900_tx_str(" HTTP/1.1\r\n\r\n", 13);
                        sim900_tx_cmd(eom, 2, 20480);
                        timer_a0_delay_noblk_ccr2(20580);
                    }
                break;
                case SIM900_IP_CLOSE:
                    if (sim900.rc == RC_OK) { // XXX
                        sim900.next_state = SIM900_IP_SHUT;
                        sim900_tx_cmd("AT+CIPCLOSE\r", 12, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
                case SIM900_IP_SHUT:
                    if (sim900.rc == RC_OK) { // XXX
                        sim900.next_state = SIM900_IDLE;
                        sim900_tx_cmd("AT+CIPSHUT\r", 11, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    }
                break;
            }
        break;

        ///////////////////////////////////
        //
        // send SMS
        //

        case CMD_SEND_SMS:
            switch (sim900.next_state) {
                case SIM900_IDLE:
                    sim900.next_state = SIM900_SET1;
                    sim900_tx_cmd("AT+CMGF=1\r", 10, REPLY_TMOUT);
                    timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                break;
                case SIM900_SET1:
                    if (sim900.rc == RC_OK) {
                        //snprintf(str_temp, STR_LEN, "AT+CMGS=\"%s\"\r", s.phone_num);
                        //sim900_tx_cmd(str_temp, strlen(str_temp));
                        sim900.next_state = SIM900_TEXT_INPUT;
                        sim900_tx_str("AT+CMGS=\"", 9);
                        sim900_tx_str(s.phone_num, 12);
                        sim900_tx_cmd("\"\r", 2, REPLY_TMOUT);
                        timer_a0_delay_noblk_ccr2(SM_R_DELAY);
                    } else {
                        // XXX
                        sim900.next_state = SIM900_IDLE;
                        sim900.cmd = CMD_NULL;
                    }
                break;
                case SIM900_TEXT_INPUT:
                    if (sim900.rc == RC_TEXT_INPUT) {
                        if (mc_f.fix) {
                            snprintf(str_temp, STR_LEN, "%d %d.%04d%c %d %d.%04d%c  %lds%s\r",
                            mc_f.lat_deg, mc_f.lat_min, mc_f.lat_fr, mc_f.lat_suffix,
                            mc_f.lon_deg, mc_f.lon_min, mc_f.lon_fr, mc_f.lon_suffix,
                            rtca_time.sys - mc_f.fixtime, eom);
                            sim900_tx_cmd(str_temp, strlen(str_temp), 20480); // wait max ~5s for reply
                        } else {
                            sim900_tx_str("no fix", 6);
                            sim900_tx_cmd(eom, 2, 20480);
                        }
                        sim900.next_state = SIM900_TEXT_RCVD;
                        timer_a0_delay_noblk_ccr2(24576); // ~6s
                    } else {
                        // XXX
                        sim900.next_state = SIM900_IDLE;
                        sim900.cmd = CMD_NULL;
                    }
                break;
                case SIM900_TEXT_RCVD:
                    if (sim900.rc == RC_CMGS) {
                        sim900.next_state = SIM900_IDLE;
                        sim900.cmd = CMD_NULL;
                        // XXX
                        uart0_tx_str("y\r\n", 3);
                    } else {
                        // XXX
                        sim900.next_state = SIM900_IDLE;
                        sim900.cmd = CMD_NULL;
                    }
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
        SIM900_RTS_LOW;
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

uint8_t sim900_tx_cmd(char *str, const uint16_t size, const uint16_t reply_tmout)
{
    uint16_t p = 0;

    if (sim900.console != TTY_NULL) {
        return EXIT_FAILURE;
    }

    sim900.cmd_type = CMD_SOLICITED;
    sim900.rc = RC_NULL;
    sim900.console = TTY_RX_WAIT;
    // set up timer that will end the wait for a reply
    timer_a0_delay_noblk_ccr3(reply_tmout);

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
    uint8_t i;

    s[size] = 0;

    if (sim900.cmd_type == CMD_SOLICITED) {
        if (strstr(s, "+CMGS:")) {
            // '\r\n+CMGS: 5\r\n\r\nOK\r\n'
            // we want to catch the +CMGS part, so parse before 'else if == "OK"'
            sim900.rc = RC_CMGS;
        } else if (strstr(s, "INITIAL")) {
            // '\r\nOK\r\n\r\nSTATE: IP INITIAL\r\n'
            sim900.rc = RC_STATE_IP_INITIAL;
        } else if (strstr(s, "START")) {
            // '\r\nOK\r\n\r\nSTATE: IP START\r\n'
            sim900.rc = RC_STATE_IP_START;
        } else if (strstr(s, "GPRSACT")) {
            // '\r\nOK\r\n\r\nOK\r\n\r\nSTATE: IP GPRSACT\r\n
            sim900.rc = RC_STATE_IP_GPRSACT;
        } else if (strstr(s, "STATUS")) {
            // '\r\nAAA.BBB.CCC.DDD\r\n\r\nOK\r\nSTATE: IP STATUS\r\n
            sim900.rc = RC_STATE_IP_STATUS;
        } else if (strstr(s, "CONNECT")) {
            // CONNECT OK
            sim900.rc = RC_STATE_IP_CONNECT;
        } else if (strstr(s, "SHUT")) {
            // SHUT OK
            sim900.rc = RC_STATE_IP_SHUT;
        } else if (strstr(s, "OK")) {
            // '\r\nOK\r\n'
            sim900.rc = RC_OK;
        } else if (strstr(s, "ERROR")) {
            sim900.rc = RC_ERROR;
        } else if (strstr(s, "> ")) {
            // '\r\n> '
            sim900.rc = RC_TEXT_INPUT;
        } else {
            // unknown solicited? reply
            sim900.rc = RC_NULL;
        }

        // shorten the state machine delay
        timer_a0_delay_noblk_ccr2(81); // ~20ms
    } else if ((sim900.cmd_type == CMD_SOLICITED_GSN) && (size == 25)) {
        for (i=2; i<17; i++) {
            sim900.imei[i-2] = s[i];
        }
    } else {
        // unsolicited messages
        if (strstr(s, "RDY")) {
            // '\r\nRDY\r\n'
            sim900.rdy |= RDY;
        } else if (strstr(s, "Call Ready")) {
            // '\r\nCall Ready\r\n'
            sim900.rdy |= CALL_RDY;
        }
        sim900.rc = RC_NULL;
    }

    //memset(s, 0, UART1_RXBUF_SZ);

    // XXX
    snprintf(str_temp, STR_LEN, "prx %d %d\r\n", sim900.cmd_type, sim900.rc);
    uart0_tx_str(str_temp, strlen(str_temp));

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
    sim900.rdy = 0;
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
    timer_a0_delay_noblk_ccr2(2048); // 0.5s
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

void sim900_get_imei(void)
{
    sim900.cmd = CMD_GET_IMEI;
    timer_a0_delay_noblk_ccr2(SM_DELAY);
}

void sim900_send_fix_sms(void)
{
    sim900.cmd = CMD_SEND_SMS;
    timer_a0_delay_noblk_ccr2(SM_DELAY);
}

void sim900_send_fix_gprs(void)
{
    sim900.cmd = CMD_SEND_GPRS;
    sim900.next_state = SIM900_IP_INITIAL;
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

