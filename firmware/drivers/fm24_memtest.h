#ifndef __FM24_MEMTEST_H__
#define __FM24_MEMTEST_H__

#include <inttypes.h>

typedef enum {
    TEST_00,
    TEST_FF,
    TEST_AA
} fm24_test_t;

uint32_t fm24_memtest(const uint32_t start_addr, const uint32_t stop_addr, fm24_test_t test, uint32_t *rows_tested);

#endif
