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

/* wiring_digital.c */

typedef struct stm32f0_pinDescription {
    __IO uint32_t *pin_port_moder;
    __IO uint32_t *pin_port_bsrr;
    __IO uint16_t *pin_port_brr;
    __IO uint16_t *pin_port_input;
    uint16_t pin_pin;
} stm32f0_pinDescription_t;

#ifdef __cplusplus
#define __FIELDINIT(name)
#else
#define __FIELDINIT(name) .name =
#endif
#define STM32F0_PORT(port, pin) { \
    __FIELDINIT(pin_port_moder)   &((GPIO ## port)->MODER),    \
    __FIELDINIT(pin_port_bsrr)    &((GPIO ## port)->BSRR),     \
    __FIELDINIT(pin_port_brr)     &((GPIO ## port)->BRR),      \
    __FIELDINIT(pin_port_input)   &((GPIO ## port)->IDR),      \
    __FIELDINIT(pin_pin)          GPIO_ODR_ ## pin,            \
}

/*
 * The reason why we define the pins here is to allow the compiler to
 * optimise the table access away.  The linker should be intelligent enough
 * to include it only once anyway.
 *
 * XXX: Study how much this helps with the code size.
 */
#ifdef __cplusplus
extern "C" {
#endif

const stm32f0_pinDescription_t stm32f0_pinDescription[] = {
    STM32F0_PORT(A, 10),  /* 0 RX0 */
    STM32F0_PORT(A, 9),   /* 1 TX0 */
    STM32F0_PORT(A, 8),   /* 2 D2 */
    STM32F0_PORT(C, 9),   /* 3 D3 */
    STM32F0_PORT(C, 8),   /* 4 D4 */
    STM32F0_PORT(A, 7),   /* 5 D5 */
    STM32F0_PORT(A, 3),   /* 6 D6 */
    STM32F0_PORT(A, 2),   /* 7 D7 */
    STM32F0_PORT(B, 10),  /* 8 D8 */
    STM32F0_PORT(B, 11),  /* 9 D9 */
    STM32F0_PORT(A, 15),  /* 10 D10 */
    STM32F0_PORT(B, 5),   /* 11 D11 */
    STM32F0_PORT(B, 4),   /* 12 D12 */
#if 0
    /* Ellduino */
    STM32F0_PORT(B, 3),   /* 13 Led Amber "L" */
#else
    /* Discovery */
    STM32F0_PORT(C, 8),   /* 13 Led Blue */
#endif
};
#ifdef __cplusplus
} // extern "C"
#endif

#undef __FIELDINIT


inline void digitalWrite(uint32_t pin, uint32_t val) {
    if (val)
        *(stm32f0_pinDescription[pin].pin_port_bsrr) = stm32f0_pinDescription[pin].pin_pin;
    else
        *(stm32f0_pinDescription[pin].pin_port_brr)  = stm32f0_pinDescription[pin].pin_pin;
}

inline int digitalRead(uint32_t pin) {
    return *(stm32f0_pinDescription[pin].pin_port_input) & stm32f0_pinDescription[pin].pin_pin? 1 /*HIGH*/: 0 /*LOW*/;
}

#define pinMode(pin, mode) pinMode_ ## mode(pin)
#define pinMode_uint32_t // EMPTY

inline void pinMode_INPUT(uint8_t pin) {
    *(stm32f0_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0   << pin * 2);
}

inline void pinMode_OUTPUT(uint8_t pin) {
    *(stm32f0_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0_1 << pin * 2);
    *(stm32f0_pinDescription[pin].pin_port_moder) |=  (GPIO_MODER_MODER0_0 << pin * 2);
}

/* wiring_analog.c */

#define PINS_COUNT (sizeof(stm32f0_pinDescription) / sizeof(stm32f0_pinDescription[0]))

/* UARTClass.h */

typedef int Uart;

/* USARTClass.h */

typedef int Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
