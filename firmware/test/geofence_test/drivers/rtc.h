#ifndef __RTC_H_
#define __RTC_H_

// fake rtc.h since we dont run this on the designated hardware

#include <stdint.h>

struct {
    uint32_t sys;
    uint16_t year;
    uint8_t mon;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} rtca_time;

//struct rtca_time_t rtca_time;
#define RTC_SET_PERIOD 0

uint32_t rtca_set_next;
extern void rtca_set_time(void);

#endif

