
// minimal NMEA parser library. decodes GPRMC and GPGSA sentences
// checksum is ignored
//
//   author:          Petre Rodan <petre.rodan@simplex.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "drivers/rtc.h"
#include "drivers/gps.h"
#include "drivers/helper.h"
#include "proj.h"

#ifdef DEBUG_GPS
#include "drivers/uart1.h"
#endif

/// function that parses one full nmea sentence
/// input
///   s   - pointer to the beginning of the nmea string
///   len - size of the string
/// returns
///   EXIT_FAILURE - if string is malformed or too short to contain valid fix
///   EXIT_SUCCESS - if all went well
uint8_t nmea_parse(char *s, const uint8_t len)
{
    char *p = s;
    char *np, *nd;
    uint32_t tmp32 = 0;
    uint16_t tmp16 = 0;
    uint8_t i;
    uint16_t dlen, rlen;
    uint8_t *dst_p, *src_p;

    if (strstr(s, "RMC")) {
        if (len < 65) {
            return EXIT_FAILURE;
        }
        // time
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        if ((np - p) > 5) {
            // only parse 6 digits thus ignore milliseconds
            if ( str_to_uint32(p, &tmp32, 0, 6, 0, 235959) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
            mc_t.hour = tmp32 / 10000;
            mc_t.minute = (tmp32 % 10000) / 100;
            mc_t.second = (tmp32 % 100);
        }

        // fix
        p = strchr(p, ',') + 1;
        if (p[0] == 'A') {
            mc_t.fix = true;
        } else if (p[0] == 'V') {
            mc_t.fix = false;
            return EXIT_FAILURE;
        } else {
            return EXIT_FAILURE;
        }

        // latitude
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        nd = strchr(p, '.');
        dlen = nd - p;
        rlen = np - nd - 1;
        if ((dlen == 4) && (rlen == 4)) {
            if ( str_to_uint16(p, &tmp16, 0, dlen, 0, 9000) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
            mc_t.lat_deg = tmp16 / 100;
            mc_t.lat_min = tmp16 % 100;
            if ( str_to_uint16(p + dlen + 1, &mc_t.lat_fr, 0, rlen, 0, 9999) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
        }

        // lat_suffix
        p = strchr(p, ',') + 1;
        if (p[0] == 'N') {
            mc_t.lat_suffix = 'N';
        } else if (p[0] == 'S') {
            mc_t.lat_suffix = 'S';
        } else if (p[0] == ',') {
            mc_t.lat_suffix = 0x20;
        } else {
            return EXIT_FAILURE;
        }

        // longitude
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        nd = strchr(p, '.');
        dlen = nd - p;
        rlen = np - nd - 1;
        if ((dlen == 5) && (rlen == 4)) {
            if ( str_to_uint16(p, &tmp16, 0, dlen, 0, 18000) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
            mc_t.lon_deg = tmp16 / 100;
            mc_t.lon_min = tmp16 % 100;
            if ( str_to_uint16(p + dlen + 1, &mc_t.lon_fr, 0, rlen, 0, 9999) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
        }

        // lon_suffix
        p = strchr(p, ',') + 1;
        if (p[0] == 'W') {
            mc_t.lon_suffix = 'W';
        } else if (p[0] == 'E') {
            mc_t.lon_suffix = 'E';
        } else if (p[0] == ',') {
            mc_t.lon_suffix = 0x20;
        } else {
            return EXIT_FAILURE;
        }

        // speed
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        nd = strchr(p, '.');
        dlen = nd - p;
        if ((dlen > 0) && (dlen < (np - p))) {
            str_to_uint16(p, &mc_t.speed, 0, dlen, 0, 300);
        }

        // heading
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        nd = strchr(p, '.');
        dlen = nd - p;
        if ((dlen > 0) && (dlen < (np - p))) {
            str_to_uint16(p, &mc_t.heading, 0, dlen, 0, 359);
        }

        // date
        p = strchr(p, ',') + 1;
        np = strchr(p, ',');
        if ((np - p) > 5) {
            if ( str_to_uint32(p, &tmp32, 0, 6, 10000, 311299) == EXIT_FAILURE ) {
                return EXIT_FAILURE;
            }
            mc_t.day = tmp32 / 10000;
            mc_t.month = (tmp32 % 10000) / 100;
            mc_t.year = (tmp32 % 100) + 2000;
        }

        mc_t.fixtime = rtca_time.sys;

#ifdef CONFIG_POSITION_AVERAGING
        float tmp_f;

        mean_pos.speed_samples++;
        mean_pos.speed_sum += mc_t.speed;
        mean_pos.speed = mean_pos.speed_sum / mean_pos.speed_samples;

        if ((mc_t.speed < 1) && (mc_t.pdop < 400)) {
            mean_pos.lat += nmea_to_float(mc_t.lat_deg, mc_t.lat_min, mc_t.lat_fr, mc_t.lat_suffix);
            mean_pos.lon += nmea_to_float(mc_t.lon_deg, mc_t.lon_min, mc_t.lon_fr, mc_t.lon_suffix);
            mean_pos.pdop += mc_t.pdop;
            mean_pos.samples++;
            tmp_f = 1000000.0 * mean_pos.lat / mean_pos.samples;
            mc_t.lat_mean = (int32_t) tmp_f;
            tmp_f = 1000000.0 * mean_pos.lon / mean_pos.samples;
            mc_t.lon_mean = (int32_t) tmp_f;
            tmp_f = mean_pos.pdop / mean_pos.samples;
            mc_t.pdop_mean = (uint16_t) tmp_f; 
        }

        snprintf(str_temp, STR_LEN, "%u\t%u\tpos %ld %ld\r\n",
                 mean_pos.samples, mean_pos.speed_sum,
                 mc_t.lat_mean, mc_t.lon_mean);
        uart1_tx_str(str_temp, strlen(str_temp));
#endif

#ifdef DEBUG_GPS
        snprintf(str_temp, STR_LEN, "t %d %d.%04d%c %d %d.%04d%c  %lds %d %dkn\r\n",
                 mc_t.lat_deg, mc_t.lat_min, mc_t.lat_fr, mc_t.lat_suffix,
                 mc_t.lon_deg, mc_t.lon_min, mc_t.lon_fr, mc_t.lon_suffix,
                 rtca_time.sys - mc_t.fixtime, mc_t.pdop, mc_t.speed);
        uart1_tx_str(str_temp, strlen(str_temp));
#endif

        if (mc_f.pdop == 0) {
            mc_f.pdop = 9999;
        }

        if (mc_t.fix && (mc_t.pdop < 300) && ((rtca_time.sys > rtca_set_next) || (rtca_time.min != mc_f.minute))) {
            rtca_time.year = mc_t.year;
            rtca_time.mon = mc_t.month;
            rtca_time.day = mc_t.day;
            rtca_time.hour = mc_t.hour;
            rtca_time.min = mc_t.minute;
            rtca_time.sec = mc_t.second;

            rtca_set_time();
            rtca_set_next += RTC_SET_PERIOD;
        }

        // store the new fix only if pdop is better or FIX_INVALIDATE_PERIOD since last fix has elapsed
        if (mc_t.fix && ((mc_t.pdop <= mc_f.pdop) || (rtca_time.sys - mc_f.fixtime > FIX_INVALIDATE_PERIOD))) {

            mc_t.lat = nmea_to_float(mc_t.lat_deg, mc_t.lat_min, mc_t.lat_fr, mc_t.lat_suffix);
            mc_t.lon = nmea_to_float(mc_t.lon_deg, mc_t.lon_min, mc_t.lon_fr, mc_t.lon_suffix);

            src_p = (uint8_t *) & mc_t;
            dst_p = (uint8_t *) & mc_f;
            for (i = 0; i < sizeof(mc_t); i++) {
                *dst_p++ = *src_p++;
            }
            memset(&mc_t, 0, sizeof(mc_t));
        }

#ifdef DEBUG_GPS
        snprintf(str_temp, STR_LEN, "f %d %d.%04d%c %d %d.%04d%c  %lds %d\r\n",
                 mc_f.lat_deg, mc_f.lat_min, mc_f.lat_fr, mc_f.lat_suffix,
                 mc_f.lon_deg, mc_f.lon_min, mc_f.lon_fr, mc_f.lon_suffix,
                 rtca_time.sys - mc_f.fixtime, mc_f.pdop);
        uart1_tx_str(str_temp, strlen(str_temp));
#endif

    } else if (strstr(s, "GSA")) {
        uint8_t c = 0;
        for (i=0;i<len;i++) {
            if (*p == ',') {
                c++;
            }
            p++;
            if (c == 15) {
                np = strchr(p, ',');
                dlen = np - p;
                if ((dlen > 0) && (dlen < 6)) {
                    str_to_uint16(p, &mc_t.pdop, 0, dlen, 0, 9999);
                }
                break;
            }
        }
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

float nmea_to_float(const uint8_t deg, const uint8_t min, const uint16_t fr, const uint8_t suffix)
{
    float rv;

    rv = (float) deg + ((float) min + ((float) fr / 10000.0)) / 60.0;

    if ((suffix == 'S') || (suffix == 'W')) {
        rv *= -1.0;
    }

    return rv;
}

void geofence_calc(void)
{
    geo.lat_home = geo.lat_cur;
    geo.lon_home = geo.lon_cur;
    geo.lat_cur = mc_f.lat;
    geo.lon_cur = mc_f.lon;
    distance_between(geo.lat_home, geo.lon_home, geo.lat_cur, geo.lon_cur, &geo.distance, &geo.bearing);
}


void distance_between(const float lat1, const float long1, const float lat2,
                const float long2, float * distance, uint16_t * bearing) {
        //courtesy of http://arduiniana.org/libraries/tinygps/
        float delta = radians(long1 - long2);
        float sdlong = _sin(delta);
        float cdlong = _cos(delta);
        float rlat1 = radians(lat1);
        float rlat2 = radians(lat2);
        float slat1 = _sin(rlat1);
        float clat1 = _cos(rlat1);
        float slat2 = _sin(rlat2);
        float clat2 = _cos(rlat2);
        delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
        float x = delta ;
        float y = sdlong * clat2;
        delta = sq(delta);
        delta += sq(clat2 * sdlong);
        delta = _sqrt(delta);
        float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
        delta = _atan2f(delta, denom);
        *distance =  delta * 6372795;
        x = (180.0 * (_atan2f(y, x)/PI)) ;
        *bearing = ((int) -x + 360)%360 ;
}


