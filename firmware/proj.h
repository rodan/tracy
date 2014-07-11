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

#define STR_LEN 64
char str_temp[STR_LEN];

void main_init(void);

void sleep(void);
void wake_up(void);
void check_events(void);

void opt_power_enable(void);
void opt_power_disable(void);
void charge_enable(void);
void charge_disable(void);
void sw_enable(void);
void sw_disable(void);

uint8_t read_ps(void);

#endif
