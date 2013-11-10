
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
#include "drivers/pmm.h"
#include "drivers/rtc.h"
#include "drivers/timer_a1.h"
#include "drivers/timer_a0.h"
#include "drivers/uart0.h"
#include "drivers/uart1.h"
#include "drivers/serial_bitbang.h"
#include "drivers/adc.h"

#define GPSMAX 255

char str_temp[64];

char gps_rx_buf[GPSMAX];
uint8_t gps_rx_buf_p = 0;

/*
static void do_smth(enum sys_message msg)
{
    snprintf(str_temp, 53,"%04d%02d%02d %02d:%02d\r\n",
            rtca_time.year, rtca_time.mon, rtca_time.day,
            rtca_time.hour, rtca_time.min
            );
    uart0_tx_str(str_temp, strlen(str_temp));
    uart1_tx_str(str_temp, strlen(str_temp));
}
*/

static void parse_gps(enum sys_message msg)
{
    /*
    for (i=0,i<uart0_p;i++) {
        if (uart0_rx_buf[i] == 0x0a)
            continue;
        if (uart0_rx_buf[i] == 0x0d) {
            uart0_tx_str(gps_rx_buf, gps_rx_buf_p);
            snprintf(str_temp, 10," _%03d_\r\n", gps_rx_buf_p);
            uart0_tx_str(str_temp, strlen(str_temp));
            gps_rx_buf_p = 0;
        } else {
            if (gps_rx_buf_p < GPSMAX-1) {
                gps_rx_buf[gps_rx_buf_p] = uart0_rx_buf;
                gps_rx_buf_p++;
            } else {
                gps_rx_buf_p = 0;
            }
        }
    }
    */
    uart0_tx_str(uart0_rx_buf, uart0_p);
    snprintf(str_temp, 10," _%03d_\r\n", uart0_p);
    uart0_tx_str(str_temp, strlen(str_temp));
    uart0_p = 0;
    uart0_rx_enable = 1;
}

int main(void)
{
    main_init();
    uart0_init();
    uart1_init();

    //sys_messagebus_register(&do_smth, SYS_MSG_RTC_SECOND);

    // parse GPS output
    sys_messagebus_register(&parse_gps, SYS_MSG_UART0_RX);

    while (1) {
        sleep();
        //__no_operation();
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
    SetVCore(3);

    // select XT1 and XT2 ports
    // select 12pF, enable both crystals
    P5SEL |= BIT5 + BIT4;
    
    // hf crystal
    /*
    uint16_t timeout = 5000;

    P5SEL |= BIT3 + BIT2;
    //UCSCTL6 |= XCAP0 | XCAP1;
    UCSCTL6 &= ~(XT1OFF + XT2OFF);
    UCSCTL3 = SELREF__XT2CLK;
    UCSCTL4 = SELA__XT1CLK | SELS__XT2CLK | SELM__XT2CLK;
    // wait until clocks settle
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
        timeout--;
    } while ((SFRIFG1 & OFIFG) && timeout);
    // decrease power
    //UCSCTL6 &= ~(XT2DRIVE0 + XT1DRIVE0);
    */
    UCSCTL6 &= ~(XT1OFF | XT1DRIVE0);

    P1SEL = 0x0;
    P1DIR = 0xff;
    //P1DIR = 0x00;
    P1OUT = 0x00;
    P1REN = 0x00;

    P2SEL = 0x0;
    P2DIR = 0x1;
    P2OUT = 0x0;

    P3SEL = 0x0;
    P3DIR = 0x1f;
    P3OUT = 0x0;

    P4SEL = 0x0;
    P4DIR = 0xff;
    P4REN = 0x0;
    P4OUT = 0x0;

    //P5SEL is set above
    P5DIR = 0x2;
    P5OUT = 0x0;

    P6SEL = 0x0;
    P6DIR = 0x2;
    P6OUT = 0x2;

    PJOUT = 0x00;
    PJDIR = 0xFF;

#ifdef CALIBRATION
    // send MCLK to P4.0
    __disable_interrupt();
    // get write-access to port mapping registers
    //PMAPPWD = 0x02D52;
    PMAPPWD = PMAPKEY;
    PMAPCTL = PMAPRECFG;
    // MCLK set out to 4.0
    P4MAP0 = PM_MCLK;
    //P4MAP0 = PM_RTCCLK;
    PMAPPWD = 0;
    __enable_interrupt();
    P4DIR |= BIT0;
    P4SEL |= BIT0;

    // send ACLK to P1.0
    P1DIR |= BIT0;
    P1SEL |= BIT0;
#endif

    // disable VUSB LDO and SLDO
    USBKEYPID = 0x9628;
    USBPWRCTL &= ~(SLDOEN + VUSBEN);
    USBKEYPID = 0x9600;

    rtca_init();
    timer_a0_init();
}

void sleep(void)
{
    //opt_power_disable();
    // turn off internal VREF, XT2, i2c power
    //UCSCTL6 |= XT2OFF;
    //PMMCTL0_H = 0xA5;
    //SVSMHCTL &= ~SVMHE;
    //SVSMLCTL &= ~(SVSLE+SVMLE);
    //PMMCTL0_H = 0x00;
    _BIS_SR(LPM3_bits + GIE);
    __no_operation();
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

    // drivers/rtca
    if (rtca_last_event) {
        msg |= rtca_last_event;
        rtca_last_event = 0;
    }
    // drivers/timer1a
    if (timer_a1_last_event) {
        msg |= timer_a1_last_event << 7;
        timer_a1_last_event = 0;
    }
    // drivers/uart0
    if (uart0_last_event == UART0_EV_RX) {
        msg |= BITA;
        uart0_last_event = 0;
    }
    // drivers/uart1
    if (uart1_last_event == UART1_EV_RX) {
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

void opt_power_enable()
{
    P1OUT &= ~BIT6;
}

void opt_power_disable()
{
    P1OUT |= BIT6;
    I2C_MASTER_DIR &= ~(I2C_MASTER_SCL + I2C_MASTER_SDA);
}

