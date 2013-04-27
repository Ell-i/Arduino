/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

/* XXX Temp definitions to compile */

#ifndef ELLDUINO_STM32F051_CHIP_H
#define ELLDUINO_STM32F051_CHIP_H 1

/* Arduino.h */

typedef int Pio, EPioType;

/* wiring.c */

extern struct SysTick {
    int VAL;
    int CTRL;
    int LOAD;
} *SysTick;

extern int SystemCoreClock;

#define SysTick_CTRL_COUNTFLAG_Msk 0

/* wiring_analog.c */

#define PINS_COUNT 1

/* UARTClass.h */

typedef int Uart;
typedef int IRQn_Type;

/* USARTClass.h */

typedef int Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
