#ifndef __NMEA_PARSE_H__
#define __NMEA_PARSE_H__

#include <inttypes.h>

struct nmea_gprmc {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t fix;
    uint8_t lat_deg;
    uint8_t lat_min;
    uint16_t lat_fr;
    uint8_t lat_suffix;
    uint8_t lon_deg;
    uint8_t lon_min;
    uint16_t lon_fr;
    uint8_t lon_suffix;
    uint16_t speed;
    uint16_t heading;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

struct nmea_gprmc mc_f;
struct nmea_gprmc mc_t;

uint8_t nmea_parse(char *str, const uint8_t len);
uint8_t str_to_uint32(char *str, uint32_t * out, const uint8_t seek,
                      const uint8_t len, const uint32_t min, const uint32_t max);
uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min, const uint16_t max);


#endif
