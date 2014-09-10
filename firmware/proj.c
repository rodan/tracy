
//  GPS/GPRS tracking system based on a MSP430F5510 uC
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "drivers/sys_messagebus.h"
#include "drivers/rtc.h"
#include "drivers/timer_a0.h"
#include "drivers/uart0.h"
#include "drivers/uart1.h"
#include "drivers/adc.h"
#include "drivers/gps.h"
#include "drivers/sim900.h"
#include "drivers/flash.h"
#include "drivers/adc.h"

#define GPSMAX 255

const char gps_init[] = "$PMTK314,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";

uint32_t rtca_set_next = 0;

uint32_t trigger_next = 0;
uint16_t loop_period = 600; // period (in seconds) between 2 fix reports sent via gprs
uint16_t gps_warmup = 120;  // period (in seconds) when only the gps antenna is active


#ifndef DEBUG_GPRS
static void parse_gps(enum sys_message msg)
{
    nmea_parse((char *)uart0_rx_buf, uart0_p);
    
    uart0_p = 0;
    uart0_rx_enable = 1;
    LED_OFF;
}
#endif

#ifndef DEBUG_GPS
static void parse_gprs(enum sys_message msg)
{
#ifdef DEBUG_GPRS
    uart0_tx_str((char *)uart1_rx_buf, uart1_p);
#endif
    sim900_parse_rx((char *)uart1_rx_buf, uart1_p);
}
#endif

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
    LED_OFF;
}
#else

static void schedule(enum sys_message msg)
{
#ifndef DEBUG_GPS
    uint16_t q_bat = 0, q_raw = 0;
    uint32_t v_bat, v_raw;
#endif

    if (rtca_time.sys >= trigger_next) {
        // time to act
        switch (main_next_state) {
            case MAIN_IDLE:
                trigger_next += 2;
                main_next_state = MAIN_START_GPS;
            break;

            case MAIN_START_GPS:
                trigger_next += 1;
                main_next_state = MAIN_INIT_GPS;

                GPS_ENABLE;
            break;

            case MAIN_INIT_GPS:
                trigger_next += gps_warmup;
                main_next_state = MAIN_START_GPRS;

                uart0_tx_str((char *)gps_init, 51);
                // invalidate last fix
                memset(&mc_f, 0, sizeof(mc_f));
            break;

            case MAIN_START_GPRS:
                trigger_next += loop_period - gps_warmup;
                main_next_state = MAIN_IDLE;

#ifndef DEBUG_GPS
                adc10_read(3, &q_bat, REFVSEL_1);
                adc10_read(2, &q_raw, REFVSEL_1);
                v_bat = (uint32_t) q_bat * s.vref * DIV_BAT / 10000;
                v_raw = (uint32_t) q_raw * s.vref * DIV_RAW / 10000;
                stat.v_bat = v_bat;
                stat.v_raw = v_raw;

                if (stat.v_bat > 340) {
                    // if battery voltage is below ~3.4v
                    // the sim will most likely lock up while trying to TX
                    sim900_exec_default_task();
                }
#endif
            break;
        }
    }
}
#endif

#ifdef CALIBRATION
static void adc_calibration(enum sys_message msg)
{
    uint16_t q_bat = 0;
    uint16_t q_raw = 0;

    uint32_t v_bat, v_raw;

    adc10_read(3, &q_bat, REFVSEL_1);
    adc10_read(2, &q_raw, REFVSEL_1);

    v_bat = (uint32_t) q_bat * VREF_2_0_6_3 * DIV_BAT / 10000;
    v_raw = (uint32_t) q_raw * VREF_2_0_6_2 * DIV_RAW / 10000;

    stat.v_bat = v_bat;
    stat.v_raw = v_raw;

    snprintf(str_temp, STR_LEN, "bat %u\t%u raw %u\t%u\r\n", q_bat, stat.v_bat, q_raw, stat.v_raw);
    uart0_tx_str(str_temp, strlen(str_temp));
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
    settings_init(SEGMENT_B);

    if (s.settings & CONF_ENABLE_CHARGING) {
        CHARGE_ENABLE;
    } else {
        CHARGE_DISABLE;
    }

    rtca_set_next = 0;
    main_next_state = MAIN_IDLE;

#ifdef DEBUG_GPS
    uart1_init(9600);
    uart1_tx_str("gps debug state\r\n", 17);
#endif

#ifdef DEBUG_GPRS
    uart0_tx_str("gprs debug state\r\n", 18);
#endif

#ifdef CALIBRATION
    sys_messagebus_register(&adc_calibration, SYS_MSG_RTC_SECOND);
#else
    #ifndef DEBUG_GPRS
        sys_messagebus_register(&schedule, SYS_MSG_RTC_SECOND);
        sys_messagebus_register(&parse_gps, SYS_MSG_UART0_RX);
    #else
        sys_messagebus_register(&parse_UI, SYS_MSG_UART0_RX);
    #endif
    #ifndef DEBUG_GPS
        sys_messagebus_register(&parse_gprs, SYS_MSG_UART1_RX);
    #endif

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

    // there is only one debug uart on (P4.1, P4.0)
    // so we either leave out the gps (P4.2, P4.3) or the gprs (P4.4, P4.5) module

#ifdef DEBUG_GPRS
    // debug interface
    P4MAP1 = PM_UCA0TXD;
    P4MAP0 = PM_UCA0RXD;
    P4SEL |= 0x3;
#else
    // enable GPS module
    P4MAP2 = PM_UCA0TXD;
    P4MAP3 = PM_UCA0RXD;
    P4SEL |= 0xc;
#endif
#ifdef DEBUG_GPS
    // debug interface
    P4MAP1 = PM_UCA1TXD;
    P4MAP0 = PM_UCA1RXD;
    P4SEL |= 0x3;
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

