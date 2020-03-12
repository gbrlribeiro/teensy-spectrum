#include "led.h"


int main(){
    led_init();
    led_blink();
    while(1);
    return 0;
}
