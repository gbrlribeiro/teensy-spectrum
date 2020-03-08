#include "led.h"

/*Sets PORT_C 5 as a GPIO then as an output pin*/
void led_init(){
    PORTC->PCR[5] |= PORT_PCR_MUX(0b001);
    GPIOC->PDDR = (1<<5);
}

/*Sets or clears the port where LED is connected */
void led_state(enum state S){
    switch(S){
        case LED_ON:
            GPIOC->PSOR = (1<<5);
            break;
        case LED_OFF:
            GPIOC->PCOR = (1<<5);
            break;
    }
}

void led_blink(){
        while(1){
        led_state(LED_OFF);
        delay(10);
        led_state(LED_ON);
    }
}

void delay(uint32_t time){
    for(uint32_t i = 0; i<16000*(time/5) ; i++){
        asm volatile("nop");
    }
}