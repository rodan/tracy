
#include "driverlib.h"
#include "config.h"
#include "i2c.h"

typedef enum {
    SM_SEND_ADDR,
    SM_WRITE_DATA,
    SM_SEND_RESTART,
    SM_READ_DATA,
    SM_DONE
} i2c_state_t;

volatile static struct {
    i2c_package_t *pkg;
    uint16_t idx;
    void (*callback) (i2c_status_t result);
    i2c_status_t status;
    i2c_state_t next_state;
} transfer;

#ifdef IRQ_I2C
#include "i2c_internal.h"

//////////////////////////////////////////////////
// interrupt controlled i2c implementation
// needs a configured i2c_config.h in the source dir
// see the i2c_config.TEMPLATE.h file for guidance


void i2c_irq_init(const uint16_t usci_base_addr)
{
    // UCBxCTLW0 and UCBxBRW must be setup externally
    I2C_CTL1 &= ~UCSWRST;       // Clear reset
    //EUSCI_B_I2C_enable(usci_base_addr);
    transfer.status = I2C_IDLE;
}

void i2c_transfer_start(const uint16_t base_addr, const i2c_package_t * pkg,
                        void (*callback) (i2c_status_t result))
{
    if (i2c_transfer_status() == I2C_BUSY) {
        return;
    }

    transfer.pkg = (i2c_package_t *) pkg;
    transfer.idx = 0;
    transfer.callback = callback;
    transfer.status = I2C_BUSY;

    if (pkg->addr_len != 0) {
        // if i2c also need to send an adress/command between the slave 
        // addr and the actual read/write of data
        transfer.next_state = SM_SEND_ADDR;
        I2C_IFG = 0;
        I2C_IE = UCNACKIE | UCTXIE | UCRXIE;
        I2C_SA = pkg->slave_addr;
        I2C_CTL1 |= UCTR;           // set to transmitter mode
        I2C_CTL1 |= UCTXSTT;        // start condition (send slave address)
        __bis_SR_register(LPM0_bits + GIE);
    }  else if (pkg->data_len != 0) {

        if (pkg->options & I2C_READ) {
            transfer.next_state = SM_SEND_RESTART;
            I2C_IFG = 0;
            I2C_IE = UCNACKIE | UCRXIE;
            I2C_SA = pkg->slave_addr;
            I2C_CTL1 &= ~UCTR;  // set to receiver mode

            while (I2C_CTL1 & UCTXSTP) {}        // Ensure stop condition got sent
            I2C_CTL1 |= UCTXSTT;        // start condition
            if (transfer.pkg->data_len == 1) {
                // wait for STT bit to drop
                while (I2C_CTL1 & UCTXSTT) {}
                I2C_CTL1 |= UCTXSTP;        // schedule stop condition
            }
            // update next state
            transfer.next_state = SM_READ_DATA;
            __bis_SR_register(LPM0_bits + GIE);
        } else if (pkg->options & I2C_WRITE) {
            transfer.next_state = SM_WRITE_DATA;
            I2C_IFG = 0;
            I2C_IE = UCNACKIE | UCTXIE | UCRXIE;
            I2C_SA = pkg->slave_addr;
            I2C_CTL1 |= UCTR;           // set to transmitter mode
            I2C_CTL1 |= UCTXSTT;        // start condition (send slave address)
            __bis_SR_register(LPM0_bits + GIE);
        }
        
    } else {
        transfer.next_state = SM_DONE;
    }
}

i2c_status_t i2c_transfer_status(void)
{
    i2c_status_t status;

    //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    status = transfer.status;
    //}

    return (status);
}


