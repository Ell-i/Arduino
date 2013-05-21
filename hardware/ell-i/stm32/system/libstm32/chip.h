/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

/* XXX Temp definitions to compile */

#ifndef ELLDUINO_STM32F051_CHIP_H
#define ELLDUINO_STM32F051_CHIP_H 1

#include <stm32f0xx.h>
#include <stm32f0xx_extra.h>

/* Arduino.h */

typedef int Pio, EPioType;

/* variant.h */

typedef struct stm32_pinDescription {
    __IO uint32_t *const pin_port_moder; /* GPIO mode register */
    __IO uint32_t *const pin_port_bsrr;  /* GPIO bit set/reset register */
    __IO uint16_t *const pin_port_brr;   /* GPIO bit reset register */
    __IO uint16_t *const pin_port_input; /* GPIO input register */
    TIM_TypeDef   *const pin_pwm_timer;  /* PWM Timer */
    __IO uint32_t *const pin_pwm_ccr;    /* PWM Timer counter; must be 32 bit pointer due to Timer 2 */
    const uint16_t pin_mask;
    const uint8_t  pin_number;
    const uint8_t  pin_adc_channel;
} stm32_pinDescription_t;

/* USARTClass.h */

typedef USART_TypeDef Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
