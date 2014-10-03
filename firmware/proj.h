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
uint32_t rtca_set_next;

#define VERSION         2   // must be incremented if struct settings_t changes
#define FLASH_ADDR      SEGMENT_B

void main_init(void);
void check_events(void);
void settings_init(uint8_t * addr);
void adc_read(void);
void store_pkt(void);

#define MAX_PHONE_LEN   16
#define MAX_APN_LEN     20
#define MAX_USER_LEN    20
#define MAX_PASS_LEN    20
#define MAX_SERVER_LEN  20

#define CONF_SHOW_CELL_LOC     0x1
#define CONF_MIN_INTERFERENCE  0x2
#define CONF_ALWAYS_CHARGE     0x4

// this struct will end up written into an information flash segment
// so it better not exceed 128bytes
// tracy_settings_t ver1 is 108bytes long

struct tracy_settings_t {
    uint8_t ver;                // firmware version
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
};

struct tracy_settings_t s;

static const struct tracy_settings_t defaults = {
    VERSION,                    // ver
    CONF_SHOW_CELL_LOC | CONF_MIN_INTERFERENCE | CONF_ALWAYS_CHARGE,  // settings
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
    200                         // adc vref
};

struct tracy_stat_t {
    uint16_t v_bat; // LiPo battery voltage multiplied by 100
    uint16_t v_raw; // 5v rail voltage multiplied by 100
    uint16_t http_post_version;
    uint16_t http_msg_id;
};

struct tracy_stat_t stat;

typedef enum {
    MAIN_IDLE,
    MAIN_START_GPS,
    MAIN_INIT_GPS,
    MAIN_SWITCHER,
    MAIN_START_GPRS
} main_state_t;

main_state_t main_next_state;

#endif