__attribute__ ((interrupt(I2C_ISR_VECTOR)))
void USCI_BX_ISR(void)
{
    switch (HWREG16(EUSCI_BASE_ADDR + OFS_UCBxIV)) {

    case USCI_NONE:
        break;
    case USCI_I2C_UCALIFG:
        // Arbitration lost interrupt
        break;                  // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:   // Vector 4: NACKIFG
        I2C_IFG = 0;
        I2C_IE = 0;
        transfer.status = I2C_FAILED;
        I2C_CTL1 |= UCTXSTP;    // set stop condition
        if (transfer.callback) {
            transfer.callback(I2C_FAILED);
        }
        __bic_SR_register_on_exit(LPM0_bits);
        return;
        break;
    case USCI_I2C_UCSTTIFG:
        // START condition detected interrupt, own address detected on the bus
        break;
    case USCI_I2C_UCSTPIFG:
        // STOP condition detected interrupt
        break;
#ifdef __MSP430_HAS_EUSCI_Bx__
    case USCI_I2C_UCRXIFG3:
        // data RX
        break;
    case USCI_I2C_UCTXIFG3:
        // data TX
        break;
    case USCI_I2C_UCRXIFG2:
        // data RX
        break;
    case USCI_I2C_UCTXIFG2:
        // data TX
        break;
    case USCI_I2C_UCRXIFG1:
        // data RX
        break;
    case USCI_I2C_UCTXIFG1:
        // data TX
        break;
    case USCI_I2C_UCRXIFG0:
        // data RX
        break;
    case USCI_I2C_UCTXIFG0:
        // data TX
        break;
    case USCI_I2C_UCBCNTIFG:
        // byte counter interrupt
        break;
    case USCI_I2C_UCCLTOIFG:
        // clock low time-out
        break;
    case USCI_I2C_UCBIT9IFG:
        // interrupt forthe ninth clock cycle of a byte of data
        break;
#endif
    default:
        break;
    }

    I2C_IFG = 0;
    
    switch (transfer.next_state) {
    case SM_SEND_ADDR:         // (TX optional register/command)
        I2C_TXBUF = transfer.pkg->addr[transfer.idx];
        transfer.idx++;
        if (transfer.idx == transfer.pkg->addr_len) {
            if (transfer.pkg->data_len != 0) {
                transfer.idx = 0;
                if (transfer.pkg->options & I2C_READ) {
                    transfer.next_state = SM_SEND_RESTART;
                } else {
                    transfer.next_state = SM_WRITE_DATA;
                    I2C_CTL1 |= UCTR;   // set to transmitter mode
                }
            } else {
                transfer.next_state = SM_DONE;
            }
        } // else { transfer.next_state remains SM_SEND_ADDR, so we end up in this case again }
        break;
    case SM_WRITE_DATA:
        I2C_TXBUF = transfer.pkg->data[transfer.idx];
        transfer.idx++;
        if (transfer.idx == transfer.pkg->data_len) {
            // that was the last data byte to send.
            // update next state
            transfer.next_state = SM_DONE;
        }
        break;
    case SM_SEND_RESTART:
        I2C_CTL1 &= ~UCTR;      // Set to receiver mode
        I2C_CTL1 |= UCTXSTT;    // write (re)start condition

        if (transfer.pkg->data_len == 1) {
            // wait for STT bit to drop
            while (I2C_CTL1 & UCTXSTT) {};
            I2C_CTL1 |= UCTXSTP;        // schedule stop condition
        }
        // update next state
        transfer.next_state = SM_READ_DATA;
        break;
    case SM_READ_DATA:
        transfer.pkg->data[transfer.idx] = I2C_RXBUF;
        transfer.idx++;
        if (transfer.idx == transfer.pkg->data_len - 1) {
            // next incoming byte is the last one.
            I2C_CTL1 |= UCTXSTP;        // schedule stop condition
            break;
        } else if (transfer.idx < transfer.pkg->data_len) {
            // still more to recv.
            break;
        }
        // otherwise, that was the last data byte to recv.
        // fall through
    case SM_DONE:
        I2C_IE = 0;
        //if (transfer.pkg->read == false) {
        if (transfer.pkg->options & I2C_WRITE) {
            // If finished a write, schedule a stop condition
            I2C_CTL1 |= UCTXSTP;
        }
        transfer.status = I2C_IDLE;
        if (transfer.callback) {
            transfer.callback(I2C_IDLE);
        }
        __bic_SR_register_on_exit(LPM0_bits);
        break;
    }
}

#else

#ifdef __MSP430_HAS_EUSCI_Bx__

//////////////////////////////////////////////////
// blocking i2c implementation
// the EUSCI_BASE_ADDR is sent over via the uint16_t base_addr argument
//

