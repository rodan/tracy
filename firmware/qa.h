#ifndef __QA_H__
#define __QA_H__

#include "proj.h"
#include "drivers/fm24_memtest.h"

void display_memtest(const uint16_t usci_base_addr, const uint32_t start_addr, const uint32_t stop_addr, FM24_test_t test);

void display_menu(void);

void parse_user_input(void);

#endif
