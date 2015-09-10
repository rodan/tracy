
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
#include "drivers/fm24.h"
#include "drivers/fm24_memtest.h"
#include "qa.h"

#define GPSMAX 255

const char gps_init[] = "$PMTK314,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";

uint32_t rtca_set_next = 0;

uint32_t status_show_next = 0;

uint32_t adc_check_next = 0;
uint16_t adc_check_interval = 15;

uint32_t gps_reinit_next = 0;
uint16_t gps_reinit_interval = 120;

uint32_t movement_next = 0;

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
    parse_user_input();

    uart0_p = 0;
    uart0_rx_enable = 1;
    LED_OFF;
}
#else

static void schedule(enum sys_message msg)
{

    // battery related
    if (rtca_time.sys > adc_check_next) {
        adc_read();

        adc_check_next = rtca_time.sys + adc_check_interval;

        if ((stat.v_raw > 400) && (stat.v_raw < 550)) {
            if (CHARGING_STOPPED) {
                if (stat.should_charge) {
                    CHARGE_DISABLE;
                    stat.should_charge = false;
                    adc_check_next = rtca_time.sys + 300;
                } else {
                    if (stat.v_bat < 390) {
                        CHARGE_ENABLE;
                        stat.should_charge = true;
                        charge_start = rtca_time.sys;
                    }
                }
            } else {
                if (rtca_time.sys > charge_start + 36000) {
                    CHARGE_DISABLE;
                    stat.should_charge = false;
                    adc_check_next = rtca_time.sys + 3600;
                }
            }
        } else {
            CHARGE_DISABLE;
            stat.should_charge = false;
        }
    }

    // GPS related
    if (rtca_time.sys > gps_trigger_next) {

        switch (gps_next_state) {
            case MAIN_GPS_IDLE:
                if (s.gps_loop_interval > s.gps_warmup_interval + 30) {
                    // when gps has OFF intervals
                    gps_disable();
                    gps_trigger_next = rtca_time.sys + s.gps_loop_interval - s.gps_warmup_interval - 2;
                }
                gps_next_state = MAIN_GPS_START;
            break;
    
            case MAIN_GPS_START:
                gps_next_state = MAIN_GPS_INIT;
                gps_enable();
            break;

            case MAIN_GPS_INIT:
                if (s.gps_loop_interval > s.gps_warmup_interval + 30) {
                    // gps had a power off interval
                    gps_trigger_next = rtca_time.sys + s.gps_warmup_interval - s.gps_invalidate_interval - 3;
                    uart0_tx_str((char *)gps_init, 51);
                } else {
                    // gps was on all the time
                    gps_trigger_next = rtca_time.sys + s.gps_loop_interval - s.gps_invalidate_interval - 3;
                    if (rtca_time.sys > gps_reinit_next) {
                        gps_reinit_next = rtca_time.sys + gps_reinit_interval;
                        uart0_tx_str((char *)gps_init, 51);
                    }
                }
                gps_next_state = MAIN_GPS_PDOP_RST;
                // zero out all the old data
                memset(&mc_f, 0, sizeof(mc_f));
            break;
    
            case MAIN_GPS_PDOP_RST:
                gps_trigger_next = rtca_time.sys + s.gps_invalidate_interval - 1;
                gps_next_state = MAIN_GPS_STORE;
                mc_f.pdop = 9999;
            break;
    
            case MAIN_GPS_STORE:
                if (mc_f.fix) {
                    // save all info to f-ram
                    store_pkt();
                }
                gps_next_state = MAIN_GPS_IDLE;
            break;
        }
    }

    // GPRS related
    // force the HTTP POST from time to time
    if (rtca_time.sys > gprs_tx_next) {
        if (gprs_tx_trig & TG_NOW_MOVING) {
            gprs_tx_next = rtca_time.sys + s.gprs_moving_tx_interval;
        } else {
            gprs_tx_next = rtca_time.sys + s.gprs_static_tx_interval;
        }
        sim900.flags |= TX_FIX_RDY;
    }

    if (sim900.flags & BLACKOUT) {
       if (rtca_time.sys > gprs_blackout_lift) {
           sim900.flags &= ~BLACKOUT;
       }
    }

    if (((rtca_time.sys > gprs_trigger_next) || (sim900.flags & TX_FIX_RDY)) && 
            !(sim900.flags & TASK_IN_PROGRESS)) {

        // time to act
        adc_read();
        gprs_trigger_next = rtca_time.sys + s.gprs_loop_interval;

        if (stat.v_bat > 350) {
#ifndef DEBUG_GPS
            // if battery voltage is below ~3.4v
            // the sim will most likely lock up while trying to TX
            if (!(sim900.flags & BLACKOUT)) {
                sim900_exec_default_task();
            }
#endif
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

    settings_init(SEGMENT_B, VERSION_BASED);
    //settings_apply();

    m.e = 0x0;
    m.seg[0] = 0x0;
    m.seg_num = 1;

    stat.http_post_version = POST_VERSION;
    stat.fix_id = 1;

    sim900.imei[0] = 0;
    sim900.flags = 0;


    gps_trigger_next = 0;
    gprs_trigger_next = s.gprs_loop_interval;

    rtca_set_next = 0;
    rtc_not_set = 1;
    gps_next_state = MAIN_GPS_IDLE;

    if (s.gps_invalidate_interval > s.gps_loop_interval) {
        s.gps_invalidate_interval = s.gps_loop_interval;
    }
   
    gprs_tx_trig = 0;
    gprs_tx_next = s.gprs_static_tx_interval;

    gprs_blackout_lift = 0;

#ifdef DEBUG_GPS
    uart1_init(9600);
    uart1_tx_str("gps debug state\r\n", 17);
#endif

#ifdef DEBUG_GPRS
    uart0_tx_str("gprs debug state\r\n", 18);
    display_menu();
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

#ifdef FM24_HAS_SLEEP_MODE
    fm24_sleep();
#endif

    // main loop
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

#ifdef FM24_HAS_SLEEP_MODE
        // sleep
        if (fm24_status & FM24_AWAKE) {
            fm24_sleep();
        }
#endif

        // P4.0 and P4.1
        //P4SEL &= ~0x3;
        
        /*
        PMMCTL0_H = 0xA5;
        SVSMHCTL &= ~SVMHE;
        SVSMLCTL &= ~(SVSLE+SVMLE);
        PMMCTL0_H = 0x00;
        */
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

    //P4DIR = 0x00;
    P4DIR = 0x3;
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
#endif

    // try to also send uart0 tx to 4.1 XXX
    //P4MAP1 = PM_UCA0TXD;
    //P4SEL |= 0x2;

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

void settings_init(uint8_t * addr, const uint8_t location)
{
    uint8_t *src_p, *dst_p;
    uint8_t i;

    src_p = addr;
    dst_p = (uint8_t *) & s;
    if (((*src_p) != FLASH_VER) || (location)) {
        src_p = (uint8_t *) & defaults;
    }
    for (i = 0; i < sizeof(s); i++) {
        *dst_p++ = *src_p++;
    }
}

void settings_apply()
{
}

void adc_read()
{
    uint16_t q_bat = 0, q_raw = 0;
    uint32_t v_bat, v_raw;

    adc10_read(3, &q_bat, REFVSEL_1);
    adc10_read(2, &q_raw, REFVSEL_1);
    v_bat = (uint32_t) q_bat *s.vref * DIV_BAT / 10000;
    v_raw = (uint32_t) q_raw *s.vref * DIV_RAW / 10000;
    stat.v_bat = v_bat;
    stat.v_raw = v_raw;
    adc10_halt();
}

void store_pkt()
{
    uint32_t i;
    uint32_t me_temp;
    uint8_t suffix = 0xff;
    uint8_t payload_content_desc = 0;
    uint16_t rv = 0;

    me_temp = m.e;

    adc_read();

    if (!(s.settings & CONF_IGNORE_CELL_LOC)) {
        for (i = 0; i < 4; i++) {
            if ((sim900.cell[i].cellid != 65535) && (sim900.cell[i].cellid != 0)) {
                payload_content_desc = i + 1;
            }
        }
    }

    if (mc_f.fix) {
        payload_content_desc |= GPS_FIX_PRESENT;
#ifdef CONFIG_GEOFENCE
        payload_content_desc |= GEOFENCE_PRESENT;
        geofence_calc();
#endif
    }

#ifndef DEBUG_GPRS
    if (payload_content_desc == 0) {
        return;
    }
#endif

    rv += fm24_write((uint8_t *) & stat.http_post_version, m.e, 2);
    rv += fm24_write((uint8_t *) sim900.imei, m.e, 15);
    rv += fm24_write((uint8_t *) & s.settings, m.e, 2);
    rv += fm24_write((uint8_t *) & stat.v_bat, m.e, 2);
    rv += fm24_write((uint8_t *) & stat.v_raw, m.e, 2);
    rv += fm24_write((uint8_t *) & sim900.err, m.e, 2);
    rv += fm24_write((uint8_t *) & stat.fix_id, m.e, 2);
    rv += fm24_write((uint8_t *) & payload_content_desc, m.e, 1);

    if (mc_f.fix) {
        //fm24_write((uint8_t *) &mc_f, 25); // epic fail, struct not continguous
        rv += fm24_write((uint8_t *) & mc_f.year, m.e, 2);
        rv += fm24_write((uint8_t *) & mc_f.month, m.e, 1);
        rv += fm24_write((uint8_t *) & mc_f.day, m.e, 1);
        rv += fm24_write((uint8_t *) & mc_f.hour, m.e, 1);
        rv += fm24_write((uint8_t *) & mc_f.minute, m.e, 1);
        rv += fm24_write((uint8_t *) & mc_f.second, m.e, 1);

        rv += fm24_write((uint8_t *) & mc_f.lat, m.e, 4);
        rv += fm24_write((uint8_t *) & mc_f.lon, m.e, 4);
        rv += fm24_write((uint8_t *) & mc_f.pdop, m.e, 2);
        rv += fm24_write((uint8_t *) & mc_f.speed, m.e, 2);
        rv += fm24_write((uint8_t *) & mc_f.heading, m.e, 2);
        rv += fm24_write((uint8_t *) & mc_f.fixtime, m.e, 4);
#ifdef CONFIG_GEOFENCE
        rv += fm24_write((uint8_t *) & geo.distance, m.e, 4);
        rv += fm24_write((uint8_t *) & geo.bearing, m.e, 2);
#endif
    } else {
        // since gps is not available RTC time will be used
        rv += fm24_write((uint8_t *) & rtca_time.year, m.e, 2);
        rv += fm24_write((uint8_t *) & rtca_time.mon, m.e, 1);
        rv += fm24_write((uint8_t *) & rtca_time.day, m.e, 1);
        rv += fm24_write((uint8_t *) & rtca_time.hour, m.e, 1);
        rv += fm24_write((uint8_t *) & rtca_time.min, m.e, 1);
        rv += fm24_write((uint8_t *) & rtca_time.sec, m.e, 1);
    }

    if (payload_content_desc & 0x7) {
        // tower cell data
        rv += fm24_write((uint8_t *) & sim900.cell[0], m.e,
               (payload_content_desc & 0x7) * sizeof(sim900_cell_t));
        memset(&sim900.cell, 0, sizeof(sim900_cell_t));
    }

    // suffix
    rv += fm24_write((uint8_t *) & suffix, m.e, 1);

    if (rv < 31) {
        sim900.err |= ERR_RAM_WRITE;
    }

    stat.fix_id++;

    // reset values
    for (i = 0; i < 4; i++) {
        sim900.cell[i].cellid = 0;
    }
    mc_f.fix = 0;

    // organize data into < MAX_SEG_SIZE byte segments
    if (fm24_data_len(m.seg[m.seg_num - 1], m.e) > MAX_SEG_SIZE) {
        if (m.seg_num == MAX_SEG) {
            // drop oldest segment
            for (i = 0; i < MAX_SEG; i++) {
                m.seg[i] = m.seg[i+1];
            }
            m.seg_num--;
        }

        // create new segment
        m.seg[m.seg_num] = me_temp;
        m.seg[m.seg_num+1] = m.e;
        m.seg_num++;

    } else {
        // just concatenate to the current segment
        // and mark the end of the segment
        m.seg[m.seg_num] = m.e;
    }

    // see if a data upload is needed
#ifdef CONFIG_GEOFENCE
    if (geo.distance > s.geofence_trigger) {
        geo.distance = 0;
        movement_next = rtca_time.sys + 600;
        if (!(gprs_tx_trig & TG_NOW_MOVING)) {
            gprs_tx_trig |= TG_NOW_MOVING;
            sim900.flags |= TX_FIX_RDY;
            gprs_tx_next = rtca_time.sys + s.gprs_moving_tx_interval;
        }
    } else {
        if (rtca_time.sys > movement_next) {
            gprs_tx_trig &= ~TG_NOW_MOVING;
        }
    }
#endif

    // if only 2 more segments can be created
    if (m.seg_num > MAX_SEG - 3) {
        sim900.flags |= TX_FIX_RDY;
    }

#ifdef DEBUG_GPRS
    snprintf(str_temp, STR_LEN, "e: %lu\tseg_num: %d\twb: %u\r\n", m.e, m.seg_num, rv);
    uart0_tx_str(str_temp, strlen(str_temp));

    for (i=0;i < m.seg_num+1;i++) {
        snprintf(str_temp, STR_LEN, "seg[%lu]: %lu\r\n", i, m.seg[i]);
        uart0_tx_str(str_temp, strlen(str_temp));
    }
    uart0_tx_str("\r\n", 2);

    if (sim900.flags & TX_FIX_RDY) {
        uart0_tx_str("rdy\r\n", 5);
    }
#endif

}

void gps_enable(void)
{
    P6OUT |= BIT0;
    P4SEL |= 0xc;
}

void gps_disable(void)
{
    P6OUT &= ~BIT0;
    P4SEL &= ~0xc;
}

