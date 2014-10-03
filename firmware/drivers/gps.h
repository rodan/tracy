#ifndef __GPS_H__
#define __GPS_H__

#include <inttypes.h>

#define FIX_INVALIDATE_PERIOD 240 // time in seconds after which the fix is no longer considered valid

struct nmea_gprmc_t {
    uint16_t year;     // date and time received from the sattelites
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    float lat;         // latitude
    float lon;         // longitude
    uint16_t pdop;     // position dilution of precision
    uint16_t speed;    // speed in _knots_ ?!
    uint16_t heading;  // heading in degrees
    uint32_t fixtime;  // system time when the fix was received
    uint8_t lat_deg;
    uint8_t lat_min;
    uint16_t lat_fr;
    uint8_t lat_suffix;
    uint8_t lon_deg;
    uint8_t lon_min;
    uint16_t lon_fr;
    uint8_t lon_suffix;
    uint8_t fix;

#ifdef CONFIG_POSITION_AVERAGING
    int32_t lat_mean;
    int32_t lon_mean;
    uint16_t pdop_mean;
#endif
};

struct nmea_gprmc_t mc_f;
struct nmea_gprmc_t mc_t;

#ifdef CONFIG_POSITION_AVERAGING
struct mean_pos_t {
    float lat;
    float lon;
    float pdop;
    uint16_t samples;
    float speed;
    uint16_t speed_sum;
    uint16_t speed_samples;
};

struct mean_pos_t mean_pos;
#endif

#ifdef CONFIG_GEOFENCE
struct geofence_t {
    float lat_home;
    float lon_home;
    float lat_cur;
    float lon_cur;
    float distance;
    uint16_t bearing;
};

struct geofence_t geo;
#endif

uint8_t nmea_parse(char *str, const uint8_t len);
float nmea_to_float(const uint8_t deg, const uint8_t min, const uint16_t fr, const uint8_t suffix);
//void haversine_km(struct geofence_t geofence);
void distance_between(const float lat1, const float long1, const float lat2,
                const float long2, float * distance, uint16_t * bearing);

void geofence_calc(void);

#endif
