#include "led.h"


/*Turn on the clock for PORT_C */
/*Sets PORT_C 5 as a GPIO then as an output pin*/
void led_init(){
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[5] = PORT_PCR_MUX(0b001);
    GPIOC->PDDR |= (1<<5);
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
        led_state(LED_ON);
        delay(300);
        led_state(LED_OFF);
        delay(300);
    }
}

void delay(uint32_t time){
    for(uint32_t i = 0; i<20000*(time/5) ; i++){
        asm volatile("nop");
    }
}