
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/sim900.h"
#include "version.h"
#include "qa.h"

void display_memtest(const uint32_t start_addr, const uint32_t stop_addr, fm24_test_t test)
{
    uint32_t el;
    uint32_t rows_tested;

    snprintf(str_temp, STR_LEN, " \e[36;1m*\e[0m testing %lx - %lx with pattern #%d\t", start_addr, stop_addr, test);
    uart0_tx_str(str_temp, strlen(str_temp));

    el = fm24_memtest(start_addr, stop_addr, test, &rows_tested);
    if (el == 0) { 
        snprintf(str_temp, STR_LEN, "%lu bytes tested \e[32;1mok\e[0m\r\n", rows_tested * 8);
    } else {
        snprintf(str_temp, STR_LEN, "%lu bytes tested with \e[31;1m%lu errors\e[0m\r\n", rows_tested * 8, el );
    }
    uart0_tx_str(str_temp, strlen(str_temp));
}

void display_menu(void)
{
    snprintf(str_temp, STR_LEN,
            "\r\n --- tracy build #%d -\r\n", BUILD);
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " gprs power on \e[33;1m!\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " gprs power off \e[33;1m)\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " gprs start default task \e[33;1m?\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " store packet \e[33;1ms\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " memtest \e[33;1mt\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " read all mem \e[33;1mr\e[0m\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));
}

void parse_user_input(void)
{
    char f = uart0_rx_buf[0];
    uint16_t i;
    uint8_t j;
    uint8_t row[8];

    if (f == '?') {
        display_menu();
    } else if (f == '"') {
        sim900_exec_default_task();
    } else if (f == '!') {
        sim900_start();
    } else if (f == ')') {
        sim900_halt();
    } else if (f == 's') {
        adc_read();
        store_pkt();
    } else if (f == 't') {
        display_memtest(0, FM_LA, TEST_00);
        display_memtest(0, FM_LA, TEST_FF);
        display_memtest(0, FM_LA, TEST_AA);
        uart0_tx_str(" * roll over test\r\n", 19);
        display_memtest(FM_LA - 4, FM_LA + 3, TEST_FF);
    } else if (f == 'r') {
        for (i=0;i<(FM_LA+1)/8;i++) {
            fm24_read_from(row, i * 8, 8);
            for (j=0; j<8; j++) {
                uart0_tx_str((char *)row + j, 1);
            }
        }
    } else {
        sim900_tx_str((char *)uart0_rx_buf, uart0_p);
        sim900_tx_str("\r", 1);
    }
}

