
// hardware UART implementation that uses P4.4 as RXI and P4.5 for TXO

//******************************************************************************
//   MSP430F550x Demo - USCI_A0, Ultra-Low Pwr UART 9600 Echo ISR, 32kHz ACLK
//
//   Description: Echo a received character, RX ISR used. Normal mode is LPM3,
//   USCI_A0 RX interrupt triggers TX Echo.
//   ACLK = 32768Hz crystal, MCLK = SMCLK = DCO ~1.045MHz
//   Baud rate divider with 32768Hz XTAL @9600 = 32768Hz/9600 = 3.41
//   See User Guide for baud rate divider table
//
//                MSP430F550x
//             -----------------
//        /|\ |              XIN|-
//         |  |                 | 32kHz
//         ---|RST          XOUT|-
//            |                 |
//            |     P3.3/UCA0TXD|------------>
//            |                 | 9600 - 8N1
//            |     P3.4/UCA0RXD|<------------
//
//   D. Archbold
//   Texas Instruments Inc.
//   March 2010
//   Built with CCSv4 and IAR Embedded Workbench Version: 4.21
//******************************************************************************



#include "uart1.h"

void uart1_init(void)
{
    // hardware UART
    P4SEL |= BIT4 + BIT5;       // P4.4,5 = USCI_A0 TXD/RXD
    UCA1CTL1 |= UCSWRST;        // put state machine in reset
    UCA1CTL1 |= UCSSEL_1;       // use ACLK
    UCA1BR0 = 0x03;             // 32kHz/9600=3.41
    UCA1BR1 = 0x00;
    UCA1MCTL = UCBRS_3 + UCBRF_0;       // modulation UCBRSx=3, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;       // initialize USCI state machine
    UCA1IE |= UCRXIE;           // enable USCI_A0 RX interrupt
}

uint16_t uart1_tx_str(char *str, uint16_t size)
{
    uint16_t p = 0;
    while (p < size) {
        while (!(UCA1IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
        UCA1TXBUF = str[p];
        p++;
    }
    return p;
}

__attribute__ ((interrupt(USCI_A1_VECTOR)))
void USCI_A1_ISR(void)
{
    uint16_t iv = UCA1IV;

    enum uart1_tevent ev = 0;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        ev = UART1_EV_RX;
        break;
    case 4:
        ev = UART1_EV_TX;
        break;
    default:
        break;
    }
    uart1_last_event |= ev;
}

