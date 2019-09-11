
#include "config.h"
#ifdef __I2C_CONFIG_H__

#include <string.h>
#include "config.h"
#include "fm24_memtest.h"
#include "fm24.h"

uint32_t FM24_memtest(const uint16_t usci_base_addr, const uint32_t start_addr,
                      const uint32_t stop_addr, FM24_test_t test, uint32_t * rows_tested)
{
    uint8_t data_r[8];          // test 8 bytes (1 row) at a time
    uint8_t data_w[8];
    uint32_t fragments, remain;
    uint32_t rv = 0;
    uint32_t i;
    uint8_t j;

    *rows_tested = 0;

    switch (test) {
    case TEST_00:
        memset(data_w, 0x00, 8);
        break;
    case TEST_FF:
        memset(data_w, 0xff, 8);
        break;
    case TEST_AA:
        memset(data_w, 0xaa, 8);
        break;
    }

    fragments = (stop_addr - start_addr) / 8;
    remain = (stop_addr - start_addr) % 8;

    for (i = 0; i < fragments; i++) {
        FM24_write(usci_base_addr, data_w, (i * 8) + start_addr, 8);
        memset(data_r, 0x11, 8);
        FM24_read(usci_base_addr, data_r, (i * 8) + start_addr, 8);
        for (j = 0; j < 8; j++) {
            if (data_w[j] != data_r[j]) {
                rv++;
            }
        }
        *rows_tested += 1;
    }

    if (remain) {
        FM24_write(usci_base_addr, data_w, (fragments * 8) + start_addr, remain);
        memset(data_r, 0x11, 8);
        FM24_read(usci_base_addr, data_r, (fragments * 8) + start_addr, remain);
        for (j = 0; j < remain; j++) {
            if (data_w[j] != data_r[j]) {
                rv++;
            }
        }
        *rows_tested += 1;
    }

    return rv;
}

#endif // __I2C_CONFIG_H__

