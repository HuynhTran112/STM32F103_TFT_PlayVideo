#include "SysClockConfig.h"
void SysClockConfig(void)
{
    /*Enable HSE (8MHz)*/
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    /*72MHz -> Flash latency = 2*/
    FLASH->ACR |= FLASH_ACR_LATENCY_2;
    /*PLL = 8MHz * 9 = 72MHz*/
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;
    /*AHB = 72MHz*/
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    /*APB1 = 36MHz*/
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    /*APB2 = 72MHz*/
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
    /*Enable PLL*/
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    /*Select PLL as system clock*/
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
