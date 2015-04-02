#ifndef _CONFIG_H_
#define _CONFIG_H_

// short timespan position averaging doesn't really improve 
// the location precision when the device can only see the sky partially
//#define CONFIG_POSITION_AVERAGING 

// debug flags, keep all of them disabled for a RELEASE version

//#define DEBUG_GPS
//#define DEBUG_GPRS
//#define CONFIG_DEBUG


// dont touch
#define USE_WATCHDOG
#define CONFIG_GEOFENCE
#define CONFIG_HAVE_FM24CL64B
//#define CONFIG_HAVE_FM24V10

// only a few of the F-RAM chips have explicit sleep mode option
#ifdef CONFIG_HAVE_FM24V10
    #define FM24_HAS_SLEEP_MODE
#endif

#endif
