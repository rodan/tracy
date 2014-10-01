
// driver for CYPRESS FM24Vxx f-ram chips
//
//   author:          Petre Rodan <petre.rodan@simplex.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3

#include <inttypes.h>

#include "fm24.h"

#include "serial_bitbang.h"

int8_t fm24_seek(const uint32_t pos)
{

    return 0;
}

uint32_t fm24_read(uint8_t *buf, const uint32_t addr, const uint32_t nbyte)
{
    uint8_t rv = 0;

    rv = i2cm_start();

    if (rv != I2C_OK) {
        return 0;
    }

    rv = i2cm_tx(FM24V10_BA | (addr >> 16), I2C_WRITE);

    if (rv == I2C_ACK) {
        // f-ram memory address
        i2cm_tx((addr & 0xff00) >> 8, I2C_NO_ADDR_SHIFT);
        i2cm_tx(addr & 0xff, I2C_NO_ADDR_SHIFT);
        i2cm_stop();

        i2cm_start();
        i2cm_tx(FM24V10_BA | (addr >> 16), I2C_READ);
        rv = i2cm_rx(buf, nbyte, I2C_LAST_NAK);
    }

    i2cm_stop();

    if (rv != I2C_ACK) {
        return 0;
    }

    return nbyte;
}

uint32_t fm24_write(const uint8_t *buf, const uint32_t addr, const uint32_t nbyte)
{
    uint8_t rv = 0;
    uint32_t i = 0;

    rv = i2cm_start();

    if (rv != I2C_OK) {
        return 0;
    }

    // device slave address + memory page bit
    rv = i2cm_tx(FM24V10_BA | (addr >> 16), I2C_WRITE);

    if (rv == I2C_ACK) {
         // f-ram memory address
        i2cm_tx((addr & 0xff00) >> 8, I2C_NO_ADDR_SHIFT);
        i2cm_tx(addr & 0xff, I2C_NO_ADDR_SHIFT);
        // send data
        for (i=0;i<nbyte;i++) {
            rv = i2cm_tx(buf[i], I2C_NO_ADDR_SHIFT);
            if (rv != I2C_ACK) {
                continue;
            }
        }
    }

    i2cm_stop();
    return i;
}

