/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

/* XXX Temp definitions to compile */

#ifndef ELLDUINO_STM32F051_CHIP_H
#define ELLDUINO_STM32F051_CHIP_H 1

#include <stm32f0xx.h>
#include <stm32f0xx_extra.h>

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
#if 0
    __IO uint32_t *pin_port_afr;
    uint32_t pin_pwm_af;
#endif
    __IO uint32_t *pin_pwm_ccr; /* Must be 32 bit pointer due to Timer 2 */
    uint16_t pin_mask;
    uint8_t pin_number;
} stm32f0_pinDescription_t;

#ifdef __cplusplus
#define __FIELDINIT(name)
#else
#define __FIELDINIT(name) .name =
#endif
#define STM32F0_PORT(port, pin, timer, channel, af) {          \
    __FIELDINIT(pin_port_moder)   &((GPIO ## port)->MODER),    \
    __FIELDINIT(pin_port_bsrr)    &((GPIO ## port)->BSRR),     \
    __FIELDINIT(pin_port_brr)     &((GPIO ## port)->BRR),      \
    __FIELDINIT(pin_port_input)   &((GPIO ## port)->IDR),      \
    /* \
    __FIELDINIT(pin_port_afr)     &((GPIO ## port)->AFR[(pin) >= 8]), \
    __FIELDINIT(pin_pwm_af)       (af) % 8,                    \
    */ \
    __FIELDINIT(pin_pwm_ccr)      &((TIM ## timer)->CCR ## channel), \
    __FIELDINIT(pin_mask)          GPIO_ODR_ ## pin,            \
    __FIELDINIT(pin_number)       pin, \
}

/*
 * The reason why we define the pins here is to allow the compiler to
 * optimise the table access away.  As it is static, it won't get
 * compiled in unless it is really needed.
 *
 * XXX: Study how much this helps with the code size.
 */
#ifdef __cplusplus
extern "C" {
#endif

static const stm32f0_pinDescription_t stm32f0_pinDescription[] = {
    STM32F0_PORT(A, 10,  1, 3, 2),   /* 0 RX0 */
    STM32F0_PORT(A,  9,  1, 2, 2),   /* 1 TX0 */
    STM32F0_PORT(A,  8,  1, 1, 2),   /* 2 D2 */
    STM32F0_PORT(C,  9,  3, 4, 0),   /* 3 D3 */
    STM32F0_PORT(C,  8,  3, 3, 0),   /* 4 D4 */
    STM32F0_PORT(A,  7, 14, 1, 4),   /* 5 D5 */
    STM32F0_PORT(A,  3, 15, 2, 0),   /* 6 D6 */
    STM32F0_PORT(A,  2, 15, 1, 0),   /* 7 D7 */
    STM32F0_PORT(B, 10,  2, 3, 2),   /* 8 D8 */
    STM32F0_PORT(B, 11,  2, 4, 2),   /* 9 D9 */
    STM32F0_PORT(A, 15,  2, 1, 2),   /* 10 D10 */
    STM32F0_PORT(B,  5,  3, 2, 1),   /* 11 D11 */
    STM32F0_PORT(B,  4,  3, 1, 1),   /* 12 D12 */
#if 0
    /* Ellduino */
    STM32F0_PORT(B, 3, 2, 2, 2),   /* 13 Led Amber "L" */
#else
    /* Discovery */
    STM32F0_PORT(C, 8, 3, 3, 0),   /* 13 Led Blue */
#endif
};
#ifdef __cplusplus
} // extern "C"
#endif

#undef __FIELDINIT


inline void digitalWrite(uint32_t pin, uint32_t val) {
    if (val)
        *(stm32f0_pinDescription[pin].pin_port_bsrr) = stm32f0_pinDescription[pin].pin_mask;
    else
        *(stm32f0_pinDescription[pin].pin_port_brr)  = stm32f0_pinDescription[pin].pin_mask;
}

inline int digitalRead(uint32_t pin) {
    return *(stm32f0_pinDescription[pin].pin_port_input) & stm32f0_pinDescription[pin].pin_mask?
        1 /*HIGH*/: 0 /*LOW*/;
}

#define pinMode(pin, mode) pinMode_ ## mode(pin)
#define pinMode_uint32_t // EMPTY

inline void pinMode_INPUT(uint8_t pin) {
    /* Input mode */
    const uint32_t pin_number = stm32f0_pinDescription[pin].pin_number;
    *(stm32f0_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0   << (pin_number * 2));
}

inline void pinMode_OUTPUT(uint8_t pin) {
    /* Output mode */
    const uint32_t pin_number = stm32f0_pinDescription[pin].pin_number;
    *(stm32f0_pinDescription[pin].pin_port_moder) |=  (GPIO_MODER_MODER0_0 << (pin_number * 2));
    *(stm32f0_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0_1 << (pin_number * 2));
}

inline void analogWrite(uint32_t pin, uint32_t value) {
    const uint32_t pin_number = stm32f0_pinDescription[pin].pin_number;
    const stm32f0_pinDescription_t * const pinp = &stm32f0_pinDescription[pin];

#if 0
    const uint32_t af_mask  = ~(0xf)              << (pinp->pin_mask);
    const uint32_t af_value =  (pinp->pin_pwm_af) << (pinp->pin_mask);

    /* XXX: Should be selected by default. */
    /* Select GPIO alternate function.  Would temporarily change values, hence the check. */
    if ((*(pinp->pin_port_afr) & af_mask) != af_value) {
        *(pinp->pin_port_afr) &= af_mask;
        *(pinp->pin_port_afr) |= af_value;
    }
#endif

    /* Place the GPIO in the Alternate Function mode.  Idempotent. */
    *(pinp->pin_port_moder) &= ~(GPIO_MODER_MODER0_0 << (pin_number * 2));
    *(pinp->pin_port_moder) |=  (GPIO_MODER_MODER0_1 << (pin_number * 2));

    /* Channel counter value */
    *(pinp->pin_pwm_ccr)   = value;
}

inline uint32_t analogRead(uint32_t pin) {
    // XXX Continue here
    return 0;
}

/* wiring_analog.c */

#define PINS_COUNT (sizeof(stm32f0_pinDescription) / sizeof(stm32f0_pinDescription[0]))

/* UARTClass.h */

typedef int Uart; /* XXX Not used */

/* USARTClass.h */

typedef USART_TypeDef Usart;

/* wiring.c */

extern uint32_t GetTickCount(void);

#endif /* ELLDUINO_STM32F051_CHIP_H */
