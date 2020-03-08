  .syntax unified
  .global _start
  .thumb

_start:
    ldr sp, =_top_stack
    bl init_bss
    bl wdog_disable
    bl main

_exit:
    bl _exit
