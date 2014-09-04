#ifndef __GPS_H__
#define __GPS_H__

#include <inttypes.h>

struct nmea_gprmc_t {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
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
    uint8_t fix;
    uint32_t fixtime;
};

struct nmea_gprmc_t mc_f;
struct nmea_gprmc_t mc_t;

#ifdef CONFIG_GEOFENCE
struct geofence_t {
    float lat_home;
    float lon_home;
    float lat_cur;
    float lon_cur;
    float distance;
    int bearing;
};

struct geofence_t geo;
#endif

uint8_t nmea_parse(char *str, const uint8_t len);
float nmea_to_float(const uint8_t deg, const uint8_t min, const uint16_t fr, const uint8_t suffix);
//void haversine_km(struct geofence_t geofence);
void distance_between(const float lat1, const float long1, const float lat2,
                const float long2, float * distance, uint16_t * bearing);

#endif
