
//  GPS/GPRS tracking system based on a MSP430F5510 uC
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
#include "drivers/flash.h"
#include "drivers/adc.h"
#include "calib.h"

#define GPSMAX 255

#define GPS_ENABLE          P6OUT |= BIT0
#define GPS_DISABLE         P6OUT &= ~BIT0
#define GPS_BKP_ENABLE      P4OUT |= BIT6
#define GPS_BKP_DISABLE     P4OUT &= ~BIT6

#define CHARGE_ENABLE       P6OUT &= ~BIT1
#define CHARGE_DISABLE      P6OUT |= BIT1

const char gps_init[] = "$PMTK314,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2A\r\n";
//const char gps_init[] = "$PMTK314,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";

const uint32_t rtca_set_period = 86400;
uint32_t rtca_set_next = 0;

uint16_t fix_period = 600; // period between 2 fix reports sent via gprs
uint32_t fix_next = 2;           // fix timer   

uint8_t gps_fix_shtd = 5;  // after how many fixes should the the gps be turned off
uint16_t gps_retry_period = 300; // timeout period until a fix is expected
uint8_t gps_fix_shtd_ctr = 0;

uint8_t s_status = 0;       // schedule status

#ifndef DEBUG_GPRS
static void parse_gps(enum sys_message msg)
{

    if ((nmea_parse((char *)uart0_rx_buf, uart0_p) == EXIT_SUCCESS) && (mc_f.fix)) {
        gps_fix_shtd_ctr++;

#ifdef NEVER
        snprintf(str_temp, STR_LEN, "%d %d.%04d%c %d %d.%04d%c  %lds\r\n",
                 mc_f.lat_deg, mc_f.lat_min, mc_f.lat_fr, mc_f.lat_suffix,
                 mc_f.lon_deg, mc_f.lon_min, mc_f.lon_fr, mc_f.lon_suffix,
                 rtca_time.sys - mc_f.fixtime);
        uart1_tx_str(str_temp, strlen(str_temp));
#endif

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
    uart0_tx_str((char *)uart1_rx_buf, uart1_p);

    sim900_parse_rx((char *)uart1_rx_buf, uart1_p);
}

#ifdef DEBUG_GPRS
static void parse_UI(enum sys_message msg)
{
    char f = uart0_rx_buf[0];

    if (f == '?') {
        sim900_exec_default_task();
    } else if (f == '!') {
        sim900_start();
    } else if (f == ')') {
        sim900_halt();
    } else {
        sim900_tx_str((char *)uart0_rx_buf, uart0_p);
        sim900_tx_str("\r", 1);
    }

    uart0_p = 0;
    uart0_rx_enable = 1;
}
#else

static void schedule(enum sys_message msg)
{
    uint16_t q_bat = 0;
    //uint16_t q_raw = 0;

    //adc10_read(2, &q_raw, REFVSEL_1);

    // gps related
    if (rtca_time.sys > fix_next) {
        GPS_ENABLE;

        if ((rtca_time.sys > fix_next + 2) && ((s_status & GPS_INITIALIZED) == 0)) {
            uart0_tx_str((char *)gps_init, 51);
            s_status |= GPS_INITIALIZED;
            // invalidate last fix
            mc_f.fix = false;
        }

        if ((rtca_time.sys > fix_next + gps_retry_period) || (gps_fix_shtd_ctr >= gps_fix_shtd)) {
            GPS_DISABLE;
            s_status &= ~GPS_INITIALIZED;
            gps_fix_shtd_ctr = 0;
            fix_next += fix_period;
            //s_status |= SIM900_INITIALIZED;
            adc10_read(3, &q_bat, REFVSEL_1);
            if (q_bat > 700) {
                // 700 is the equivalent of ~3.4v
                sim900_exec_default_task();
            }
        }
    }
}
#endif

int main(void)
{
    main_init();
    rtca_init();
    timer_a0_init();
    uart0_init();
    sim900_init_messagebus();
    sim900.next_state = SIM900_OFF;

    GPS_BKP_ENABLE;
    CHARGE_ENABLE;

    settings_init(SEGMENT_B);

    sys_messagebus_register(&parse_gprs, SYS_MSG_UART1_RX);
#ifndef DEBUG_GPRS
    sys_messagebus_register(&parse_gps, SYS_MSG_UART0_RX);
    sys_messagebus_register(&schedule, SYS_MSG_RTC_SECOND);
#else
    sys_messagebus_register(&parse_UI, SYS_MSG_UART0_RX);
#endif

    while (1) {
        _BIS_SR(LPM3_bits + GIE);
        //wake_up();
#ifdef USE_WATCHDOG
        // reset watchdog counter
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        // new messages can be sent from within a check_events() call, so 
        // parse the message linked list multiple times
        check_events();
        check_events();
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
    P1DIR = 0x85;
    //P1REN = 0x2;
    // make sure CTS is pulled low so the software doesn't get stuck 
    // in case the sim900 is missing - or broken.
    P1REN = 0x22;
    P1OUT = 0x2;

    P2SEL = 0x0;
    P2DIR = 0x1;
    P2OUT = 0x0;

    P3SEL = 0x0;
    P3DIR = 0x1f;
    P3OUT = 0x0;

    P4DIR = 0xc0;
    P4OUT = 0x0;

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

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer0a
    if (timer_a0_last_event) {
        msg |= timer_a0_last_event;
        timer_a0_last_event = 0;
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
    // drivers/rtca
    if (rtca_last_event & RTCA_EV_SECOND) {
        msg |= BITF;
        rtca_last_event = 0;
    }
    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}

void settings_init(uint8_t * addr)
{
    uint8_t *src_p, *dst_p;
    uint8_t i;

    src_p = addr;
    dst_p = (uint8_t *) & s;
    if ((*src_p) != VERSION) {
        src_p = (uint8_t *) & defaults;
    }
    for (i = 0; i < sizeof(s); i++) {
        *dst_p++ = *src_p++;
    }
}

