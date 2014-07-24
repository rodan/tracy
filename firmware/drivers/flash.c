
//  IO driver for information memory segments
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include "flash.h"

uint8_t flash_read(uint8_t *segment_addr, void *data, const uint8_t len)
{
    uint8_t *dst_p, *src_p;
    uint8_t i;

    src_p = segment_addr;
    dst_p = (uint8_t *) data;
    for (i = 0; i < len; i++) {
        *dst_p++ = *src_p++;
    }

    return 0;
}

uint8_t flash_save(uint8_t *segment_addr, void *data, const uint8_t len)
{
    uint8_t *src_p, *dst_p;
    uint8_t i;

    if ((segment_addr < SEGMENT_D) || (segment_addr > SEGMENT_B) || (len > 128)) {
        return 1;
    }

    dst_p = segment_addr;
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTHOLD;
#endif
    FCTL3 = FWPW;
    FCTL1 = FWPW + ERASE;
    *dst_p = 0; // dummy write to erase flash segment

    FCTL1 = FWPW + WRT;
    src_p = (uint8_t *) data;
    for (i = 0; i < len; i++) {
        *dst_p++ = *src_p++;
    }

    FCTL1 = FWPW;
    FCTL3 = FWPW + LOCK;

    return 0;
}

uint8_t flash_erase(uint8_t *segment_addr)
{
    if ((segment_addr < SEGMENT_D) || (segment_addr > SEGMENT_B)) {
        return 1;
    }

#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTHOLD;
#endif
    FCTL3 = FWPW;
    FCTL1 = FWPW + ERASE;
    *segment_addr = 0; // dummy write to erase flash segment
    FCTL1 = FWPW;
    FCTL3 = FWPW + LOCK;

    return 0;
}

