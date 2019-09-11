/*
* Copyright (c) 2013, Alexander I. Mykyta
* Copyright (c) 2019, Petre Rodan
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer. 
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
* \addtogroup MOD_I2C I2C Bus
* \brief Application-level I2C master driver.
* \author Alex Mykyta 
* \author Petre Rodan
* 
* \{
**/

/**
* \file
* \brief Include file for \ref MOD_I2C
* \author Alex Mykyta 
* \author Petre Rodan
**/

#ifndef _I2C_H_
#define _I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

// option flags
#define I2C_READ                0x1
#define I2C_WRITE               0x2
// sometimes the master has to wait for the slave to release SDA
// some sensors pull SDA low to signal that data is ready
#define I2C_SDA_WAIT            0x4
// some devices want the last read byte to not be ACKed
#define I2C_LAST_NAK            0x8
#define I2C_NO_ADDR_SHIFT       0x10
#define I2C_REPEAT_SA_ON_READ   0x20

// special start/stop seq needed by sensirion SHT sensors
#define I2C_SHT_INIT            0x40

    typedef struct {
        uint8_t slave_addr;     ///< chip address of slave device
        uint8_t *addr;          ///< register/command payload
        uint16_t addr_len;      ///< number of addr bytes to use
        uint8_t *data;          ///< pointer to data transfer buffer
        uint16_t data_len;      ///< number of bytes to transfer
        uint8_t options;        ///< see above the possible option flags
    } i2c_package_t;

    typedef enum {
        I2C_IDLE,               ///< bus is idle. ready for new transfer.
        I2C_BUSY,               ///< a transfer is in progress.
        I2C_FAILED              ///< previous transfer failed. ready for new transfer.
    } i2c_status_t;

/**
 * \brief Start an I2C transfer
 * 
 * This function begins a new I2C transaction as described by the \c pkg struct. This function
 * is nonblocking and returns immediately after the transfer is started. The status of the transfer 
 * can be polled using the i2c_transfer_status() function. Alternatively, a \c callback function can
 * be executed when the transfer completes.
 * 
 * \note Global interrupts must be enabled.
 * 
 * \param base_address  MSP430-related register address of the USCI subsystem. can be USCI_B0_BASE - USCI_B1_BASE, EUSCI_B0_BASE - EUSCI_B3_BASE
 * \param pkg           Pointer to a package struct that describes the transfer operation.
 * \param callback      Optional pointer to a callback function to execute once the transfer completes
 *                      or fails. A NULL pointer disables the callback.
 **/
    void i2c_transfer_start(const uint16_t base_address, const i2c_package_t * pkg,
                            void (*callback) (i2c_status_t result));

/**
 * \brief Get the status of the I2C module
 * \return status of the bus.
 **/
    i2c_status_t i2c_transfer_status(void);

    void i2c_irq_init(uint16_t usci_base_address);

#ifdef __cplusplus
}
#endif
#endif
///\}
