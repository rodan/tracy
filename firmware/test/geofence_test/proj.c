
#include <stdio.h>
#include <math.h>
#include "gps.h"
#include "helper.h"

int main()
{
    char loc_str[LOC_MAX][STR_MAX] = {
"44 25.9822N 26 1.8231E",
"44 25.9781N 26 1.8269E",
"44 25.9779N 26 1.8404E",
"44 25.9839N 26 1.8798E",
"44 25.9695N 26 1.8581E",
"44 25.9649N 26 1.8655E",
"44 25.9736N 26 1.8708E",
"44 25.9697N 26 1.8686E",
"44 26.0184N 26 1.8377E",
"44 25.9694N 26 1.8593E",
"44 26.0228N 26 1.8454E",
"44 25.9678N 26 1.8447E",
"44 25.9795N 26 1.8527E",
"44 25.9835N 26 1.8606E",
"44 25.9873N 26 1.8827E",
"44 25.9765N 26 1.8703E",
"44 25.9856N 26 1.8722E",
"44 26.0550N 26 1.8231E",
"44 26.0565N 26 1.8316E",
"44 26.0473N 26 1.8294E"
    };    

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

        distance_between(l.lat[0], l.lon[0], l.lat[1], l.lon[1], &l.dist_s_fl, &l.bearing_s_fl);

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
        printf("\tdist_simple: %f m, %d\r\n", l.dist_s_fl, l.bearing_s_fl);
        printf("}\r\n");
    }

    return EXIT_SUCCESS;
}

