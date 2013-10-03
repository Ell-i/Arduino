/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_VARIANT_H
#define ELLDUINO_VARIANT_H

#define DEBUG 1

#define VARIANT_MCK 48000000

#ifdef __cplusplus
#define __FIELDINIT(name)
#else
#define __FIELDINIT(name) .name =
#endif
// XXX: AF not used at the moment
#define STM32_PIN(port, pin, timer, channel, af, adc_ch, etr) {    \
    __FIELDINIT(pin_port_moder)   &((GPIO ## port)->MODER),    \
    __FIELDINIT(pin_port_bsrr)    &((GPIO ## port)->BSRR),     \
    __FIELDINIT(pin_port_brr)     &((GPIO ## port)->BRR),      \
    __FIELDINIT(pin_port_input)   &((GPIO ## port)->IDR),      \
    __FIELDINIT(pin_pwm_timer)    TIM ## timer,            \
    __FIELDINIT(pin_pwm_ccr)      &((TIM ## timer)->CCR ## channel), \
    __FIELDINIT(pin_mask)          GPIO_ODR_ ## pin,            \
    __FIELDINIT(pin_number)       pin, \
    __FIELDINIT(pin_adc_channel)  (uint8_t)adc_ch,  \
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

static const stm32_pinDescription_t stm32_pinDescription[] = {
#define TIM0 ((TIM_TypeDef *) 0)
#define CCR0 CCR1
    /*      Port Pin Tim Ch AF ADC ETR */
    STM32_PIN(A, 10,  1, 3, 2, -1, NONE),    /* 0 RX0 */
    STM32_PIN(A,  9,  1, 2, 2, -1, TIMCH),   /* 1 TX0 */
    STM32_PIN(A,  8,  1, 1, 2, -1, TIMCH),   /* 2 D2 */
    STM32_PIN(C,  9,  3, 4, 0, -1, NONE),    /* 3 D3 */
    STM32_PIN(C,  8,  3, 3, 0, -1, NONE),    /* 4 D4 */
    STM32_PIN(A,  7, 14, 1, 4, -1, NONE),    /* 5 D5; could use TIM3 CH2 for trigger? */
    STM32_PIN(A,  3, 15, 2, 0, -1, TIMCH),   /* 6 D6 */
    STM32_PIN(A,  2, 15, 1, 0, -1, TIMCH),   /* 7 D7 */
    STM32_PIN(B, 10,  2, 3, 2, -1, NONE),    /* 8 D8 */
    STM32_PIN(B, 11,  2, 4, 2, -1, NONE),    /* 9 D9 */
    STM32_PIN(A, 15,  2, 1, 2, -1, TIMCH),   /* 10 D10 */
    STM32_PIN(B,  5,  3, 2, 1, -1, TIMCH),   /* 11 D11 */
    STM32_PIN(B,  4,  3, 1, 1, -1, TIMCH),   /* 12 D12 */
    STM32_PIN(B,  3,  2, 2, 2, -1, TIMCH),   /* 13 Led Amber "L" */
    STM32_PIN(C,  6,  3, 1, 0, -1, TIMCH),   /* 14 PC6 */
    STM32_PIN(C,  7,  3, 1, 0, -1, TIMCH),   /* 15 PC7 */
    STM32_PIN(A, 14,  0, 0, 0, -1, NONE),    /* 16 PA14, USART2_TX/SWCLK */
    STM32_PIN(D,  2,  3, 0, 0, -1, TIMETR),  /* 17 PD2 ???? XXX Check */
    STM32_PIN(B,  6, 16, 1, 0, -1, NONE),   /* 18 PB6 USART1_TX, TIM16 CH1N */
    STM32_PIN(B,  7, 17, 1, 0, -1, NONE),   /* 19 PB7 USART1_RX, TIM17 CH1N */
    STM32_PIN(B,  9, 17, 1, 0, -1, NONE),   /* 20 PB9 I2C1_SDA, TIM17 CH1 */
    STM32_PIN(B,  8, 16, 1, 0, -1, NONE),   /* 21 PB8 I2C1_SCL, TIM16 CH1 */
    STM32_PIN(C,  0,  0, 0, 0, 10, NONE),   /* 54 PC0 ADC_IN10 */
    STM32_PIN(C,  1,  0, 0, 0, 11, NONE),   /* 55 PC1 ADC_IN11 */
    STM32_PIN(C,  2,  0, 0, 0, 12, NONE),   /* 56 PC2 ADC_IN12 */
    STM32_PIN(C,  3,  0, 0, 0, 13, NONE),   /* 57 PC3 ADC_IN13 */
    STM32_PIN(A,  5,  2, 1, 0,  5, TIMCH),   /* 58 PA5 ADC_IN5 */
    STM32_PIN(A,  6,  3, 1, 0,  6, TIMCH),   /* 59 PA6 ADC_IN6 */
    STM32_PIN(A,  0,  2, 1, 0,  0, TIMCH),   /* 60 PA0 ADC_IN0 / WKUP1 */
    STM32_PIN(A,  1,  2, 2, 0,  1, TIMCH),   /* 61 PA1 ADC_IN1 */
    STM32_PIN(C,  4,  0, 0, 0, 14, NONE),   /* 62 PC4 ADC_IN14 */
    STM32_PIN(C,  5,  0, 0, 0, 15, NONE),   /* 63 PC5 ADC_IN15 */
    STM32_PIN(B,  0,  3, 3, 0,  8, NONE),   /* 64 PB0 ADC_IN8 */
    STM32_PIN(B,  1,  3, 4, 0,  9, NONE),   /* 65 PB1 ADC_IN9 */
    STM32_PIN(A,  4, 14, 1, 0, -1, NONE),   /* 66 PA4 DAC1_OUT */
#undef TIM0
#undef CCR0
};

#ifdef __cplusplus
} // extern "C"
#endif

#undef __FIELDINIT

/*
 * Timers
 */

static TIM_TypeDef *const stm32_arduino_pwm_timers[] = {
    TIM1, TIM2, TIM3, TIM14, TIM15
};

/*
 * Analog pins -- compatible with mega and due
 */
static const uint8_t A0  = 22;
static const uint8_t A1  = 23;
static const uint8_t A2  = 24;
static const uint8_t A3  = 25;
static const uint8_t A4  = 26;
static const uint8_t A5  = 27;
static const uint8_t A6  = 28;
static const uint8_t A7  = 29;
static const uint8_t A8  = 30;
static const uint8_t A9  = 31;
static const uint8_t A10 = 32;
static const uint8_t A11 = 33;
static const uint8_t DAC0 = 34;
#define ADC_RESOLUTION		12
#define NUM_ANALOG_INPUTS       11
#if 0
/* From Mega -- is this needed? */
#define analogInputToDigitalPin(p)  ((p < NUM_ANALOG_INPUTS) ? (p) + A0 : -1)
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

/*
 * Return the STM32F0 chip unique id.
 */
inline void getStmUniqueId(unsigned char uid[3 * sizeof(uint32_t)]) {
    uint32_t *p = (uint32_t *)uid;
    p[0] = *(uint32_t *)(0x1FFFF7AC + 0x00);
    p[1] = *(uint32_t *)(0x1FFFF7AC + 0x04);
    p[2] = *(uint32_t *)(0x1FFFF7AC + 0x08);
}

#ifdef __cplusplus

extern "C" {

extern RingBuffer rx_buffer;
extern RingBuffer rx_buffer1;

/* XXX Don't use a constructor at the moment, to initialise these as PODs */
static const HardwareSerial Serial =  {
    USART2,
    &rx_buffer,
    GPIOA->AFR[1],
    GPIOA->MODER,
    ~(GPIO_AFRH_AFRH6     | GPIO_AFRH_AFRH7),
    GPIO_AFRx_AFRx6_AF1 | GPIO_AFRx_AFRx7_AF1,
    ~(GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0),
    (GPIO_MODER_MODER14_1  | GPIO_MODER_MODER15_1),
    0, 0, 0, 0, /* XXX FIX THIS */
};

static const HardwareSerial Serial1 = {
    USART1,
    &rx_buffer1,
    GPIOA->AFR[1],
    GPIOA->MODER,
    ~(GPIO_AFRH_AFRH1     | GPIO_AFRH_AFRH2),
    GPIO_AFRx_AFRx1_AF1 | GPIO_AFRx_AFRx2_AF1,
    ~(GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0),
    (GPIO_MODER_MODER9_1  | GPIO_MODER_MODER10_1),
    0, 0, 0, 0, /* XXX FIX THIS */
};


};

#endif

#endif /* ELLDUINO_VARIANT_H */
