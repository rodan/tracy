
/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 *
 * port to msp430 by Petre Rodan <petre.rodan@simplex.ro>
 *
 */

#include "ir_remote.h"
#include "timer_a1.h"
#include "proj.h"
#include "sys_messagebus.h"

volatile irparams_t irparams;
decode_results results;

// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
uint8_t ir_decode(decode_results * results)
{
    results->rawbuf = irparams.rawbuf;
    results->rawlen = irparams.rawlen;
    if (irparams.rcvstate != STATE_STOP) {
        return ERR;
    }
    if (decode_rc5(results)) {
        return DECODED;
    }
    // decodeHash returns a hash on any input.
    // Thus, it needs to be last in the list.
    // If you add any decodes, add them before this.
    //if (decode_hash(results)) {
    //    return DECODED;
    //}
    // Throw away and start over
    ir_resume();
    return ERR;
}

uint8_t decode_rc5(decode_results * results)
{
    if (irparams.rawlen < MIN_RC5_SAMPLES + 2) {
        return ERR;
    }
    int offset = 1;             // Skip gap space
    long data = 0;
    int used = 0;
    // Get start bits
    if (get_rc_level(results, &offset, &used, RC5_T1) != MARK)
        return ERR;
    if (get_rc_level(results, &offset, &used, RC5_T1) != SPACE)
        return ERR;
    if (get_rc_level(results, &offset, &used, RC5_T1) != MARK)
        return ERR;
    int nbits;
    for (nbits = 0; offset < irparams.rawlen; nbits++) {
        int levelA = get_rc_level(results, &offset, &used, RC5_T1);
        int levelB = get_rc_level(results, &offset, &used, RC5_T1);
        if (levelA == SPACE && levelB == MARK) {
            // 1 bit
            data = (data << 1) | 1;
        } else if (levelA == MARK && levelB == SPACE) {
            // zero bit
            data <<= 1;
        } else {
            return ERR;
        }
    }

    if (data > 2047) {
        data -= 2048;
    }
    // Success
    results->bits = nbits;
    results->value = data;
    results->decode_type = RC5;
    return DECODED;
}

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
uint8_t decode_hash(decode_results * results)
{
    // Require at least 6 samples to prevent triggering on noise
    if (results->rawlen < 6) {
        return ERR;
    }
    uint32_t hash = FNV_BASIS_32;
    uint16_t i;
    for (i = 1; i + 2 < results->rawlen; i++) {
        int value = compare(results->rawbuf[i], results->rawbuf[i + 2]);
        // Add value into the hash
        hash = (hash * FNV_PRIME_32) ^ value;
    }
    results->value = hash;
    results->bits = 32;
    results->decode_type = UNKNOWN;
    return DECODED;
}

void ir_resume(void)
{
    irparams.rcvstate = STATE_IDLE;
    irparams.rawlen = 0;
}

// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
uint16_t get_rc_level(decode_results * results, int *offset, int *used,
                      const int t1)
{
    if (*offset >= results->rawlen) {
        // After end of recorded buffer, assume SPACE.
        return SPACE;
    }
    int width = results->rawbuf[*offset];
    int val = ((*offset) % 2) ? MARK : SPACE;
    int correction = (val == MARK) ? MARK_EXCESS : -MARK_EXCESS;

    int avail;
    if (MATCH(width, t1 + correction)) {
        avail = 1;
    } else if (MATCH(width, 2 * t1 + correction)) {
        avail = 2;
    } else if (MATCH(width, 3 * t1 + correction)) {
        avail = 3;
    } else {
        return -1;
    }

    (*used)++;
    if (*used >= avail) {
        *used = 0;
        (*offset)++;
    }
    return val;
}

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
uint8_t compare(const uint16_t oldval, const uint16_t newval)
{
    if (newval < oldval * .8) {
        return 0;
    } else if (oldval < newval * .8) {
        return 2;
    } else {
        return 1;
    }
}

uint8_t MATCH(const uint16_t measured, const uint16_t desired)
{
    return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

static void ir_isr(enum sys_message msg)
{
    uint8_t irdata = IR_IN & IR_PIN;
    if (irdata) {
        irdata = 1;
    }

    irparams.timer++;           // One more 50us tick
    if (irparams.rawlen >= RAWBUF) {
        // Buffer overflow
        irparams.rcvstate = STATE_STOP;
    }
    switch (irparams.rcvstate) {
    case STATE_IDLE:           // In the middle of a gap
        if (irdata == MARK) {
            if (irparams.timer < GAP_TICKS) {
                // Not big enough to be a gap.
                irparams.timer = 0;
            } else {
                // gap just ended, record duration and start recording transmission
                irparams.rawlen = 0;
                irparams.rawbuf[irparams.rawlen++] = irparams.timer;
                irparams.timer = 0;
                irparams.rcvstate = STATE_MARK;
            }
        }
        break;
    case STATE_MARK:           // timing MARK
        if (irdata == SPACE) {  // MARK ended, record time
            irparams.rawbuf[irparams.rawlen++] = irparams.timer;
            irparams.timer = 0;
            irparams.rcvstate = STATE_SPACE;
        }
        break;
    case STATE_SPACE:          // timing SPACE
        if (irdata == MARK) {   // SPACE just ended, record it
            irparams.rawbuf[irparams.rawlen++] = irparams.timer;
            irparams.timer = 0;
            irparams.rcvstate = STATE_MARK;
        } else {                // SPACE
            if (irparams.timer > GAP_TICKS) {
                // big SPACE, indicates gap between codes
                // Mark current code as ready for processing
                // Switch to STOP
                // Don't reset timer; keep counting space width
                irparams.rcvstate = STATE_STOP;
            }
        }
        break;
    case STATE_STOP:           // waiting, measuring gap
        if (irdata == MARK) {   // reset gap timer
            irparams.timer = 0;
        }
        break;
    }
}

void ir_init(void)
{
    __disable_interrupt();
    IR_SEL &= ~IR_PIN;
    IR_DIR &= ~IR_PIN;
    timer_a1_init();
    __enable_interrupt();
    sys_messagebus_register(&ir_isr, SYS_MSG_TIMER1_CRR0);
    ir_resume();
}
