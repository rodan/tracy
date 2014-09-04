
#include <stdio.h>
#include <math.h>
#include "gps.h"
#include "helper.h"

char loc_str[LOC_MAX][STR_MAX] = {
"44 25.9628N 26 1.8785E",
"44 25.9836N 26 1.7893E",
"44 25.9726N 26 1.8237E",
"44 25.9838N 26 1.8191E",
"44 26.0167N 26 1.8183E",
"44 26.0094N 26 1.8337E",
"44 26.0219N 26 1.8453E",
"44 25.9773N 26 1.8476E"
};


void haversine_dbl(const double lat1, const double long1, const double lat2, const double long2, double * distance)
{
    #define d2r (M_PI / 180.0)

    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    * distance = 6372795 * c;
}

void haversine_f(const float lat1, const float long1, const float lat2, const float long2, float * distance)
{
    #define d2r (M_PI / 180.0)

    float dlong = (long2 - long1) * d2r;
    float dlat = (lat2 - lat1) * d2r;
    float a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    * distance = 6372795 * c;
}

int main()
{
    uint8_t i, j;
    char *p;
    uint16_t tmp16 = 0;

    for (i=0;i<LOC_MAX-1;i++) {
        snprintf(l.nmea[0], STR_MAX, "%s", loc_str[i]);
        snprintf(l.nmea[1], STR_MAX, "%s", loc_str[i+1]);

        for (j=0;j<2;j++) {
            p = l.nmea[j];
            // latitude
            str_to_uint16(p, &tmp16, 0, 2, 0, 90);
            l.mc_t[j].lat_deg = tmp16;
            str_to_uint16(p + 3, &tmp16, 0, 2, 0, 60);
            l.mc_t[j].lat_min = tmp16;
            str_to_uint16(p + 6, &l.mc_t[j].lat_fr, 0, 4, 0, 9999);
            l.mc_t[j].lat_suffix = p[10];

            // longitude
            str_to_uint16(p + 12, &tmp16, 0, 2, 0, 90);
            l.mc_t[j].lon_deg = tmp16;
            str_to_uint16(p + 15, &tmp16, 0, 1, 0, 60); // XXX
            l.mc_t[j].lon_min = tmp16;
            str_to_uint16(p + 17, &l.mc_t[j].lon_fr, 0, 4, 0, 9999);
            l.mc_t[j].lon_suffix = p[21];

            l.lat[j] = nmea_to_float(l.mc_t[j].lat_deg, l.mc_t[j].lat_min, l.mc_t[j].lat_fr, l.mc_t[j].lat_suffix);
            l.lon[j] = nmea_to_float(l.mc_t[j].lon_deg, l.mc_t[j].lon_min, l.mc_t[j].lon_fr, l.mc_t[j].lon_suffix);

        }

        distance_between(l.lat[0], l.lon[0], l.lat[1], l.lon[1], &l.dist_s_f, &l.bearing_s_f);
        haversine_dbl(l.lat[0], l.lon[0], l.lat[1], l.lon[1], &l.dist_math_dbl);
        haversine_f(l.lat[0], l.lon[0], l.lat[1], l.lon[1], &l.dist_math_f);

        printf("{\r\n");
        printf("\tsrc: %s\r\n\tsrc: %02d %02d.%04d%c, %02d %02d.%04d%c\r\n", 
                l.nmea[0],
                l.mc_t[0].lat_deg, l.mc_t[0].lat_min, l.mc_t[0].lat_fr, l.mc_t[0].lat_suffix,
                l.mc_t[0].lon_deg, l.mc_t[0].lon_min, l.mc_t[0].lon_fr, l.mc_t[0].lon_suffix
                );
        printf("\tdst: %s\r\n\tdst: %02d %02d.%04d%c, %02d %02d.%04d%c\r\n", 
                l.nmea[1],
                l.mc_t[1].lat_deg, l.mc_t[1].lat_min, l.mc_t[1].lat_fr, l.mc_t[1].lat_suffix,
                l.mc_t[1].lon_deg, l.mc_t[1].lon_min, l.mc_t[1].lon_fr, l.mc_t[1].lon_suffix
                );
        printf("\tsrc: %f %f\r\n\tdst: %f %f\r\n", 
                l.lat[0], l.lon[0],
                l.lat[1], l.lon[1]
                );
        printf("\tdist_simple float:     %f m, bearing %d\r\n", l.dist_s_f, l.bearing_s_f);
        printf("\tdist_haversine double: %f m\r\n", l.dist_math_dbl);
        printf("\tdist_haversine float:  %f m\r\n", l.dist_math_f);
        printf("}\r\n");
    }

    return EXIT_SUCCESS;
}

