#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_WATCHDOG
#define CONFIG_GEOFENCE

#define USE_HTTP_POST
//#define CONFIG_RTC_DST
//#define CONFIG_RTC_DST_ZONE 4
//
// short timespan position averaging doesn't really improve 
// the location precision when the device can only see the sky partially
//#define CONFIG_POSITION_AVERAGING 


// debug flags, keep all of them disabled for a RELEASE version

//#define CALIBRATION
//#define DEBUG_GPS
//#define DEBUG_GPRS
//#define CONFIG_DEBUG

// soon to be deprecated
//#define USE_HTTP_GET

#endif
