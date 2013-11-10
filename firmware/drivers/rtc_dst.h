#ifndef RTC_DST_H_
#define RTC_DST_H_

#define RTC_DST_STATE_ST 0
#define RTC_DST_STATE_DST 1

#define DST_US 1
#define DST_MEX 2
#define DST_BRZ 3
#define DST_EU 4
#define DST_AUS 5
#define DST_NZ 6

struct rtc_dst_date_struct {
    uint8_t month;
    uint8_t day;
};

extern struct rtc_dst_date_struct dst_dates[];
extern uint8_t rtc_dst_state;

void rtc_dst_init(void);
void rtc_dst_calculate_dates(uint16_t year, uint8_t month, uint8_t day,
                             uint8_t hour);
void rtc_dst_hourly_update(void);

#endif
