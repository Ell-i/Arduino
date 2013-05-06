/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

/* XXX Temp definitions to compile */

#ifndef ELLDUINO_STM32F051_CHIP_H
#define ELLDUINO_STM32F051_CHIP_H 1

#include <stm32f0xx.h>

#undef ADC1 /* Arduino.h uses for EAnalogChannel */
#define ADC_1 ((ADC_TypeDef *) ADC1_BASE)

/* Arduino.h */

typedef int Pio, EPioType;

/* wiring_analog.c */

#define PINS_COUNT 1

/* UARTClass.h */

typedef int Uart;

/* USARTClass.h */

typedef int Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
