
#include <string.h>

#include "proj.h"
#include "fm24_memtest.h"
#include "fm24.h"

uint32_t fm24_memtest(const uint32_t start_addr, const uint32_t stop_addr, fm24_test_t test, uint32_t *rows_tested)
{
    uint8_t data_r[8]; // test 8 bytes (1 row) at a time
    uint8_t data_w[8];
    uint32_t fragments;
    uint32_t rv = 0;
    uint16_t i;
    uint8_t j;

    *rows_tested = 0;

    switch (test) {
        case TEST_00:
            for (i=0; i<8; i++) {
                data_w[i] = 0x00;
            }
        break;
        case TEST_FF:
            for (i=0; i<8; i++) {
                data_w[i] = 0xff;
            }
        break;
        case TEST_AA:
            for (i=0; i<8; i++) {
                data_w[i] = 0xaa;
            }
        break;
    }

    fragments = (stop_addr + 1 - start_addr) / 8;

    for (i = 0; i < fragments; i++) {
        fm24_write(data_w, (i * 8) + start_addr, 8);
        memset(data_r, 0x11, 8);
        fm24_read_from(data_r, (i * 8) + start_addr, 8);
        for (j=0; j<8; j++) {
            if (data_w[j] != data_r[j]) {
                rv++;
            }
        }
        *rows_tested += 1;
    }

    return rv;
}


