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

/* wiring_digital.c */

typedef struct stm32_pinDescription {
    __IO uint32_t *pin_port_moder;
    __IO uint32_t *pin_port_bsrr;
    __IO uint16_t *pin_port_brr;
    __IO uint16_t *pin_port_input;
    __IO uint32_t *pin_pwm_ccr; /* Must be 32 bit pointer due to Timer 2 */
    uint16_t pin_mask;
    uint8_t pin_number;
    uint8_t pin_adc_channel;
} stm32_pinDescription_t;

/* USARTClass.h */

typedef USART_TypeDef Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
