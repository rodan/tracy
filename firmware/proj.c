
//  sensor control board based on a MSP430F5510 uC
//   - compatible with hardware rev 03 -
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "calib.h"
#include "drivers/sys_messagebus.h"
#include "drivers/rtc.h"
#include "drivers/timer_a0.h"
#include "drivers/uart0.h"
#include "drivers/uart1.h"
#include "drivers/adc.h"
#include "drivers/nmea_parse.h"
#include "drivers/sim900.h"

#define GPSMAX 255

#define GPS_ENABLE          P6OUT |= BIT0
#define GPS_DISABLE         P6OUT &= ~BIT0
#define GPS_BKP_ENABLE      P4OUT |= BIT6
#define GPS_BKP_DISABLE     P4OUT &= ~BIT6

#define CHARGE_ENABLE       P6OUT &= ~BIT1
#define CHARGE_DISABLE      P6OUT |= BIT1

//const char gps_init[] = "$PMTK314,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2A\r\n";
const char gps_init[] = "$PMTK314,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";

const uint32_t rtca_set_period = 86400;
uint32_t rtca_set_next = 0;

const uint16_t gps_get_period = 600;
const uint8_t gps_fix_shtd = 5;
uint16_t gps_retry_period = 300;
uint32_t gps_get_next = 2;
uint8_t gps_fix_shtd_ctr = 0;
uint8_t gps_initialized = false;

#ifndef DEBUG_GPRS
static void parse_gps(enum sys_message msg)
{

    //uart1_tx_str((char *)uart0_rx_buf, uart0_p);
    //uart1_tx_str("\r\n", 2);

    if ((nmea_parse((char *)uart0_rx_buf, uart0_p) == EXIT_SUCCESS) && (mc_f.fix)) {
        gps_fix_shtd_ctr++;

        //snprintf(str_temp, STR_LEN, "%d %d.%04d%c %d %d.%04d%c  %lds\r\n",
        //         mc_f.lat_deg, mc_f.lat_min, mc_f.lat_fr, mc_f.lat_suffix,
        //         mc_f.lon_deg, mc_f.lon_min, mc_f.lon_fr, mc_f.lon_suffix,
        //         rtca_time.sys - mc_f.fixtime);
        //uart1_tx_str(str_temp, strlen(str_temp));

        if ((rtca_time.sys > rtca_set_next) || (rtca_time.min != mc_f.minute)) {
            rtca_time.year = mc_f.year;
            rtca_time.mon = mc_f.month;
            rtca_time.day = mc_f.day;
            rtca_time.hour = mc_f.hour;
            rtca_time.min = mc_f.minute;
            rtca_time.sec = mc_f.second;

            rtca_set_time();
            rtca_set_next += rtca_set_period;
        }
    }

    uart0_p = 0;
    uart0_rx_enable = 1;
}
#endif

static void parse_gprs(enum sys_message msg)
{
    /*
    uart0_tx_str((char *)uart1_rx_buf, uart1_p);
    uart0_tx_str("\r\n", 2);
    */

    uart1_p = 0;
    uart1_rx_enable = true;
    // signal that we are ready to receive more
    SIM900_RTS_LOW;
}

static void parse_UI(enum sys_message msg)
{
    int8_t f = uart0_rx_buf[0];

    if (f == 'S') {
        //sim900_setup();
    } else if (f == 'h') {
        SIM900_DTR_HIGH;
    } else if (f == 'w') {
        SIM900_DTR_LOW;
    }
    
    else {
        sim900_tx_str((char *)uart0_rx_buf, uart0_p);
        sim900_tx_str("\r", 1);
    }

    uart0_p = 0;
    uart0_rx_enable = 1;
}

static void schedule(enum sys_message msg)
{
    //P1OUT ^= BIT2;
    /*
    snprintf(str_temp, STR_LEN, "%04d%02d%02d %02d:%02d %ld\r\n",
             rtca_time.year, rtca_time.mon, rtca_time.day,
             rtca_time.hour, rtca_time.min, rtca_time.sys);
    uart0_tx_str(str_temp, strlen(str_temp));
    */

    /*
    snprintf(str_temp, STR_LEN, "cmd %d, state %d TA0CTL 0x%x, TA0CCTL2 0x%x, TA0CCR2 0x%x\r\n", sim900.cmd, sim900.next_state, TA0CTL, TA0CCTL2, TA0CCR2);
    uart0_tx_str(str_temp, strlen(str_temp));
    */

    if (rtca_time.sys > gps_get_next) {
        GPS_ENABLE;

        if ((rtca_time.sys > gps_get_next + 2) && (!gps_initialized)) {
            uart0_tx_str((char *)gps_init, 51);
            gps_initialized = true;
        }

        if ((rtca_time.sys > gps_get_next + gps_retry_period) || (gps_fix_shtd_ctr >= gps_fix_shtd)) {
            GPS_DISABLE;
            gps_initialized = false;
            gps_fix_shtd_ctr = 0;
            gps_get_next += gps_get_period;
        }
    }
    
}

