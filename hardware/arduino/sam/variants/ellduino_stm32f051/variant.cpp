
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void Peripheral_Init(void);

/*
 * The following derived from STM Copyrighted work.
 * If you remove that or modify it enough, please consider
 * removing the SMT copyright notice from the top of this file.
 */

extern __I uint8_t APBAHBPrescTable[16];

#ifndef assert
#define assert(x) do { while (!x) { GPIOC->ODR ^= GPIO_ODR_11; for (volatile int i = 0; i < 100; i++); } } while (0)
#endif

uint32_t
RCC_GetHCLKFreq() {
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

void init( void ) {
    /*
      In STM32F, SystemInit and __libc_init_array are called from the Reset_Handler,
      before main() gets called, and there is no need to call them again from here.
    */

    /* SysTick end of count event each 1ms */
    //SysTick_Config(SystemCoreClock / 1000); /* CMSIS */
    GPIOC->ODR |= GPIO_ODR_10;
    SysTick_Config(RCC_GetHCLKFreq() / 1000); /* CMSIS */
    GPIOC->ODR &= ~GPIO_ODR_10;

    Peripheral_Init();

    assert((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == SysTick_CTRL_ENABLE_Msk);

    __enable_irq();
}

static volatile uint32_t tickCount;

uint32_t GetTickCount(void) {
    return tickCount;
}

void TimeTick_Increment(void) {
    tickCount++;
}

/*
 * XXX: Why does not this come linked from cortex_handlers.c?
 */

#include <Reset.h>

extern int sysTickHook(void);
void SysTick_Handler(void)
{
	if (sysTickHook())
		return;

	tickReset();

	// Increment tick count each ms
	TimeTick_Increment();
}

#ifdef __cplusplus
}
#endif
