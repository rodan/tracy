#ifndef __UART0_H__
#define __UART0_H__

#include "proj.h"

enum uart0_tevent {
    UART0_EV_RX = BIT0,
    UART0_EV_TX = BIT1
};

void uart0_init();
uint16_t uart0_tx_str(char *str, uint16_t size);

volatile enum uart0_tevent uart0_last_event;

#endif
