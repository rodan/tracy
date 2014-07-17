#ifndef __SIM900_H__
#define __SIM900_H__

#include "proj.h"

#define SIM900_DTR_HIGH         P1OUT |= BIT3
#define SIM900_DTR_LOW          P1OUT &= ~BIT3
#define SIM900_RTS_HIGH         P1OUT |= BIT6
#define SIM900_RTS_LOW          P1OUT &= ~BIT6
#define SIM900_PWRKEY_HIGH      P1OUT &= ~BIT7
#define SIM900_PWRKEY_LOW       P1OUT |= BIT7
#define SIM900_VBAT_ENABLE      P2OUT |= BIT0
#define SIM900_VBAT_DISABLE     P2OUT &= ~BIT0

#define SIM900_CTS              BIT5
#define SIM900_CTS_IN           P1IN & BIT5
#define SIM900_UCAIFG           UCA1IFG
#define SIM900_UCATXBUF         UCA1TXBUF

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
    CMD_NULL,
    CMD_ON,
    CMD_OFF
} sim900_cmd_t;

typedef enum {
    TTY_NULL,
    TTY_RX_PENDING,
    TTY_RX_WAIT
} sim900_tty_t;

struct sim900_t {
    uint32_t sim900_sc; // timestamp of state change
    sim900_cmd_t cmd;
    sim900_state_t next_state;
    sim900_tty_t console;
};

struct sim900_t sim900;

void sim900_init(void);
void sim900_setup(void);
uint16_t sim900_tx_str(char *str, const uint16_t size);

#endif
