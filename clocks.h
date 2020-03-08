#ifndef CLOCKS_H
#define CLOCKS_H

#include "MK64F12.h"

void clock_init();
uint32_t pll_init(uint8_t prdiv_val, uint8_t vdiv_val);
void wdog_disable();

#endif //CLOCKS_H