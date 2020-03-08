#ifndef LED_H
#define LED_H

#include "MK64F12.h"

enum state{LED_ON, LED_OFF};
void led_init();
void led_state(enum state);
void led_blink();
void delay(uint32_t time);

#endif //LED_H1