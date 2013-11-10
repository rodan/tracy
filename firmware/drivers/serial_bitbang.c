
//  software bitbang of serial protocols
//  currently supported:
//        - i2c master
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3


#include "serial_bitbang.h"

// returns one of I2C_ACK, I2C_NAK, I2C_MISSING_SCL_PULLUP or I2C_MISSING_SDA_PULLUP
uint8_t i2cm_rxfrom(const uint8_t slave_address, uint8_t* data, uint16_t length)
{
    uint8_t rv;
    rv = i2cm_start();
    if (rv != I2C_OK) {
        return rv;
    }
    rv = i2cm_tx(slave_address, 1);
    if (rv == I2C_ACK) {
        i2cm_rx(data, length, 0);
    }
    i2cm_stop();
    return rv;
}

// returns  I2C_ACK or I2C_NAK
uint8_t i2cm_tx(const uint8_t data, const uint8_t options)
{
    uint8_t rv;
    register uint8_t i = 0;
    register uint8_t slarw = 0;

    if (options & I2C_READ) {
        slarw = ( data << 1 ) | BIT0;
    } else if (options & I2C_WRITE) {
        slarw = data << 1;
    } else if (options & I2C_NO_ADDR_SHIFT) {
        slarw = data;
    }

    for (i=0; i<8; i++) {
        if (slarw & 0x80) {
            sda_high;
        } else {
            sda_low;
        }
        slarw <<= 1;
        scl_high;
        delay_c;
        delay_c;
        scl_low;
    }
    sda_high;
    scl_high;
    delay_c;

    if (I2C_MASTER_IN & I2C_MASTER_SDA) {
        rv = I2C_NAK;
    } else {
        rv = I2C_ACK;
    }
    scl_low;
    return rv;
}

uint8_t i2cm_rx(uint8_t *buf, const uint16_t length, const uint8_t options)
{
    uint8_t data;
    volatile unsigned int i,j;

    if (options & I2C_SDA_WAIT) {
        delay_c;
        // wait until the data line is pulled low
        // this method is used by sensirion sensors
        while (I2C_MASTER_IN & I2C_MASTER_SDA) {
            _NOP();
        }
    }

    for (j=0; j<length; j++) {
        sda_high;
        data = 0;
        i = 0;
        for (; i < 8; ++i) {
            scl_high;
            if (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
                _NOP(); // wait if slave holds the clk low
            }
            data <<= 1;
            if (I2C_MASTER_IN & I2C_MASTER_SDA)
                data |= 0x01;
            scl_low;
        }
        *buf++ = data;
        if (j != length-1) {
            sda_low;
        }
        // send ack
        scl_high;
        delay_c;
        scl_low;
    }
    return 0;
}

// returns one of I2C_OK, I2C_MISSING_SCL_PULLUP and/or I2C_MISSING_SDA_PULLUP
uint8_t i2cm_start(void)
{
    uint8_t rv = 0;
    // set both SCL and SDA pins as inputs
    I2C_MASTER_DIR &= ~(I2C_MASTER_SCL + I2C_MASTER_SDA);
    // when direction is switched from INPUT to OUTPUT, the pin will go LOW
    I2C_MASTER_OUT &= ~(I2C_MASTER_SDA | I2C_MASTER_SCL);
    sda_high;
    delay_s;
    scl_high;
    delay_s;
    if (!(I2C_MASTER_IN & I2C_MASTER_SDA)) {
        rv |= I2C_MISSING_SDA_PULLUP;
    }
    if (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
        rv |= I2C_MISSING_SCL_PULLUP;
    }
    if (rv) {
        return rv;
    }
    sda_low;
    delay_s;
    scl_low;
    delay_s;
    return I2C_OK;
}

void i2cm_stop(void)
{
    sda_low;
    delay_s;
    scl_high;
    delay_s;
    sda_high;
    delay_s;
}


