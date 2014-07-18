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

// states that can be reached by the state machine
typedef enum {
    SIM900_ON,
    SIM900_OFF,
    SIM900_PWRKEY_ACT,
    SIM900_VBAT_ON,
    SIM900_PRE_IDLE,
    SIM900_IDLE,
    SIM900_AT,
    SIM900_WAITREPLY
//    SIM900_SLEEP,
//    SIM900_WAKEUP,
//    SIM900_VBAT_OFF
} sim900_state_t;

// commands that are compatible with the state machine
typedef enum {
    CMD_NULL,
    CMD_ON,
    CMD_OFF,
    CMD_FIRST_PWRON
} sim900_cmd_t;

// command type
typedef enum {
    CMD_UNSOLICITED,
    CMD_SOLICITED,
} sim900_cmd_type_t;

// return codes
typedef enum {
    RC_NULL,
    RC_OK,
    RC_ERROR,
    RC_TMOUT
} sim900_rc_t;

typedef enum {
    TTY_NULL,
    TTY_RX_PENDING,
    TTY_RX_WAIT
} sim900_tty_t;

struct sim900_t {
    uint8_t checks;
    uint8_t rdy;
    sim900_cmd_t cmd;
    sim900_cmd_type_t cmd_type;
    sim900_rc_t  rc;
    sim900_state_t next_state;
    sim900_tty_t console;
};

struct sim900_t sim900;

void sim900_init(void);
void sim900_init_messagebus(void);
void sim900_first_pwron(void);
uint16_t sim900_tx_str(char *str, const uint16_t size);
uint8_t sim900_tx_cmd(char *str, const uint16_t size);
uint8_t sim900_parse_rx(char *str, const uint16_t size);

#endif
