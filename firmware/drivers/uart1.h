#ifndef __UART1_H__
#define __UART1_H__

#include "proj.h"

enum uart1_tevent {
    UART1_EV_RX = BIT0,
    UART1_EV_TX = BIT1
};

void uart1_init();
uint16_t uart1_tx_str(char *str, uint16_t size);

volatile enum uart1_tevent uart1_last_event;

#endif
