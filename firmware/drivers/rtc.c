
/*
    rtca.c: TI CC430 Hardware Realtime Clock (RTC_A)

    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>

				http://www.openchronos-ng.sourceforge.net

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rtc.h"
#include "rtca_now.h"

#ifdef CONFIG_RTC_DST
#include "rtc_dst.h"
#endif

void rtca_init(void)
{

    rtca_time.year = COMPILE_YEAR;
    rtca_time.mon = COMPILE_MON;
    rtca_time.day = COMPILE_DAY;
    rtca_time.dow = COMPILE_DOW;
    rtca_time.hour = COMPILE_HOUR;
    rtca_time.min = COMPILE_MIN;
    rtca_time.sec = 59;

    /* Enable calendar mode (date/time registers are automatically reset)
       and enable read ready interrupts
       and set time event interrupts at each minute
       also enable alarm interrupts */
    RTCCTL01 |= RTCMODE | RTCRDYIE | RTCAIE | RTCTEVIE;

    rtca_set_time();

#ifdef CONFIG_RTC_DST
    /* initialize DST module */
    rtc_dst_init();
#endif

}

void rtca_set_time()
{
    /* Stop RTC timekeeping for a while */
    rtca_stop();

    /* update RTC registers */
    RTCSEC = rtca_time.sec;
    RTCMIN = rtca_time.min;
    RTCHOUR = rtca_time.hour;
    RTCDAY = rtca_time.day;
    RTCDOW = rtca_time.dow;
    RTCMON = rtca_time.mon;
    RTCYEARL = rtca_time.year & 0xff;
    RTCYEARH = rtca_time.year >> 8;

    /* Resume RTC time keeping */
    rtca_start();
}

/*
void rtca_get_alarm(uint8_t *hour, uint8_t *min)
{
	*hour = RTCAHOUR & 0x1F;
	*min  = RTCAMIN  & 0x3F;
}

void rtca_set_alarm(uint8_t hour, uint8_t min)
{
	RTCAHOUR = (RTCAHOUR & 0x80) | hour;
	RTCAMIN  = (RTCAMIN & 0x80) | min;
}

void rtca_enable_alarm()
{
	RTCCTL01 &= ~RTCAIE;
	RTCAHOUR |= 0x80;
	RTCAMIN  |= 0x80;
	RTCCTL01 |= RTCAIE;
}

void rtca_disable_alarm()
{
	RTCAHOUR &= 0x7F;
	RTCAMIN  &= 0x7F;
	RTCCTL01 &= ~RTCAIE;
}
*/

__attribute__ ((interrupt(RTC_VECTOR)))
void RTC_A_ISR(void)
{
    /* the IV is cleared after a read, so we store it */
    uint16_t iv = RTCIV;

    /* copy register values */
    rtca_time.sec = RTCSEC;

    /* count system time */
    rtca_time.sys++;

    enum rtca_tevent ev = 0;

    /* second event (from the read ready interrupt flag) */
    if (iv == RTCIV_RTCRDYIFG) {
        ev = RTCA_EV_SECOND;
        goto finish;
    }

    if (iv == RTCIV_RTCAIFG) {
        ev = RTCA_EV_ALARM;
        goto finish;
    }

    {
        if (iv != RTCIV_RTCTEVIFG)      /* Minute changed! */
            goto finish;

        ev |= RTCA_EV_MINUTE;
        rtca_time.min = RTCMIN;

        if (rtca_time.min != 0) /* Hour changed */
            goto finish;

        ev |= RTCA_EV_HOUR;
        rtca_time.hour = RTCHOUR;

#ifdef CONFIG_RTC_DST
        rtc_dst_hourly_update();
#endif

        if (rtca_time.hour != 0)        /* Day changed */
            goto finish;

        ev |= RTCA_EV_DAY;
        rtca_time.day = RTCDAY;
        rtca_time.dow = RTCDOW;

        if (rtca_time.day != 1) /* Month changed */
            goto finish;

        ev |= RTCA_EV_MONTH;
        rtca_time.mon = RTCMON;

        if (rtca_time.mon != 1) /* Year changed */
            goto finish;

        ev |= RTCA_EV_YEAR;
        rtca_time.year = RTCYEARL | (RTCYEARH << 8);
#ifdef CONFIG_RTC_DST
        /* calculate new DST switch dates */
        rtc_dst_calculate_dates(rtca_time.year, rtca_time.mon, rtca_time.day,
                                rtca_time.hour);
#endif
    }

 finish:
    /* append events, since ISR could be triggered
       multipe times until rtca_last_event gets parsed */
    rtca_last_event |= ev;

    /* exit from LPM3, give execution back to mainloop */
    _BIC_SR_IRQ(LPM3_bits);
}
