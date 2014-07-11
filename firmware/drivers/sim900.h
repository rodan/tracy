#ifndef __SIM900_H__
#define __SIM900_H__

#include "proj.h"

#define SIM900_DTR_HIGH       P1OUT |= BIT3
#define SIM900_DTR_LOW        P1OUT &= ~BIT3
#define SIM900_RTS_HIGH       P1OUT |= BIT6
#define SIM900_RTS_LOW        P1OUT &= ~BIT6
#define SIM900_PWRKEY_HIGH    P1OUT &= ~BIT7
#define SIM900_PWRKEY_LOW     P1OUT |= BIT7
#define SIM900_VBAT_ENABLE    P2OUT |= BIT0
#define SIM900_VBAT_DISABLE   P2OUT &= ~BIT0

typedef enum {
    SIM900_ON,
    SIM900_OFF,
    SIM900_PWRKEY_ACT,
    SIM900_VBAT_ON,
    SIM900_PRE_IDLE,
    SIM900_IDLE,
//    SIM900_SLEEP,
//    SIM900_WAKEUP,
//    SIM900_VBAT_OFF
} sim900_state_t;

typedef enum {
    CSIM900_NULL,
    CSIM900_ON,
    CSIM900_OFF
} sim900_cmd_t;

struct sim900_t {
    uint32_t sim900_sc; // timestamp of state change
    sim900_cmd_t cmd;
    sim900_state_t next_state;
};

struct sim900_t sim900;

void sim900_init(void);

#endif
