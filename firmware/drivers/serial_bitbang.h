#ifndef __SERIAL_BITBANG_
#define __SERIAL_BITBANG_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"
#include "i2c_config.h"

/*
// define the SDA/SCL ports
// these should be defined in proj.h
#define I2C_MASTER_DIR P1DIR
#define I2C_MASTER_OUT P1OUT
#define I2C_MASTER_IN  P1IN
#define I2C_MASTER_SCL BIT2
#define I2C_MASTER_SDA BIT3
*/

#define sda_high    I2C_MASTER_DIR &= ~I2C_MASTER_SDA
#define sda_low     I2C_MASTER_DIR |= I2C_MASTER_SDA
#define scl_high    I2C_MASTER_DIR &= ~I2C_MASTER_SCL
#define scl_low     I2C_MASTER_DIR |= I2C_MASTER_SCL

// i2cm_start, i2cm_tx error levels
#define I2C_OK                  0x0
#define I2C_ACK                 0x10
#define I2C_NAK                 0x20
#define I2C_MISSING_SCL_PULLUP  0x40
#define I2C_MISSING_SDA_PULLUP  0x80

#define delay_s     { _NOP(); }
#define delay_c     { _NOP(); _NOP(); _NOP(); }

uint8_t i2cm_transfer(const i2c_package_t * pkg);

#ifdef __cplusplus
}
#endif

#endif
