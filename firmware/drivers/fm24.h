/*
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
* \addtogroup fm24 ferroelectric random access memory driver
* \brief driver for the fm24 series of FRAM memory chips.
* \author Petre Rodan
* 
* \{
**/

#ifndef __FM24_H__
#define __FM24_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "config.h"

// config.h should either contain a define for CONFIG_FM24V10 
//  or CONFIG_FM24CL64B

#define FM24_BA      0x50       // base address
#define FM24_RSVD    0xF8       // reserved slave address
#define FM24_SLEEP   0x86       // sleep command

#define FM24_MAX_RETRY   4      // how many times to retry read/write opperations

// FM24V10 has 131072 addressable bytes, so uint16_t is not enough

#if defined(CONFIG_FM24V10)
#define FM_LA        0x1FFFF    // last addressable byte
#define MAX_SEG      12

#elif defined(CONFIG_FM24CL64B)
#define FM_LA        0x1FFF     // last addressable byte
#define MAX_SEG      9

#else
// fake values to help compilation in case the project does not use this chip
#define FM_LA        0x1FFF     // last addressable byte
#define MAX_SEG      9

#endif

// sim900 can only send about 1000 bytes at a time
// so the unsent data has to be segmented up
// transmitted packets also include a header which can become 106bytes long
// MAX_SEG must be at least 4
#define MAX_SEG_SIZE  894       // (1000 - 106)

#if MAX_SEG * MAX_SEG_SIZE > FM_LA
#error "invalid segmentation in fm24.h"
#endif

uint8_t fm24_status;
#define FM24_AWAKE  0x1

/**
 * \brief start an multi-byte selective read
 * 
 * this function begins a new I2C multi-byte selective read.
 * 
 * \param usci_base_address MSP430-related register address of the USCI subsystem. 
 *                              can be USCI_B0_BASE - USCI_B1_BASE, EUSCI_B0_BASE - EUSCI_B3_BASE
 * \param data              pointer to pre-allocated buffer where the read data is to be written.
 *                              make sure it's at least data_len in lenght
 * \param addr              FM24 address where the read will start
 * \param data_len          how many bytes to be read
 **/

uint32_t FM24_read(const uint16_t usci_base_addr, uint8_t * data, const uint32_t addr,
                   const uint32_t data_len);

/**
 * \brief start an multi-byte write
 * 
 * this function begins a new I2C multi-byte selective read.
 * 
 * \param usci_base_address MSP430-related register address of the USCI subsystem.
 *                              can be USCI_B0_BASE - USCI_B1_BASE, EUSCI_B0_BASE - EUSCI_B3_BASE
 * \param data              pointer to buffer where the written data is provided
 * \param addr              FM24 address where the write will start
 * \param data_len          how many bytes to be written
 **/

uint32_t FM24_write(const uint16_t usci_base_addr, uint8_t * data, const uint32_t addr,
                    const uint32_t data_len);

uint8_t FM24_sleep(const uint16_t usci_base_addr);

// helpers

struct mem_mgmt_t {
    uint32_t e;                 // first empty address
    uint32_t seg[MAX_SEG + 1];  // [start, end] addresses for the MAX_SEG_SIZE byte segments
    uint8_t seg_num;            // number of allocated segments
};

struct mem_mgmt_t m;

uint32_t FM24_data_len(const uint32_t first, const uint32_t last);

#ifdef __cplusplus
}
#endif

#endif
