
#include "Arduino.h"
#include "enc28j60.h"
#include "debug.h"

extern "C" {

#include "contiki.h"
#include "sys/etimer.h"

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

/*
 * Usually called from main.cpp in cores/arduino
 */

void init(void) {
#if DEBUG
    /* Start GPIO C early, and make 6 (DEBUG0) and 7 (DEBUG1) outputs */
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;
    DEBUG_SET_LED0(1);
#endif

    /*
      In STM32F, SystemInit and __libc_init_array are called from the Reset_Handler,
      before main() gets called, and there is no need to call them again from here.
    */

    /* SysTick end of count event each 1ms */
    //SysTick_Config(SystemCoreClock / 1000); /* CMSIS */
    DEBUG_SET_LED0(0);
    SysTick_Config(RCC_GetHCLKFreq() / 1000); /* CMSIS */

    DEBUG_SET_LED0(1);
    Peripheral_Init();

    assert((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == SysTick_CTRL_ENABLE_Msk);

    DEBUG_SET_LED0(0);
    __enable_irq();

    GPIOC->BSRR |= GPIO_BSRR_BS_11; // XXX Abstract into a function/macro
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

    if (etimer_pending()) {
        etimer_request_poll();
    }

    // TimingDelay_Decrement();

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
    Serial1.irqHandler();
}

void serialEvent1() __attribute__((weak));
void serialEvent1() { }

void USART2_IRQHandler(void);
void USART2_IRQHandler(void) {
    Serial.irqHandler();
}

} // extern "C"
