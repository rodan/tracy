#ifndef __PROJ_H_
#define __PROJ_H_

#include <stdint.h>
#include <stdlib.h>
#include "gps.h"

#define true            1
#define false           0

#define LOC_MAX 8
#define STR_MAX 23

struct loc_t
{
    char nmea[2][STR_MAX];
    float lat[2];
    float lon[2];
    struct nmea_gprmc_t mc_t[2];
    float dist_math_f;
    double dist_math_dbl;
    float dist_s_f;
    uint16_t bearing_s_f;
};

struct loc_t l;

// fake declarations since we don't run on designated hardware

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
#define RTC_SET_INTERVAL  0

uint8_t rtc_not_set;
uint32_t rtca_set_next;
extern void rtca_set_time(void);


#endif
