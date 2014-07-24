#ifndef __FLASH_H__
#define __FLASH_H__

#include "proj.h"

#define SEGMENT_D   ((uint8_t *)0x1800)
#define SEGMENT_C   ((uint8_t *)0x1880)
#define SEGMENT_B   ((uint8_t *)0x1900)

uint8_t flash_read(uint8_t *segment_addr, void *data, const uint8_t len);
uint8_t flash_save(uint8_t *segment_addr, void *data, const uint8_t len);
uint8_t flash_erase(uint8_t *segment_addr);

#endif
