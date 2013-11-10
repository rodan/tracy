#ifndef __IR_REMOTE_H__
#define __IR_REMOTE_H__

#include <stdint.h>

/*
#define IR_SEL P2SEL
#define IR_DIR P2DIR
#define IR_IN  P2IN
#define IR_PIN BIT0
*/

#define USECPERTICK 50
#define RAWBUF 28

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

// information for the interrupt handler
typedef struct {
    uint8_t rcvstate;           // state machine
    uint16_t timer;             // state timer, counts 50uS ticks.
    uint16_t rawbuf[RAWBUF];    // raw data
    uint8_t rawlen;             // counter of entries in rawbuf
} irparams_t;

// Results returned from the decoder
typedef struct {
    int decode_type;            // NEC, SONY, RC5, UNKNOWN
    uint32_t value;             // Decoded value
    int bits;                   // Number of bits in decoded value
    volatile uint16_t *rawbuf;  // Raw intervals in .5 us ticks
    int rawlen;                 // Number of records in rawbuf.
} decode_results;

extern volatile irparams_t irparams;
extern decode_results results;

#define TOLERANCE 25            // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.)
#define UTOL (1.0 + TOLERANCE/100.)

#define _GAP 5000               // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

#define IR_CCR    (MCLK_FREQ * USECPERTICK / 1000000)

// IR detector output is active low
#define MARK  0
#define SPACE 1

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

#define ERR 0
#define DECODED 1

//#define NEC 1
//#define SONY 2
#define RC5 3
//#define RC6 4
//#define DISH 5
//#define SHARP 6
#define UNKNOWN -1

#define MIN_RC5_SAMPLES 11
#define RC5_T1		889

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261UL

void ir_init(void);
void ir_halt(void);
void ir_resume(void);
uint8_t ir_decode(decode_results * results);
uint8_t decode_rc5(decode_results * results);
uint8_t decode_hash(decode_results * results);
uint16_t get_rc_level(decode_results * results, int *offset, int *used,
                      const int t1);
uint8_t compare(const uint16_t oldval, const uint16_t newval);
uint8_t MATCH(const uint16_t measured, const uint16_t desired);

#endif
