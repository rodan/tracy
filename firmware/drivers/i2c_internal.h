/*
* Copyright (c) 2013, Alexander I. Mykyta
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
* \addtogroup MOD_I2C
* \{
**/

/**
* \file
* \brief Internal include for \ref MOD_I2C. Abstracts register names between MSP430 devices
* \author Alex Mykyta
**/

#ifndef _I2C_INTERNAL_H_
#define _I2C_INTERNAL_H_

#include "i2c_config.h"

#if I2C_CLK_SRC > 2
#error "Invalid I2C_CLK_SRC in i2c_config.h"
#elif I2C_CLK_SRC < 1
#error "Invalid I2C_CLK_SRC in i2c_config.h"
#endif

#if I2C_CLK_DIV < 4
#error "Invalid I2C_CLK_DIV in i2c_config.h"
#endif

#if I2C_USE_DEV == 0
#if defined(__MSP430_HAS_USCI_B0__)
#define I2C_CTL0        UCB0CTL0
#define I2C_CTL1        UCB0CTL1
#define I2C_BR          UCB0BRW
#define I2C_STAT        UCB0STAT
#define I2C_RXBUF       UCB0RXBUF
#define I2C_TXBUF       UCB0TXBUF
#define I2C_OA          UCB0I2COA
#define I2C_SA          UCB0I2CSA
#define I2C_ICTL        UCB0ICTL
#define I2C_IE          UCB0IE
#define I2C_IFG         UCB0IFG
#define I2C_IV          UCB0IV
#define I2C_ISR_VECTOR  USCI_B0_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 1
#if defined(__MSP430_HAS_USCI_B1__)
#define I2C_CTL0        UCB1CTL0
#define I2C_CTL1        UCB1CTL1
#define I2C_BR          UCB1BRW
#define I2C_STAT        UCB1STAT
#define I2C_RXBUF       UCB1RXBUF
#define I2C_TXBUF       UCB1TXBUF
#define I2C_OA          UCB1I2COA
#define I2C_SA          UCB1I2CSA
#define I2C_ICTL        UCB1ICTL
#define I2C_IE          UCB1IE
#define I2C_IFG         UCB1IFG
#define I2C_IV          UCB1IV
#define I2C_ISR_VECTOR  USCI_B1_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 2
#if defined(__MSP430_HAS_USCI_B2__)
#define I2C_CTL0        UCB2CTL0
#define I2C_CTL1        UCB2CTL1
#define I2C_BR          UCB2BRW
#define I2C_STAT        UCB2STAT
#define I2C_RXBUF       UCB2RXBUF
#define I2C_TXBUF       UCB2TXBUF
#define I2C_OA          UCB2I2COA
#define I2C_SA          UCB2I2CSA
#define I2C_ICTL        UCB2ICTL
#define I2C_IE          UCB2IE
#define I2C_IFG         UCB2IFG
#define I2C_IV          UCB2IV
#define I2C_ISR_VECTOR  USCI_B2_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 3
#if defined(__MSP430_HAS_USCI_B3__)
#define I2C_CTL0        UCB3CTL0
#define I2C_CTL1        UCB3CTL1
#define I2C_BR          UCB3BRW
#define I2C_STAT        UCB3STAT
#define I2C_RXBUF       UCB3RXBUF
#define I2C_TXBUF       UCB3TXBUF
#define I2C_OA          UCB3I2COA
#define I2C_SA          UCB3I2CSA
#define I2C_ICTL        UCB3ICTL
#define I2C_IE          UCB3IE
#define I2C_IFG         UCB3IFG
#define I2C_IV          UCB3IV
#define I2C_ISR_VECTOR  USCI_B3_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 4
#if defined(__MSP430_HAS_EUSCI_B0__)
#define I2C_CTL0        UCB0CTLW0
#define I2C_CTL1        UCB0CTLW0
#define I2C_CTLW1       UCB0CTLW1
#define I2C_BR          UCB0BRW
#define I2C_STAT        UCB0STATW
#define I2C_RXBUF       UCB0RXBUF
#define I2C_TXBUF       UCB0TXBUF
#define I2C_OA          UCB0I2COA0
#define I2C_SA          UCB0I2CSA
#define I2C_IE          UCB0IE
#define I2C_IFG         UCB0IFG
#define I2C_IV          UCB0IV
#define I2C_ISR_VECTOR  EUSCI_B0_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif


#elif I2C_USE_DEV == 5
#if defined(__MSP430_HAS_EUSCI_B1__)
#define I2C_CTL0        UCB1CTLW0
#define I2C_CTL1        UCB1CTLW0
#define I2C_CTLW1       UCB1CTLW1
#define I2C_BR          UCB1BRW
#define I2C_STAT        UCB1STATW
#define I2C_RXBUF       UCB1RXBUF
#define I2C_TXBUF       UCB1TXBUF
#define I2C_OA          UCB1I2COA0
#define I2C_SA          UCB1I2CSA
#define I2C_IE          UCB1IE
#define I2C_IFG         UCB1IFG
#define I2C_IV          UCB1IV
#define I2C_ISR_VECTOR  EUSCI_B1_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 6
#if defined(__MSP430_HAS_EUSCI_B2__)
#define I2C_CTL0        UCB2CTLW0
#define I2C_CTL1        UCB2CTLW0
#define I2C_CTLW1       UCB2CTLW1
#define I2C_BR          UCB2BRW
#define I2C_STAT        UCB2STATW
#define I2C_RXBUF       UCB2RXBUF
#define I2C_TXBUF       UCB2TXBUF
#define I2C_OA          UCB2I2COA0
#define I2C_SA          UCB2I2CSA
#define I2C_IE          UCB2IE
#define I2C_IFG         UCB2IFG
#define I2C_IV          UCB2IV
#define I2C_ISR_VECTOR  EUSCI_B2_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#elif I2C_USE_DEV == 7
#if defined(__MSP430_HAS_EUSCI_B3__)
#define I2C_CTL0        UCB3CTLW0
#define I2C_CTL1        UCB3CTLW0
#define I2C_CTLW1       UCB3CTLW1
#define I2C_BR          UCB3BRW
#define I2C_STAT        UCB3STATW
#define I2C_RXBUF       UCB3RXBUF
#define I2C_TXBUF       UCB3TXBUF
#define I2C_OA          UCB3I2COA0
#define I2C_SA          UCB3I2CSA
#define I2C_IE          UCB3IE
#define I2C_IFG         UCB3IFG
#define I2C_IV          UCB3IV
#define I2C_ISR_VECTOR  EUSCI_B3_VECTOR
#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#else
#error "Invalid I2C_USE_DEV in i2c_config.h"
#endif

#endif

///\}
