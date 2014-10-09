#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include "config.h"

// msp430's stdlib.h is missing these
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE    1
#endif

#define true            1
#define false           0

#define LED_SWITCH      P1OUT ^= BIT2
#define LED_ON          P1OUT |= BIT2
#define LED_OFF         P1OUT &= ~BIT2

#define GPS_ENABLE      P6OUT |= BIT0
#define GPS_DISABLE     P6OUT &= ~BIT0

#define GPS_IRQ_ENABLE  UCA0IE |= UCRXIE
#define GPS_IRQ_DISABLE UCA0IE &= ~UCRXIE

#ifdef PCB_REV1
#define GPS_BKP_ENABLE  P4OUT |= BIT6
#define GPS_BKP_DISABLE P4OUT &= ~BIT6
#endif

#define CHARGE_ENABLE   P6OUT &= ~BIT1
#define CHARGE_DISABLE  P6OUT |= BIT1

#define I2C_MASTER_DIR  P4DIR
#define I2C_MASTER_OUT  P4OUT
#define I2C_MASTER_IN   P4IN
#define I2C_MASTER_SCL  BIT6
#define I2C_MASTER_SDA  BIT7

// resistor divider ratio
// calculated as (R1+R2)/R2 * 100 / 1023 * 100
#define DIV_RAW         91
#define DIV_BAT         25

#define STR_LEN 64
char str_temp[STR_LEN];

#define RTC_SET_PERIOD  86400 // maximum period (in seconds) after which the local RTC is set using gps values

uint32_t gps_trigger_next;
uint32_t gprs_trigger_next;
uint32_t gprs_tx_next;

uint32_t rtca_set_next;
uint8_t rtc_not_set;

#define VERSION         5   // must be incremented if struct settings_t changes
#define FLASH_ADDR      SEGMENT_B

void main_init(void);
void check_events(void);
void settings_init(uint8_t * addr);
void adc_read(void);
void store_pkt(void);

#define MAX_PHONE_LEN           16
#define MAX_APN_LEN             20
#define MAX_USER_LEN            20
#define MAX_PASS_LEN            20
#define MAX_SERVER_LEN          20

#define CONF_SHOW_CELL_LOC      0x1
#define CONF_MIN_INTERFERENCE   0x2
#define CONF_ALWAYS_CHARGE      0x4

#define GEOFENCE_TRIGGER        100

// this struct will end up written into an information flash segment
// so it better not exceed 128bytes
// tracy_settings_t VERSION 4 is 117bytes long

struct tracy_settings_t {
    uint8_t ver;                    // settings struct version
    uint16_t settings;
    uint8_t ctrl_phone_len;
    char ctrl_phone[MAX_PHONE_LEN];
    uint8_t apn_len;
    char apn[MAX_APN_LEN];
    uint8_t user_len;
    char user[MAX_USER_LEN];
    uint8_t pass_len;
    char pass[MAX_PASS_LEN];
    uint8_t server_len;
    char server[MAX_SERVER_LEN];
    uint16_t port;
    uint8_t vref;
    uint16_t gps_loop_period;
    uint16_t gps_warmup_period;
    uint16_t gps_invalidate_period;
    uint16_t gprs_loop_period;
    uint16_t gprs_tx_period;
};

struct tracy_settings_t s;

static const struct tracy_settings_t defaults = {
    VERSION,                    // ver
    CONF_SHOW_CELL_LOC | CONF_ALWAYS_CHARGE,  // settings
    0,                          // ctrl_phone_len
    "",                         // ctrl_phone
    17,                         // gprs apn_len
    "live.vodafone.com",        // gprs apn
    4,                          // gprs user_len
    "live",                     // gprs user
    8,                          // gprs pass_len
    "vodafone",                 // gprs pass
    14,                         // server_len
    "trk.simplex.ro",           // server
    80,                         // port
    200,                        // adc vref
    120,                        // period (in seconds) between 2 gps measurements
    40,                         // period (in seconds) between gps powerup and NMEA data gathering
    20,                         // period (in seconds) in which the best PDOP is searched for
    900,                        // period (in seconds) between 2 gsm connection attempts (used to get tower id data and sms commands)
    3600                        // maximum period (in seconds) between 2 HTTP POSTs
};

struct tracy_stat_t {
    uint16_t v_bat; // LiPo battery voltage multiplied by 100
    uint16_t v_raw; // 5v rail voltage multiplied by 100
    uint16_t http_post_version;
    uint16_t http_msg_id;
};

struct tracy_stat_t stat;

typedef enum {
    MAIN_GPS_IDLE,
    MAIN_GPS_START,
    MAIN_GPS_INIT,
    MAIN_GPS_PDOP_RST,
    MAIN_GPS_STORE,
} main_gps_state_t;

main_gps_state_t gps_next_state;

typedef enum {
    MAIN_GPRS_IDLE,
    MAIN_GPRS_START,
} main_gprs_state_t;

main_gprs_state_t gprs_next_state;


#endif
