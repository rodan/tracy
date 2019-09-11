#ifndef __FM24_MEMTEST_H__
#define __FM24_MEMTEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef enum {
    TEST_00,
    TEST_FF,
    TEST_AA
} FM24_test_t;

uint32_t FM24_memtest(const uint16_t usci_base_addr, const uint32_t start_addr, const uint32_t stop_addr, FM24_test_t test, uint32_t *rows_tested);

#ifdef __cplusplus
}
#endif

#endif
