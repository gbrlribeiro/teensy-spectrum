#include "led.h"
#include "clocks.h"


int main(){
    clock_init();
    led_init();
    led_blink();
    return 0;
}