
// hardware UART implementation that uses P4.4 as RXI and P4.5 for TXO

#include "uart0.h"

void uart0_init(void)
{

    // get write-access to port mapping registers
    //PMAPPWD = 0x02D52;
    PMAPPWD = PMAPKEY;
    PMAPCTL = PMAPRECFG;
    P4MAP2 = PM_UCA0TXD;
    P4MAP3 = PM_UCA0RXD;
    PMAPPWD = 0;
    // hardware UART
    P4SEL |= BIT2 + BIT3;       // P4.4,5 = USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSWRST;        // put state machine in reset
    UCA0CTL1 |= UCSSEL_1;       // use ACLK
    UCA0BR0 = 0x03;             // 32kHz/9600=3.41
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS_3 + UCBRF_0;       // modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;       // initialize USCI state machine
    UCA0IE |= UCRXIE;           // enable USCI_A0 RX interrupt
}

uint16_t uart0_tx_str(char *str, uint16_t size)
{
    uint16_t p = 0;
    while (p < size) {
        while (!(UCA0IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
        UCA0TXBUF = str[p];
        p++;
    }
    return p;
}

__attribute__ ((interrupt(USCI_A0_VECTOR)))
void USCI_A0_ISR(void)
{
    uint16_t iv = UCA0IV;

    enum uart0_tevent ev = 0;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        ev = UART0_EV_RX;
        uart0_rx_buf = UCA0RXBUF;
        _BIC_SR_IRQ(LPM3_bits);
        break;
    case 4:
        ev = UART0_EV_TX;
        break;
    default:
        break;
    }
    uart0_last_event |= ev;
}