int main(void)
{
    main_init();
    rtca_init();
    timer_a0_init();
    uart0_init();
    uart1_init();
    sim900_init();

    //GPS_BKP_ENABLE;
    CHARGE_ENABLE;

    //sys_messagebus_register(&parse_gps, SYS_MSG_UART0_RX);
    sys_messagebus_register(&parse_gprs, SYS_MSG_UART1_RX);
    sys_messagebus_register(&parse_UI, SYS_MSG_UART0_RX);
    sys_messagebus_register(&schedule, SYS_MSG_RTC_SECOND);

    uart0_tx_str("new\r\n", 5);

    while (1) {
        _BIS_SR(LPM3_bits + GIE);
        //wake_up();
#ifdef USE_WATCHDOG
        // reset watchdog counter
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        check_events();
    }
}

void main_init(void)
{

    // watchdog triggers after 4 minutes when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__8192K + WDTSSEL__ACLK + WDTCNTCL;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif
    //SetVCore(3);

    // enable LF crystal
    P5SEL |= BIT5 + BIT4;
    UCSCTL6 &= ~(XT1OFF | XT1DRIVE0);

    P1SEL = 0x0;
    P1DIR = 0xcd;
    P1REN = 0x2;
    P1OUT |= 0x2;

    P2SEL = 0x0;
    P2DIR = 0x1;

    P3SEL = 0x0;
    P3DIR = 0x1f;

    P4DIR = 0xc0;

    PMAPPWD = 0x02D52;
    // set up UART port mappings

    // there is only one debug uart so we either leave out the gps or the gprs module

#ifdef DEBUG_GPRS
    // debug interface
    P4MAP1 = PM_UCA0TXD;
    P4MAP0 = PM_UCA0RXD;
    P4SEL |= 0x3;
#else
    // GPS module
    P4MAP2 = PM_UCA0TXD;
    P4MAP3 = PM_UCA0RXD;
    P4SEL |= 0xc;
#endif

#ifdef DEBUG_GPS
    // debug interface
    P4MAP1 = PM_UCA1TXD;
    P4MAP0 = PM_UCA1RXD;
    P4SEL |= 0x3;
#else
    // GPRS module
    P4MAP4 = PM_UCA1TXD;
    P4MAP5 = PM_UCA1RXD;
    P4SEL |= 0x30;
#endif
    PMAPPWD = 0;

    //P5SEL is set above
    P5DIR = 0xf;
    P5OUT = 0x0;

    P6SEL = 0xc;
    P6DIR = 0x3;
    P6OUT = 0x2;

    PJDIR = 0xFF;
    PJOUT = 0x00;

    // disable VUSB LDO and SLDO
    USBKEYPID = 0x9628;
    USBPWRCTL &= ~(SLDOEN + VUSBEN);
    USBKEYPID = 0x9600;

}

/*
void wake_up(void)
{
    uint16_t timeout = 5000;
    UCSCTL6 &= ~XT2OFF;
    // wait until clocks settle
    do {
        //UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        UCSCTL7 &= ~(XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
        timeout--;
    } while ((SFRIFG1 & OFIFG) && timeout);
}
*/

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer0a
    if (timer_a0_last_event) {
        msg |= timer_a0_last_event;
        timer_a0_last_event = 0;
    }
    // drivers/rtca
    if (rtca_last_event & RTCA_EV_SECOND) {
        msg |= BIT5;
        rtca_last_event = 0;
    }
    // drivers/uart0
    if (uart0_last_event & UART0_EV_RX) {
        msg |= BITA;
        uart0_last_event = 0;
    }
    // drivers/uart1
    if (uart1_last_event & UART1_EV_RX) {
        msg |= BITB;
        uart1_last_event = 0;
    }
    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}
