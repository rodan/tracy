#ifndef __RTC_H_
#define __RTC_H_

#include <stdint.h>

struct rtca_time_t 
{
    uint32_t sys; // fake system time
};

struct rtca_time_t rtca_time;

#endif

