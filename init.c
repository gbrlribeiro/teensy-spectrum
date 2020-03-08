#include <stdint.h>

extern uint32_t _start_bss, _end_bss;


void init_bss(){
    //Initialize bss region as 0;
    for(uint32_t *bss_ptr = &_start_bss; bss_ptr < &_end_bss;){
        *bss_ptr++ = 0;
    }
}



