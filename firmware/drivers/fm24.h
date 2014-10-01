#ifndef __FM24V_H__
#define __FM24V_H__

#include <inttypes.h>

#define FM24V10_BA  0x50    // base address

// FM24V10 has 131072 addressable bytes, so uint16_t is not enough

#ifdef CONFIG_HAVE_FM24V10
#define FM_LA   0x1FFFF     // last addressable address
#endif

int8_t fm24_seek(const uint32_t pos);
uint32_t fm24_read(uint8_t *buf, const uint32_t addr, const uint32_t nbyte);
uint32_t fm24_write(const uint8_t *buf, const uint32_t addr, const uint32_t nbyte);

#endif
