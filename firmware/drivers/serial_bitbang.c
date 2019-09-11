
//  software bitbang of serial protocols
//  currently supported:
//        - i2c master
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/
//  license:         BSD

#include "config.h"
#ifdef __I2C_CONFIG_H__

#include <msp430.h>

#include "serial_bitbang.h"

// returns one of I2C_OK, I2C_MISSING_SCL_PULLUP and/or I2C_MISSING_SDA_PULLUP
uint8_t i2cm_start(uint8_t options)
{
    uint8_t rv = 0;
    // set both SCL and SDA pins as inputs
    I2C_MASTER_DIR &= ~(I2C_MASTER_SCL + I2C_MASTER_SDA);
    I2C_MASTER_OUT &= ~(I2C_MASTER_SDA | I2C_MASTER_SCL);

    // bus is currently inactive

    sda_high;
    delay_s;
    scl_high;
    delay_s;
    if (!(I2C_MASTER_IN & I2C_MASTER_SDA)) {
        // pin is low, but should have been pulled high by an external resistor
        rv |= I2C_MISSING_SDA_PULLUP;
    }
    if (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
        // pin is low, but should have been pulled high by an external resistor
        rv |= I2C_MISSING_SCL_PULLUP;
    }
    if (rv) {
        return rv;
    }
    if (options & I2C_SHT_INIT) {
        scl_low;
        delay_c;
        delay_c;
        scl_high;
        sda_low;
        scl_low;
        scl_high;
        sda_high;
        scl_low;
    } else {
        // i2c start sequence
        sda_low;
        delay_s;
        scl_low;
        delay_s;
    }
    return I2C_OK;
}

void i2cm_stop(uint8_t options)
{
    if (options & I2C_SHT_INIT) {
        sda_high;
        scl_high;
    } else {
        sda_low;
        delay_s;
        scl_high;
        delay_s;
        sda_high;
        delay_s;
    }
}

// returns  I2C_ACK or I2C_NAK
uint8_t i2cm_tx(const uint8_t data, const uint8_t options)
{
    uint8_t rv;
    register uint8_t i = 0;
    register uint8_t slarw = 0;

    if (options & I2C_NO_ADDR_SHIFT) {
        slarw = data;
    } else if (options & I2C_WRITE) {
        slarw = data << 1;
    } else if (options & I2C_READ) {
        slarw = (data << 1) | BIT0;
    }

    for (i = 0; i < 8; i++) {
        if (slarw & 0x80) {
            sda_high;
        } else {
            sda_low;
        }
        slarw <<= 1;
        scl_high;
        delay_c;
        delay_c;
        while (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
            delay_c;         // wait if slave holds the clk low
        }
        scl_low;
    }
    sda_high;
    delay_c;
    delay_c;
    scl_high;
    delay_c;
    delay_c;
    while (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
        delay_c;         // wait if slave holds the clk low
    }

    if (I2C_MASTER_IN & I2C_MASTER_SDA) {
        rv = I2C_NAK;
    } else {
        rv = I2C_ACK;
    }
    scl_low;
    return rv;
}

// returns one of I2C_ACK, I2C_NAK, I2C_MISSING_SCL_PULLUP or I2C_MISSING_SDA_PULLUP
uint8_t i2cm_tx_buff(const uint8_t * data, uint16_t data_len, const uint8_t options)
{
    uint8_t rv = I2C_ACK;
    uint16_t i;

    for (i=0;i<data_len;i++) {
        rv = i2cm_tx(data[i], I2C_NO_ADDR_SHIFT | options);
        if (rv != I2C_ACK) {
            break;
        }
    }

    return rv;
}

uint8_t i2cm_rx(uint8_t * buf, const uint16_t length, const uint8_t options)
{
    uint8_t data;
    volatile unsigned int i, j;

    if (options & I2C_SDA_WAIT) {
        delay_c;
        // wait until the data line is pulled low
        // this method is used by sensirion sensors
        while (I2C_MASTER_IN & I2C_MASTER_SDA) {
            delay_c;
        }
    }

    for (j = 0; j < length; j++) {
        sda_high;
        data = 0;
        i = 0;
        for (; i < 8; ++i) {
            scl_high;
            while (!(I2C_MASTER_IN & I2C_MASTER_SCL)) {
                delay_c;         // wait if slave holds the clk low
            }
            data <<= 1;
            if (I2C_MASTER_IN & I2C_MASTER_SDA) {
                data |= 0x01;
            }
            scl_low;
        }
        *buf++ = data;
        // send ack

        if (j != length - 1) {
            sda_low;
        }

        if ((j == length - 1) && (options & I2C_LAST_NAK)) {
            // send nack
            sda_high;
            delay_c;
            scl_high;
            delay_c;
            scl_low;
        } else {
            // send ack
            scl_high;
            delay_c;
            scl_low;
        }
    }
    return I2C_ACK; // FIXME ?
}


uint8_t i2cm_transfer(const i2c_package_t * pkg)
{
    uint8_t rv;

    // START
    rv = i2cm_start(pkg->options);
    if (rv != I2C_OK) {
        i2cm_stop(pkg->options);
        return rv;
    }

    if (pkg->options & I2C_READ) {
        // some devices need to write a register address/command before a read
        if (pkg->addr_len) {
            // SLAVE ADDR + W
            rv = i2cm_tx(pkg->slave_addr, I2C_WRITE | pkg->options);
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
            // REGISTER ADDR/COMMAND
            rv = i2cm_tx_buff(pkg->addr, pkg->addr_len, pkg->options);
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
        } else {
            // SLAVE ADDR + R
            rv = i2cm_tx(pkg->slave_addr, pkg->options);
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
        }
        if (pkg->options & I2C_REPEAT_SA_ON_READ) {
            rv = i2cm_start(pkg->options);
            if (rv == I2C_OK) {
                rv = i2cm_tx(pkg->slave_addr, pkg->options);
            }
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
        }
        rv = i2cm_rx(pkg->data, pkg->data_len, pkg->options);
    } else if (pkg->options & I2C_WRITE) {
        // SLAVE ADDR
        rv = i2cm_tx(pkg->slave_addr, pkg->options);
        if (rv != I2C_ACK) {
            i2cm_stop(pkg->options);
            return rv;
        }

        if (pkg->addr_len) {
            rv = i2cm_tx_buff(pkg->addr, pkg->addr_len, pkg->options);
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
        }
        if (pkg->data_len) {
            rv = i2cm_tx_buff(pkg->data, pkg->data_len, pkg->options);
            if (rv != I2C_ACK) {
                i2cm_stop(pkg->options);
                return rv;
            }
        }
    }
    
    i2cm_stop(pkg->options);
    return rv;
}
#endif