void i2c_transfer_start(const uint16_t base_addr, const i2c_package_t * pkg,
                        void (*callback) (i2c_status_t result))
{
    uint8_t i;

    if (pkg->options & I2C_READ) {
        // some devices need to write a register address/command before a read
        if (pkg->addr_len) {
            EUSCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
            EUSCI_B_I2C_setMode(base_addr, EUSCI_B_I2C_TRANSMIT_MODE);
            EUSCI_B_I2C_enable(base_addr);
            // send START, slave address
            EUSCI_B_I2C_masterSendStart(base_addr);

            for (i = 0; i < pkg->addr_len; i++) {
                if (i == 0) {
                    EUSCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->addr[0]);
                } else {
                    EUSCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->addr[i]);
                }
            }
            // send STOP after address/command
            EUSCI_B_I2C_masterSendMultiByteStop(base_addr);
        }
        // SLAVE ADDR + R, read into pkg->data pkg->data_len times
        EUSCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
        EUSCI_B_I2C_setMode(base_addr, EUSCI_B_I2C_RECEIVE_MODE);
        EUSCI_B_I2C_enable(base_addr);
        // send slave address
        EUSCI_B_I2C_masterReceiveStart(base_addr);
        for (i = 0; i < pkg->data_len - 1; i++) {
            pkg->data[i] = EUSCI_B_I2C_masterReceiveSingle(base_addr);
            //pkg->data[i] = EUSCI_B_I2C_masterReceiveMultiByteNext(base_addr);
        }
        pkg->data[pkg->data_len - 1] = EUSCI_B_I2C_masterReceiveMultiByteFinish(base_addr);

    } else if (pkg->options & I2C_WRITE) {
        EUSCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
        EUSCI_B_I2C_setMode(base_addr, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_enable(base_addr);
        // send START, slave address
        EUSCI_B_I2C_masterSendStart(base_addr);

        for (i = 0; i < pkg->addr_len; i++) {
            if (i == 0) {
                EUSCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->addr[0]);
            } else {
                EUSCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->addr[i]);
            }
        }

        for (i = 0; i < pkg->data_len; i++) {
            if (i == 0) {
                if (pkg->addr_len) {
                    // we already did a SendMultiByteStart
                    EUSCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->data[0]);
                } else {
                    EUSCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->data[0]);
                }
            } else {
                EUSCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->data[i]);
            }
        }
        // send STOP
        EUSCI_B_I2C_masterSendMultiByteStop(base_addr);
    }
}
#endif

#ifdef __MSP430_HAS_USCI_Bx__

//////////////////////////////////////////////////
// blocking i2c implementation
// the EUSCI_BASE_ADDR is sent over via the uint16_t base_addr argument
//

void i2c_transfer_start(const uint16_t base_addr, const i2c_package_t * pkg,
                        void (*callback) (i2c_status_t result))
{
    uint8_t i;

    if (pkg->options & I2C_READ) {
        // some devices need to write a register address/command before a read
        if (pkg->addr_len) {
            USCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
            USCI_B_I2C_setMode(base_addr, USCI_B_I2C_TRANSMIT_MODE);
            USCI_B_I2C_enable(base_addr);
            // send START, slave address
            USCI_B_I2C_masterSendStart(base_addr);

            for (i = 0; i < pkg->addr_len; i++) {
                if (i == 0) {
                    USCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->addr[0]);
                } else {
                    USCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->addr[i]);
                }
            }
            // send STOP after address/command
            USCI_B_I2C_masterSendMultiByteStop(base_addr);
        }
        // SLAVE ADDR + R, read into pkg->data pkg->data_len times
        USCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
        USCI_B_I2C_setMode(base_addr, USCI_B_I2C_RECEIVE_MODE);
        USCI_B_I2C_enable(base_addr);
        // send slave address
        USCI_B_I2C_masterReceiveMultiByteStart(base_addr);
        for (i = 0; i < pkg->data_len - 1; i++) {
            pkg->data[i] = USCI_B_I2C_masterReceiveSingle(base_addr);
            //pkg->data[i] = USCI_B_I2C_masterReceiveMultiByteNext(base_addr);
        }
        pkg->data[pkg->data_len - 1] = USCI_B_I2C_masterReceiveMultiByteFinish(base_addr);

    } else if (pkg->options & I2C_WRITE) {
        USCI_B_I2C_setSlaveAddress(base_addr, pkg->slave_addr);
        USCI_B_I2C_setMode(base_addr, USCI_B_I2C_TRANSMIT_MODE);
        USCI_B_I2C_enable(base_addr);
        // send START, slave address
        USCI_B_I2C_masterSendStart(base_addr);

        for (i = 0; i < pkg->addr_len; i++) {
            if (i == 0) {
                USCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->addr[0]);
            } else {
                USCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->addr[i]);
            }
        }

        for (i = 0; i < pkg->data_len; i++) {
            if (i == 0) {
                if (pkg->addr_len) {
                    // we already did a SendMultiByteStart
                    USCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->data[0]);
                } else {
                    USCI_B_I2C_masterSendMultiByteStart(base_addr, pkg->data[0]);
                }
            } else {
                USCI_B_I2C_masterSendMultiByteNext(base_addr, pkg->data[i]);
            }
        }
        // send STOP
        USCI_B_I2C_masterSendMultiByteStop(base_addr);
    }
}

#endif

#endif
///\}
