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
    uint8_t pin_adc_channel;
} stm32f0_pinDescription_t;

#ifdef __cplusplus
#define __FIELDINIT(name)
#else
#define __FIELDINIT(name) .name =
#endif
#define STM32F0_PIN(port, pin, timer, channel, af, adc_ch) {         \
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
    __FIELDINIT(pin_adc_channel)  adc_ch,  \
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
#define TIM0 ((TIM_TypeDef *) 0)
#define CCR0 CCR1
    /*        Port Pin Tim Ch AF ADC */
    STM32F0_PIN(A, 10,  1, 3, 2, -1),   /* 0 RX0 */
    STM32F0_PIN(A,  9,  1, 2, 2, -1),   /* 1 TX0 */
    STM32F0_PIN(A,  8,  1, 1, 2, -1),   /* 2 D2 */
    STM32F0_PIN(C,  9,  3, 4, 0, -1),   /* 3 D3 */
    STM32F0_PIN(C,  8,  3, 3, 0, -1),   /* 4 D4 */
    STM32F0_PIN(A,  7, 14, 1, 4, -1),   /* 5 D5 */
    STM32F0_PIN(A,  3, 15, 2, 0, -1),   /* 6 D6 */
    STM32F0_PIN(A,  2, 15, 1, 0, -1),   /* 7 D7 */
    STM32F0_PIN(B, 10,  2, 3, 2, -1),   /* 8 D8 */
    STM32F0_PIN(B, 11,  2, 4, 2, -1),   /* 9 D9 */
    STM32F0_PIN(A, 15,  2, 1, 2, -1),   /* 10 D10 */
    STM32F0_PIN(B,  5,  3, 2, 1, -1),   /* 11 D11 */
    STM32F0_PIN(B,  4,  3, 1, 1, -1),   /* 12 D12 */
#if 0
    /* Ellduino */
    STM32F0_PIN(B,  3,  2, 2, 2, -1),   /* 13 Led Amber "L" */
#else
    /* Discovery */
    STM32F0_PIN(C,  8,  3, 3, 0, -1),   /* 13 Led Blue */
#endif
    STM32F0_PIN(C,  6,  0, 0, 0, -1),   /* 14 PC6 */
    STM32F0_PIN(C,  7,  0, 0, 0, -1),   /* 15 PC7 */
    STM32F0_PIN(A, 14,  0, 0, 0, -1),   /* 16 PA14, USART2_TX/SWCLK */
    STM32F0_PIN(D,  2,  0, 0, 0, -1),   /* 17 PD2 ???? XXX Check */
    STM32F0_PIN(B,  6,  0, 0, 0, -1),   /* 18 PB6 USART1_TX */
    STM32F0_PIN(B,  7,  0, 0, 0, -1),   /* 19 PB7 USART1_RX */
    STM32F0_PIN(B,  9,  0, 0, 0, -1),   /* 20 PB9 I2C1_SDA */
    STM32F0_PIN(B,  8,  0, 0, 0, -1),   /* 21 PB8 I2C1_SCL */
    STM32F0_PIN(C,  0,  0, 0, 0, 10),   /* 54 PC0 ADC_IN10 */
    STM32F0_PIN(C,  1,  0, 0, 0, 11),   /* 55 PC1 ADC_IN11 */
    STM32F0_PIN(C,  2,  0, 0, 0, 12),   /* 56 PC2 ADC_IN12 */
    STM32F0_PIN(C,  3,  0, 0, 0, 13),   /* 57 PC3 ADC_IN13 */
    STM32F0_PIN(A,  5,  0, 0, 0,  5),   /* 58 PA5 ADC_IN5 */
    STM32F0_PIN(A,  6,  0, 0, 0,  6),   /* 59 PA6 ADC_IN6 */
    STM32F0_PIN(A,  0,  0, 0, 0,  0),   /* 60 PA0 ADC_IN0 / WKUP1 */
    STM32F0_PIN(A,  1,  0, 0, 0,  1),   /* 61 PA1 ADC_IN1 */
    STM32F0_PIN(C,  4,  0, 0, 0, 14),   /* 62 PC4 ADC_IN14 */
    STM32F0_PIN(C,  5,  0, 0, 0, 15),   /* 63 PC5 ADC_IN15 */
    STM32F0_PIN(B,  0,  0, 0, 0,  8),   /* 64 PB0 ADC_IN8 */
    STM32F0_PIN(B,  1,  0, 0, 0,  9),   /* 65 PB1 ADC_IN9 */
    STM32F0_PIN(A,  4,  0, 0, 0, -1),   /* 66 PA4 DAC1_OUT */
#undef TIM0
#undef CCR0
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
    const uint32_t pin_number = stm32f0_pinDescription[pin].pin_number;
    const stm32f0_pinDescription_t * const pinp = &stm32f0_pinDescription[pin];

    /* Configure the I/O pin to analog mode */
    *(pinp->pin_port_moder) |=  (GPIO_MODER_MODER0 << (pin_number * 2));

    /* Configure the selection channel */
    ADC1->CHSELR = 1 << pinp->pin_adc_channel;

    /* Wait for ADC to be ready */
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
      GPIOC->ODR ^= GPIO_ODR_9;
      GPIOC->ODR ^= GPIO_ODR_7;
      /* XXX yield() */;
    }

    /* Start conversion */
    ADC1->CR |= ADC_CR_ADSTART;

    /* Wait for the ADC to complete conversion */
    while (!(ADC1->ISR & ADC_ISR_EOC)) {
      GPIOC->ODR ^= GPIO_ODR_9;
      GPIOC->ODR ^= GPIO_ODR_6;
      /* XXX yield() */;
    }

    /* Return the data */
    return ADC1->DR;
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
