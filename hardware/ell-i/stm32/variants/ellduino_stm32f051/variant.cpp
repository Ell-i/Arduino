
#include "Arduino.h"
#include "enc28j60.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void Peripheral_Init(void);

#ifndef assert
#define assert(x) \
    do { while (!(x)) { GPIOC->ODR ^= GPIO_ODR_8; for (volatile int i = 0; i < 100; i++); } } while (0)
#endif

/*
 * The following derived from STM Copyrighted work.
 * If you remove that or modify it enough, please consider
 * removing the SMT copyright notice from the top of this file.
 */

extern __I uint8_t APBAHBPrescTable[16];

uint32_t
RCC_GetHCLKFreq(void) {
    /* Make sure we are running from PLL from HSI */
    assert((RCC->CFGR & RCC_CFGR_SWS)    == RCC_CFGR_SWS_PLL);
    assert((RCC->CFGR & RCC_CFGR_PLLSRC) == RCC_CFGR_PLLSRC_HSI_Div2);

    /* Get PLL clock multiplication factor ----------------------*/
    const uint32_t pllmull   = ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18) + 2;

    /* HSI oscillator clock divided by 2 as PLL clock entry */
    const uint32_t sysclk = (HSI_VALUE >> 1) * pllmull;

    /* HCLK clock frequency */
    return sysclk;
}

/*
 * End of STM derived work
 */

#if 0
const uint8_t mac_address[ETH_ADDRESS_LEN] = { 0xae, 0x68, 0x2e, 0xe2, 0xbf, 0xe0 };
#else
const uint8_t mac_address[ETH_ADDRESS_LEN] = { 0, 0, 0, 0, 0, 0, };
#endif

void init(void) {
#if DEBUG
    /* Start GPIO C early, and make 8, 9, 10 and 11 outputs */
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
    GPIOC->ODR   |= GPIO_ODR_8;
#endif
    /*
      In STM32F, SystemInit and __libc_init_array are called from the Reset_Handler,
      before main() gets called, and there is no need to call them again from here.
    */

    /* SysTick end of count event each 1ms */
    //SysTick_Config(SystemCoreClock / 1000); /* CMSIS */
#if DEBUG
    GPIOC->ODR |= GPIO_ODR_9;
#endif
    SysTick_Config(RCC_GetHCLKFreq() / 1000); /* CMSIS */

    Peripheral_Init();

#if DEBUG
    GPIOC->ODR &= ~GPIO_ODR_9;
#endif

    assert((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == SysTick_CTRL_ENABLE_Msk);

    __enable_irq();

    GPIOC->BSRR |= GPIO_BSRR_BS_11; // XXX Abstract into a function/macro
    DEBUG_SET_LED1(1);
    enc_init(mac_address);
    DEBUG_SET_LED1(0);
}

/*
 * XXX: Why does not this come linked from cortex_handlers.c?
 */
#include <Reset.h>

extern "C" { volatile uint32_t millisecondCount; }

extern int sysTickHook(void);
void SysTick_Handler(void);
void SysTick_Handler(void)
{
    // Increment the millisecond count first.
    // Incrementing first avoids race conditions
    millisecondCount++;

#if DEBUG
    GPIOC->ODR ^= GPIO_ODR_8;
#endif

    if (sysTickHook())
        return;

    tickReset();
}

// ----------------------------------------------------------------------------
/*
 * USART objects
 */

RingBuffer rx_buffer;
RingBuffer rx_buffer1;

void serialEvent() __attribute__((weak));
void serialEvent() { }

void USART1_IRQHandler(void);
void USART1_IRQHandler(void) {
    //Serial.IrqHandler();
}

void serialEvent1() __attribute__((weak));
void serialEvent1() { }

void USART2_IRQHandler(void);
void USART2_IRQHandler(void) {
    //Serial1.IrqHandler();
}

#ifdef __cplusplus
}
#endif
