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

#define ERR_SIM_MISSING         BIT0
#define ERR_IMEI_UNKNOWN        BIT1
#define ERR_SEND_FIX_GPRS       BIT2
#define ERR_SEND_FIX_SMS        BIT3
#define ERR_PARSE_SMS           BIT4

// state machine timeouts
#define SM_STEP_DELAY   81 // ~20ms
#define SM_DELAY        819 // ~200ms
//#define SM_R_DELAY      4700 // REPLY_TMOUT + RXBUF_TMOUT + ~100
#define SM_R_DELAY      5424 // REPLY_TMOUT + RXBUF_TMOUT + ~100

#define TASK_TMOUT 65500 // ~16s

// states that can be reached by the state machine
typedef enum {
    SIM900_ON,
    SIM900_OFF,
    SIM900_PWRKEY_ACT,
    SIM900_VBAT_ON,
    SIM900_PRE_IDLE,
    SIM900_WAIT_FOR_RDY,
    SIM900_IDLE,
    SIM900_AT,
    SIM900_WAITREPLY,
    SIM900_VBAT_OFF,
    SIM900_SET1,
    SIM900_GET_IMEI,
    SIM900_PARSE_SMS,
    SIM900_DEL_SMS,
    SIM900_CLOSE_SMS,
    SIM900_TEXT_INPUT,
    SIM900_TEXT_RCVD,
    SIM900_IP_INITIAL,
    SIM900_IP_START,
    SIM900_IP_GPRSACT,
    SIM900_IP_STATUS,
    SIM900_IP_CONNECT,
    SIM900_IP_CONNECT_OK,
    SIM900_IP_SEND,
    SIM900_IP_PUT,
    SIM900_IP_CLOSE,
    SIM900_IP_SHUT,
    SIM900_SEND_OK,
    SIM900_HTTP_REPLY
} sim900_state_t;

// commands that are compatible with the state machine
typedef enum {
    CMD_NULL,
    CMD_ON,
    CMD_OFF,
    CMD_FIRST_PWRON,
    CMD_GET_READY,
    CMD_GET_IMEI,
    CMD_SEND_SMS,
    CMD_SEND_GPRS,
    CMD_PARSE_SMS,
} sim900_cmd_t;

// highest level tasks for commanding a sim900
typedef enum {
    TASK_NULL,
    TASK_DEFAULT,
} sim900_task_t;

// return values for subtasks
typedef enum {
    SUBTASK_NO_REPLY,
    SUBTASK_GET_IMEI_OK,
    SUBTASK_SEND_FIX_GPRS_OK,
    SUBTASK_SEND_FIX_SMS_OK,
    SUBTASK_PARSE_SMS_OK
} sim900_task_rv_t;

// discrete states within a task
typedef enum {
    SUBTASK_NULL,
    SUBTASK_ON,
    SUBTASK_WAIT_FOR_RDY,
    SUBTASK_GET_IMEI,
    SUBTASK_SEND_FIX_GPRS,
    SUBTASK_SEND_FIX_SMS,
    SUBTASK_PWROFF,
    SUBTASK_PARSE_SMS,
    SUBTASK_SWITCHER
} sim900_task_state_t;

// command type
typedef enum {
    CMD_UNSOLICITED,
    CMD_SOLICITED,
    CMD_SOLICITED_GSN,
    CMD_IGNORE
} sim900_cmd_type_t;

// return codes
typedef enum {
    RC_NULL,
    RC_OK,
    RC_ERROR,
    RC_TMOUT,
    RC_TEXT_INPUT,
    RC_CMGS,
    RC_CMGL,
    RC_CMGR,
    RC_IMEI_RCVD,
    RC_STATE_IP_INITIAL,
    RC_STATE_IP_START,
    RC_STATE_IP_GPRSACT,
    RC_STATE_IP_STATUS,
    RC_STATE_IP_CONNECT,
    RC_STATE_IP_SHUT,
    RC_SEND_OK,
    RC_200_OK,
} sim900_rc_t;

typedef enum {
    TTY_NULL,
    TTY_RX_PENDING,
    TTY_RX_WAIT
} sim900_tty_t;


#define RDY         BIT0
#define CALL_RDY    BIT1

#define TASK_IDLE       BIT0
#define TASK_ONGOING    BIT1

#define TASK_MAX_RETRIES   3
#define TASK_QUEUE_SIZE    5

struct sim900_t {
    uint8_t checks;
    uint8_t rdy;
    uint8_t task_counter;
    uint8_t current_q;
    sim900_task_state_t queue[TASK_QUEUE_SIZE];
    uint16_t err;
    char imei[16];
    char sms_id[3];
    uint8_t sms_id_len;
    sim900_task_t task;
    sim900_task_state_t task_next_state;
    sim900_task_rv_t task_rv;
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
void sim900_start(void);
void sim900_halt(void);
void sim900_exec_default_task(void);

uint16_t sim900_tx_str(char *str, const uint16_t size);
uint8_t sim900_tx_cmd(char *str, const uint16_t size, const uint16_t reply_tmout);
uint8_t sim900_parse_rx(char *str, const uint16_t size);
uint8_t sim900_parse_sms(char *str, const uint16_t size);
void extract_str(const char *haystack, const char *needle, char *str, uint8_t *len, const uint8_t maxlen);

#endif
