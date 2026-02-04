#include "delay.h"
void delayuS(int us)
{
    int i;
    /*72MHz/1us = 72*/
    SysTick->LOAD = 72 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;
    /*wait for 1us and reset*/
    for (i = 0; i < us; i++) {
        while (!(SysTick->CTRL & 0x10000));
    }
    SysTick->CTRL = 0;
}

void delay(int ms)
{
    int i;
    /*72MHz/1ms = 72000*/
    SysTick->LOAD = 72000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;
    /*wait for 1ms and reset*/
    for (i = 0; i < ms; i++) {
        while (!(SysTick->CTRL & 0x10000));
    }
    SysTick->CTRL = 0;
}
