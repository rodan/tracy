#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_XT1
//#define USE_WATCHDOG
#define HARDWARE_I2C
//#define IRQ_I2C

// short timespan position averaging doesn't really improve 
// the location precision when the device can only see the sky partially
//#define CONFIG_POSITION_AVERAGING 

// debug flags, keep all of them disabled for a RELEASE version

//#define DEBUG_GPS
//#define DEBUG_GPRS
//#define CONFIG_DEBUG

// dont touch
#define USE_WATCHDOG
//#define CONFIG_GEOFENCE

#define CONFIG_FM24CL64B
#define CONFIG_CYPRESS_FM24
//#define CONFIG_HAVE_FM24V10

#include "i2c_config.h"

// only a few of the F-RAM chips have explicit sleep mode option
#ifdef CONFIG_FM24V10
    #define FM24_HAS_SLEEP_MODE
#endif

#endif
