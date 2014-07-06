#ifndef __UART1_H__
#define __UART1_H__

#include "proj.h"

enum uart1_tevent {
    UART1_EV_RX = BIT0,
    UART1_EV_TX = BIT1
};

#define UART1_RXBUF_SZ     8

volatile char uart1_rx_buf[UART1_RXBUF_SZ];
volatile uint8_t uart1_p;
volatile uint8_t uart1_rx_enable;
uint8_t uart1_rx_err;

void uart1_init();
uint16_t uart1_tx_str(char *str, const uint16_t size);

volatile enum uart1_tevent uart1_last_event;

#endif
