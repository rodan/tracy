#include <nmea/nmea.h>

#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef NMEA_WIN
#include <io.h>
#endif

#define SMAX 256

void trace(const char *str, int str_size)
{
    printf("Trace: ");
    write(1, str, str_size);
    printf("\n");
}

void error(const char *str, int str_size)
{
    printf("Error: ");
    write(1, str, str_size);
    printf("\n");
}

int main()
{
    nmeaINFO info;
    nmeaPARSER parser;
    nmeaPOS dpos;
    char buff[SMAX];
    //    "$GPRMC,173843,A,3349.896,N,11808.521,W,000.0,360.0,230108,013.4,E*69\r\n"
    uint8_t c, p = 0;

    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while (read(0, &c, 1)) {
        if (c == 0x0a) {
            buff[p] = c;
            nmea_parse(&parser, buff, SMAX, &info);
            nmea_info2pos(&info, &dpos);
            printf("Lat: %f, Lon: %f, Sig: %d, Fix: %d\n",
                   dpos.lat, dpos.lon, info.sig, info.fix);
            p = 0;
            memset(buff, 0, SMAX);
        } else {
            buff[p] = c;
            p++;
        }
    }

    nmea_parser_destroy(&parser);
    return 0;
}

