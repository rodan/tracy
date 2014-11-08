
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/sim900.h"
#include "drivers/uart1.h"
#include "drivers/timer_a0.h"
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
            "\r\n --- tracy build #%d\r\n available commands:\r\n", BUILD);
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1m?\e[0m - show menu\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1m!\e[0m - gprs power on\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1m)\e[0m - gprs power off\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1m$\e[0m - gprs initial setup\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1m@\e[0m - gprs start default task\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1ms\e[0m - store packet\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1mt\e[0m - memtest\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, STR_LEN, " \e[33;1mr\e[0m - read all mem\r\n" );
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
    } else if (f == '@') {
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
        display_memtest(FM_LA - 3, FM_LA + 5, TEST_FF);
    } else if (f == '$') {
        uart1_init(2400);
        sim900.cmd = CMD_FIRST_PWRON;
        sim900.next_state = SIM900_IDLE;
        timer_a0_delay_noblk_ccr2(SM_STEP_DELAY);
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

