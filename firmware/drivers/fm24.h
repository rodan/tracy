#ifndef __FM24_H__
#define __FM24_H__

#include <inttypes.h>

#define FM24_BA      0x50    // base address
#define FM24_RSVD    0xF8    // reserved slave address
#define FM24_SLEEP   0x86    // sleep command

#define FM24_MAX_RETRY   4   // how many times to retry read/write opperations

// FM24V10 has 131072 addressable bytes, so uint16_t is not enough

#ifdef CONFIG_HAVE_FM24V10
#define FM_LA        0x1FFFF         // last addressable byte
#define MAX_SEG      12
#endif

#ifdef CONFIG_HAVE_FM24CL64B
#define FM_LA        0x1FFF          // last addressable byte
#define MAX_SEG      8
#endif

// sim900 can only send about 1000 bytes at a time
// so the unsent data has to be segmented up
// transmitted packets also include a header which can become 106bytes long
// MAX_SEG must be at least 4
#define MAX_SEG_SIZE  1000 - 106

#if MAX_SEG * MAX_SEG_SIZE > FM_LA
#error "invalid segmentation in fm24.h"
#endif

uint8_t fm24_status;
#define FM24_AWAKE  0x1

uint8_t fm24_seek(const uint32_t addr);
uint32_t fm24_read(uint8_t * buf, const uint32_t nbyte);
uint32_t fm24_read_from(uint8_t * buf, const uint32_t addr,
                        const uint32_t nbyte);
uint32_t fm24_write(const uint8_t * buf, const uint32_t addr,
                    const uint32_t nbyte);
uint8_t fm24_sleep(void);

// helpers

struct mem_mgmt_t {
    uint32_t e;                 // first empty address
    uint32_t seg[MAX_SEG+1];    // [start, end] addresses for the MAX_SEG_SIZE byte segments
    uint8_t seg_num;            // number of allocated segments
};

struct mem_mgmt_t m;

uint32_t fm24_data_len(const uint32_t first, const uint32_t last);

#endif
