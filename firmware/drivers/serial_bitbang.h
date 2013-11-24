#ifndef __SERIAL_BITBANG_
#define __SERIAL_BITBANG_

#include "proj.h"

#define sda_high    I2C_MASTER_DIR &= ~I2C_MASTER_SDA
#define sda_low     I2C_MASTER_DIR |= I2C_MASTER_SDA
#define scl_high    I2C_MASTER_DIR &= ~I2C_MASTER_SCL
#define scl_low     I2C_MASTER_DIR |= I2C_MASTER_SCL
//#define scl_f_high  I2C_MASTER_OUT |= I2C_MASTER_SCL
//#define scl_f_low   I2C_MASTER_OUT &= ~I2C_MASTER_SCL

// i2cm_tx options
#define I2C_READ                0x1
#define I2C_WRITE               0x2
#define I2C_NO_ADDR_SHIFT       0x4

// i2cm_rx options
// sometimes the master has to wait for the slave to release SDA
// some sensors pull SDA low to signal that data is ready
#define I2C_SDA_WAIT            0x1

// i2cm_start, i2cm_tx error levels
#define I2C_OK                  0x0
#define I2C_ACK                 0x10
#define I2C_NAK                 0x20
#define I2C_MISSING_SCL_PULLUP  0x40
#define I2C_MISSING_SDA_PULLUP  0x80

#define delay_s     { _NOP(); }
#define delay_c     { _NOP(); _NOP(); _NOP(); }

/*
// define the SDA/SCL ports
#define I2C_MASTER_DIR P1DIR
#define I2C_MASTER_OUT P1OUT
#define I2C_MASTER_IN  P1IN
#define IC2_MASTER_SCL BIT2
#define I2C_MASTER_SDA BIT3
*/

// send one byte
uint8_t i2cm_tx(const uint8_t slave_address, const uint8_t options);

// read 'length' number of bytes and place them into buf
uint8_t i2cm_rx(uint8_t * buf, const uint16_t length, const uint8_t options);

// read 'length' bytes into 'data' from 'address'
uint8_t i2cm_rxfrom(const uint8_t slave_address, uint8_t * data,
                    const uint16_t length);

// send a 'start' sequence
uint8_t i2cm_start(void);

// send a 'stop' sequence
void i2cm_stop(void);

#endif
