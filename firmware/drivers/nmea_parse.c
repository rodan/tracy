
#include <string.h>
#include <stdio.h>
#include "drivers/rtc.h"
#include "drivers/nmea_parse.h"
#include "proj.h"

uint8_t nmea_parse(char *s, const uint8_t len)
{
    char *p = s;
    char *np, *nd;
    uint32_t tmp32 = 0;
    uint16_t tmp16 = 0;
    uint8_t i;
    uint16_t dlen, rlen;
    uint8_t *dst_p, *src_p;

    if (strstr(s, "$GPRMC")) {
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

        if (mc_t.fix) {
            mc_t.fixtime = rtca_time.sys;
            src_p = (uint8_t *) & mc_t;
            dst_p = (uint8_t *) & mc_f;
            for (i = 0; i < sizeof(mc_t); i++) {
                *dst_p++ = *src_p++;
            }
            memset(&mc_t, 0, sizeof(mc_t));
        }
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min, const uint16_t max)
{
    uint16_t val = 0, pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

uint8_t str_to_uint32(char *str, uint32_t * out, const uint8_t seek,
                      const uint8_t len, const uint32_t min, const uint32_t max)
{
    uint32_t val = 0, pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
