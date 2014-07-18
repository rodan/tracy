
#include "uart1.h"
#include "timer_a0.h"
#include "sim900.h"

void uart1_init(uint16_t speed)
{
    UCA1CTL1 |= UCSWRST;        // put state machine in reset
    UCA1CTL1 |= UCSSEL_1;       // use ACLK

    if (speed == 9600) {
        UCA1BR0 = 0x03;
        UCA1BR1 = 0x00;
        UCA1MCTL = UCBRS_3 + UCBRF_0;       // modulation UCBRSx=3, UCBRFx=0
    } else if (speed == 2400) {
        UCA1BR0 = 0x0D;
        UCA1BR1 = 0x00;
        UCA1MCTL |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=6, UCBRFx=0
    }

    UCA1CTL1 &= ~UCSWRST;       // initialize USCI state machine
    UCA1IE |= UCRXIE;           // enable USCI_A0 RX interrupt
    uart1_p = 0;
    uart1_rx_enable = true;
}

uint16_t uart1_tx_str(char *str, const uint16_t size)
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
    register char rx;
    enum uart1_tevent ev = 0;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        rx = UCA1RXBUF;
        if (uart1_rx_enable && (uart1_p < UART1_RXBUF_SZ-2)) {
                if (uart1_p == 0) {
                    sim900.console = TTY_RX_PENDING;
                    // set up timer that will end the buffer
                    timer_a0_delay_noblk_ccr3(RXBUF_TMOUT);
                }
                uart1_rx_buf[uart1_p] = rx;
                uart1_p++;
        } else {
            // send partial buffer
        }
        break;
    case 4:
        ev = UART1_EV_TX;
        break;
    default:
        break;
    }
    uart1_last_event |= ev;
}
