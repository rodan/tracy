#!/bin/bash

strip_zero()
{
	cat | sed 's|^0||'
}


cat << EOF > drivers/rtca_now.h

#ifndef __RTCA_NOW_H__
#define __RTCA_NOW_H__

//#define COMPILE_YEAR `date -d '+1 minute 20 seconds' +%Y`
//#define COMPILE_MON `date -d '+1 minute 20 seconds' +%m | strip_zero`
//#define COMPILE_DAY `date -d '+1 minute 20 seconds' +%d | strip_zero`
//#define COMPILE_DOW `date -d '+1 minute 20 seconds' +%u | sed 's|7|0|'`
//#define COMPILE_HOUR `date -d '+1 minute 20 seconds' +%H | strip_zero`
//#define COMPILE_MIN `date -d '+1 minute 20 seconds' +%M | strip_zero`

#define COMPILE_YEAR 2001
#define COMPILE_MON 1
#define COMPILE_DAY 1
#define COMPILE_DOW 1
#define COMPILE_HOUR 1
#define COMPILE_MIN 1

#endif
EOF
