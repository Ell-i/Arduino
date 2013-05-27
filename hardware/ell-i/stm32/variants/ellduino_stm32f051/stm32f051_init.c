/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

/*
 * A data driven approach to initialise STM32F peripherals.
 */

#include <stm32f0xx.h>
#include <stm32f0xx_extra.h>

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "init.h"
#include "Arduino.h"

#define D16(d, r, v)    DEVICE_REGISTER_INIT_STRUCT_VALUE16((d), r, (v))
#define D32(d, r, v)    DEVICE_REGISTER_INIT_STRUCT_VALUE32((d), r, (v))
#define M32(d, r, m, v) DEVICE_REGISTER_INIT_STRUCT_MASK_VALUE32((d), r, (m), (v))
#define W32(d, r, m, v) DEVICE_REGISTER_INIT_STRUCT_WAIT_VALUE32((d), r, (m), (v))


/*
 * Enable the clock for all peripherals possibly used in the Arduino
 * mode.  We do not care about power saving here, as we assume that
 * the MCU is running out of PoE or line power.
 */

/*
 * XXX: Try to remove the masked initialisation from here,
 *      paying more attention to what is initialised by default.
 */

const struct device_register_init_masked_32bit reset_and_clock_control[] = {
    M32(RCC, AHBENR,
        0
        /* SRAM and Flash enabled by default */
        | RCC_AHBENR_GPIOAEN    /* Port A, used as Arduino I/O ports */
        | RCC_AHBENR_GPIOBEN    /* Port B, used both as Arduino I/O ports and for SPI2 */
        | RCC_AHBENR_GPIOCEN    /* Port C, used as Arduino I/O ports */
        | RCC_AHBENR_GPIODEN    /* Port D, used as Arduino I/O ports */
        | RCC_AHBENR_GPIOFEN    /* Port F, used as Arduino I/O ports */
        ,
        0
        | RCC_AHBENR_GPIOAEN
        | RCC_AHBENR_GPIOBEN
        | RCC_AHBENR_GPIOCEN
        | RCC_AHBENR_GPIODEN
        | RCC_AHBENR_GPIOFEN
        ),
    M32(RCC, APB2ENR,
        0
        | RCC_APB2ENR_SYSCFGEN
        | RCC_APB2ENR_ADC1EN    /* ADC, used for the Arduino analogRead */
        | RCC_APB2ENR_SPI1EN    /* SPI 1, TBD */
        | RCC_APB2ENR_USART1EN  /* USART 1, used as Arduino Serial */
        | RCC_APB2ENR_TIM1EN    /* Timer 1,  used for Arduino PWM D0 (CH3), D1 (CH2), D2 (CH1) */
        | RCC_APB2ENR_TIM15EN   /* Timer 15, used for Arduino PWM D6 (CH2), D7 (CH1) */
        ,
        0
        | RCC_APB2ENR_SYSCFGEN
        | RCC_APB2ENR_ADC1EN
        | RCC_APB2ENR_SPI1EN
        | RCC_APB2ENR_USART1EN
        | RCC_APB2ENR_TIM1EN
        | RCC_APB2ENR_TIM15EN
        ),
    M32(RCC, APB1ENR,
        0
        | RCC_APB1ENR_TIM2EN   /* Timer 2,  used for Arduino PWM D8 (CH3), D9 (CH4), D10 (CH1), D13 (CH2) */
        | RCC_APB1ENR_TIM3EN   /* Timer 3,  used for Arduino PWM D3 (CH4), D4 (CH3), D11 (CH2), D12 (CH1) */
        | RCC_APB1ENR_TIM14EN  /* Timer 14, used for Arduino PWM D5 (CH1) */
        | RCC_APB1ENR_SPI2EN   /* SPI 2, used for communicating with ENC28J60 */
        | RCC_APB1ENR_USART2EN /* USART 2, used as Arduino Serial */
        | RCC_APB1ENR_DACEN    /* DAC, used as Arduino DUE DAC 1 */
        ,
        0
        | RCC_APB1ENR_TIM2EN
        | RCC_APB1ENR_TIM3EN
        | RCC_APB1ENR_TIM14EN
        | RCC_APB1ENR_SPI2EN
        | RCC_APB1ENR_USART2EN
        | RCC_APB1ENR_DACEN
        ),
};

/*
 * Assign a given priority for an interrupt, and enable the interrupt.
 *
 * See page 4-7 of ARM DUI 0497A, Cortex-M0 Peripherals and
 * NVIC_SetPriority implementation in CMSIS/Include/core_cm0.h
 */
#define _NVIC_PRIO_SHIFT(prio) ((prio) << (8 - __NVIC_PRIO_BITS))
#define ENABLE_INTERRUPT(irq, prio)                                 \
    M32(NVIC, IP[_IP_IDX((irq))],                                   \
        (                         0x000000FF) << _BIT_SHIFT((irq)), \
        (_NVIC_PRIO_SHIFT(prio) & 0x000000FF) << _BIT_SHIFT((irq))  \
        ),                                                          \
    M32(NVIC, ISER[0],                                              \
        1 << (irq), /* NOP */                                       \
        1 << (irq)                                                  \
       )

const struct device_register_init_masked_32bit nvic[] = {
    ENABLE_INTERRUPT(SPI2_IRQn, 3), /* Lowest priority */
    ENABLE_INTERRUPT(USART1_IRQn, 3),
    ENABLE_INTERRUPT(USART2_IRQn, 3),
};

/*
 * Arduino (Atmega) pins default to inputs, so they don't need to be
 * explicitly declared as inputs with pinMode(). Pins configured as
 * inputs are in a high-impedance state.  Pins configured as OUTPUT
 * with pinMode() are said to be in a low-impedance state.
 *
 * For STM32F051, during and just after reset, the alternate functions
 * are not active and the I/O ports are configured in input floating
 * mode, with the exception of PA13 and PA14, which are used by default
 * by the SWD protocol.  Output is by default push/pull (low impedance)
 * and low speed, which is good for Arduino compatibility.
 *
 * As the reset state of the Arduino Analog input pins are not clearly
 * defined, we default them also to the input mode.
 */

/************
 *  Port A  *
 ************/

const struct device_register_init_static_32bit general_purpose_io_a[] = {
    /*
     * Port mode.  Defaults to Input for all but DAC.
     */
    D32(GPIOA, MODER,
        0
        | ! GPIO_MODER_MODER0    /* 00  PA0  A4 */
        | ! GPIO_MODER_MODER1    /* 00  PA1  A5 */
        | ! GPIO_MODER_MODER2    /* 00  PA2  D7, COMP1_INM */
        | ! GPIO_MODER_MODER3    /* 00  PA3  D6, COMP1_INP */
        /* Once the DAC channel 1 is enabled, the corresponding GPIO
         * pin (PA4) is automatically connected to the analog
         * converter output (DAC1_OUT). In order to avoid parasitic
         * consumption, the PA4 pin should first be configured to
         * analog (AIN).
         */
        |   GPIO_MODER_MODER4    /* 11  PA4  DAC0 */
        | ! GPIO_MODER_MODER5    /* 00  PA5  A6 */
        | ! GPIO_MODER_MODER6    /* 00  PA6  A7 */
        | ! GPIO_MODER_MODER7    /* 00  PA7  D5 */
        | ! GPIO_MODER_MODER8    /* 00  PA8  D2 */
        | ! GPIO_MODER_MODER9    /* 00  PA9  D1 (TX0) */
        | ! GPIO_MODER_MODER10   /* 00 PA10  D0 (RX0) */
        | ! GPIO_MODER_MODER11   /* 00 PA11  DAC1  (CTS0), used as input by default */
        | ! GPIO_MODER_MODER12   /* 00 PA12  CANRX (RTS0), used as input by default */
#ifdef DISABLE_SWD
        | ! GPIO_MODER_MODER13   /* 00 PA13  CANTX, SW Data */
        | ! GPIO_MODER_MODER14   /* 00 PA14  TX2,   SW Clock */
#else
        |   GPIO_MODER_MODER13_1 /* 10 PA13 AF     SWDAT, reset-time value */
        |   GPIO_MODER_MODER14_1 /* 10 PA14 AF     SWCLK, reset-time value */
#endif
        | ! GPIO_MODER_MODER15   /* 00 PA15  D10, SPI1_NSS */
        ),
    /*
     * Output type: Push-pull for all Output.  Default value.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOA, OTYPER, 0),
#endif
    /*
     * See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet.
     * Set the speed to high, as we don't care about power consumption.
     */
    D32(GPIOA, OSPEEDR,
        0
        |   GPIO_OSPEEDER_OSPEEDR0
        |   GPIO_OSPEEDER_OSPEEDR1
        |   GPIO_OSPEEDER_OSPEEDR2
        |   GPIO_OSPEEDER_OSPEEDR3
        |   GPIO_OSPEEDER_OSPEEDR4
        |   GPIO_OSPEEDER_OSPEEDR5
        |   GPIO_OSPEEDER_OSPEEDR6
        |   GPIO_OSPEEDER_OSPEEDR7
        |   GPIO_OSPEEDER_OSPEEDR8
        |   GPIO_OSPEEDER_OSPEEDR9
        |   GPIO_OSPEEDER_OSPEEDR10
        |   GPIO_OSPEEDER_OSPEEDR11
        |   GPIO_OSPEEDER_OSPEEDR12
        |   GPIO_OSPEEDER_OSPEEDR13 /* SWDAT, Reset-time value */
#ifdef DISABLE_SWD
        |   GPIO_OSPEEDER_OSPEEDR14
#else
        | ! GPIO_OSPEEDER_OSPEEDR14 /* SWCLK, Reset-time value */
#endif
        |   GPIO_OSPEEDER_OSPEEDR15
        ),
    /*
     * Let the inputs to float.
     */
    D32(GPIOA, PUPDR,
        0
        | ! GPIO_PUPDR_PUPDR0
        | ! GPIO_PUPDR_PUPDR1
        | ! GPIO_PUPDR_PUPDR2
        | ! GPIO_PUPDR_PUPDR3
        | ! GPIO_PUPDR_PUPDR4
        | ! GPIO_PUPDR_PUPDR5
        | ! GPIO_PUPDR_PUPDR6
        | ! GPIO_PUPDR_PUPDR7
        | ! GPIO_PUPDR_PUPDR8
        | ! GPIO_PUPDR_PUPDR9
        | ! GPIO_PUPDR_PUPDR10
        | ! GPIO_PUPDR_PUPDR11
        | ! GPIO_PUPDR_PUPDR12
#ifdef DISABLE_SWD
        | ! GPIO_PUPDR_PUPDR13
        | ! GPIO_PUPDR_PUPDR14
#else
        |   GPIO_PUPDR_PUPDR13_0 /* PA13 <> SWDAT   pull up,   reset-time value */
        |   GPIO_PUPDR_PUPDR14_1 /* PA10 <- SWCLK   pull down, reset-time value */
#endif
        | ! GPIO_PUPDR_PUPDR15
        ),
    /*
     * Pre-select the appropriate default AF for each pin so
     * that e.g. analogWrite does not need to change it.
     */
    D32(GPIOA, AFR[0],
        0
        | ! GPIO_AFRL_AFRL0      /* 0   PA0   A6, no alternate function */
        | ! GPIO_AFRL_AFRL1      /* 0   PA1   A7, no alternate function */
        |   GPIO_AFRx_AFRx2_AF0  /* 0   PA2   D7, Timer 15 Channel 1 */
        |   GPIO_AFRx_AFRx3_AF0  /* 0   PA3   D6, Timer 15 Channel 2 */
        | ! GPIO_AFRL_AFRL4      /* 0   PA4 DAC0, no alternate function */
        | ! GPIO_AFRL_AFRL5      /* 0   PA5   A4, no alternate function */
        | ! GPIO_AFRL_AFRL6      /* 0   PA6   A5, no alternate function */
        |   GPIO_AFRx_AFRx7_AF4  /* 4   PA7   D5, Timer 14 Channel 1 */
        ),
    D32(GPIOA, AFR[1],
        0
        |   GPIO_AFRx_AFRx0_AF2  /* 2   PA8   D2, Timer 1 Channel 1 */
        |   GPIO_AFRx_AFRx1_AF2  /* 2   PA9   D1, Timer 1 Channel 2 */
        |   GPIO_AFRx_AFRx2_AF2  /* 2  PA10   D0, Timer 1 Channel 3 */
        | ! GPIO_AFRH_AFRH3      /* 0  PA11 DAC1, no alternate function (USART1_CTS?) */
        | ! GPIO_AFRH_AFRH4      /* 0  PA12  CAN, no alternate function (USART1_RTS?) */
#ifdef DISABLE_SWD
        | ! GPIO_AFRH_AFRH5      /* 0  PA13  CAN, no alternate function (SWD) */
        |   GPIO_AFRx_AFRx6_AF1  /* 1  PA14  TX2, USART2_TX */
#else
        |   GPIO_AFRx_AFRx5_AF0  /* 0  PA13  SWD, reset-time value */
        |   GPIO_AFRx_AFRx6_AF0  /* 0  PA14  SWC, reset-time value */
#endif
        |   GPIO_AFRx_AFRx7_AF2  /* 2  PA15  D10, Timer 2 Channel 1 */
        ),

};

/************
 *  Port B  *
 ************/

const struct device_register_init_static_32bit general_purpose_io_b[] = {
    /*
     * Port mode.  Defaults to Input for B0-B11.  B12-B15 used for SPI.
     */
    D32(GPIOB, MODER,
        0
        | ! GPIO_MODER_MODER0    /* 00  PB0  A10 */
        | ! GPIO_MODER_MODER1    /* 00  PB1  A11 */
        | ! GPIO_MODER_MODER2    /* 00  PB2  (NPCS0) */
        | ! GPIO_MODER_MODER3    /* 00  PB3  D13 SPI1_SCK  Amber Led "L" */
        | ! GPIO_MODER_MODER4    /* 00  PB4  D12 SPI1_MISO */
        | ! GPIO_MODER_MODER5    /* 00  PB5  D11 SPI1_MOSI */
        | ! GPIO_MODER_MODER6    /* 00  PB6  TX1 */
        | ! GPIO_MODER_MODER7    /* 00  PB7  RX1 */
        | ! GPIO_MODER_MODER8    /* 00  PB8  SCL, also FM+, also CEC */
        | ! GPIO_MODER_MODER9    /* 00  PB9  SDA, also FM+, also IR */
        | ! GPIO_MODER_MODER10   /* 00 PB10  D8 */
        | ! GPIO_MODER_MODER11   /* 00 PB11  D9 */
        |   GPIO_MODER_MODER12_1 /* 10 PB12  SPI2_NSS: XXX Is this right? */
        |   GPIO_MODER_MODER13_1 /* 10 PB13  SPI2_SCK */
        |   GPIO_MODER_MODER14_1 /* 10 PB14  SPI2_MISO */
        |   GPIO_MODER_MODER15_1 /* 10 PB15  SPI2_MOSI */
        ),
    /*
     * Output type: Push-pull for all Output.  Default value.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOB, OTYPER, 0),
#endif
    /*
     * See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet.
     * Set the speed to high, as we don't care about power consumption.
     */
    D32(GPIOB, OSPEEDR, ~0),
    /*
     * Let the inputs to float.  Handle ENC28J60 SPI.
     */
    D32(GPIOB, PUPDR,
        0
        |   GPIO_PUPDR_PUPDR14_1  /* PB14  <- MISO    pull down */
        ),

    /*
     * Pre-select the appropriate default AF for each pin so
     * that e.g. analogWrite does not need to change it.
     */
    D32(GPIOB, AFR[0],
        0
        | ! GPIO_AFRL_AFRL0      /* 0  PB0  A10, no alternate function */
        | ! GPIO_AFRL_AFRL1      /* 0  PB1  A11, no alternate function */
        | ! GPIO_AFRL_AFRL2      /* 0  PB2  (NPCS0) */
        |   GPIO_AFRx_AFRx3_AF2  /* 2  PB3  D13, Timer 2 Channel 2 */
        |   GPIO_AFRx_AFRx4_AF1  /* 1  PB4  D12, Timer 3 Channel 1 */
        |   GPIO_AFRx_AFRx5_AF1  /* 1  PB5  D11, Timer 3 Channel 2 */
        |   GPIO_AFRx_AFRx6_AF0  /* 0  PB6  TX1  USART1_TX */
        |   GPIO_AFRx_AFRx7_AF0  /* 0  PB7  RX1  USART1_RX */
        ),
    D32(GPIOB, AFR[1],
        0
        |   GPIO_AFRx_AFRx0_AF1  /* 1  PB8  SCL  I2C1_SCL */
        |   GPIO_AFRx_AFRx1_AF1  /* 1  PB9  SDA  I2C1_SDA */
        |   GPIO_AFRx_AFRx2_AF2  /* 2 PB10  D8, Timer 2 Channel 3 */
        |   GPIO_AFRx_AFRx3_AF2  /* 2 PB11  D9, Timer 2 Channel 4 */

        |   GPIO_AFRx_AFRx4_AF0  /* 0 PB12  SPI2_NSS  ENC28J60 */
        |   GPIO_AFRx_AFRx5_AF0  /* 0 PB13  SPI2_SCK  ENC28J60 */
        |   GPIO_AFRx_AFRx6_AF0  /* 0 PB14  SPI2_MISO ENC28J60 */
        |   GPIO_AFRx_AFRx7_AF0  /* 0 PB15  SPI2_MOSI ENC28J60 */
        ),
};

/************
 *  Port C  *
 ************/

const struct device_register_init_static_32bit general_purpose_io_c[] = {
    D32(GPIOC, MODER,
        0
        | ! GPIO_MODER_MODER0    /* 00  PC0  A0 */
        | ! GPIO_MODER_MODER1    /* 00  PC1  A1 */
        | ! GPIO_MODER_MODER2    /* 00  PC2  A2 */
        | ! GPIO_MODER_MODER3    /* 00  PC3  A3 */
        | ! GPIO_MODER_MODER4    /* 00  PC4  A8 */
        | ! GPIO_MODER_MODER5    /* 00  PC5  A9 */
#ifdef DISABLE_DISCOVERY_LEDS
        | ! GPIO_MODER_MODER6    /* 00  PC6  TX3 */
        | ! GPIO_MODER_MODER7    /* 00  PC7  RX3 */
        | ! GPIO_MODER_MODER8    /* 00  PC8  D4, Timer 3 Channel 3 */
        | ! GPIO_MODER_MODER9    /* 00  PC9  D5, Timer 3 Channel 4 */
#else
        |   GPIO_MODER_MODER6_0  /* 01  PC6  Debugging */
        |   GPIO_MODER_MODER7_0  /* 01  PC7  Debugging */
        |   GPIO_MODER_MODER8_0  /* 01  PC8  Discovery board Blue LED */
        |   GPIO_MODER_MODER9_0  /* 01  PC9  Discovery board Green LED */
#endif
        | ! GPIO_MODER_MODER10   /* 00 PC10  INT  ENC28J60 */
        |   GPIO_MODER_MODER11_0 /* 01 PC11  RES  ENC28J60 */
        | ! GPIO_MODER_MODER12   /* 00 PC12  N/C */
        | ! GPIO_MODER_MODER13   /* 00 PC13  RTC */
        | ! GPIO_MODER_MODER14   /* 00 PC14  N/C */
        | ! GPIO_MODER_MODER15   /* 00 PC15  N/C */
        ),
    /*
     * Output type: Push-pull for all Output.  Default value.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOC, OTYPER, 0),
#endif
    /*
     * See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet.
     * Set the speed to high, as we don't care about power consumption.
     *
     * PC13, PC14 and PC15 are supplied through the power switch.
     * Since the switch only sinks a limited amount of current
     * (3 mA), the use of GPIO PC13 to PC15 in output mode is limited:
     *
     * - The speed should not exceed 2 MHz with a maximum load of 30 pF
     * - these GPIOs must not be used as a current sources (e.g. to
     *   drive an LED).
     */
    D32(GPIOC, OSPEEDR, 0x03FFFFFF),
    /*
     * Let the inputs to float; outputs are driven by push/pull.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOC, PUPDR, 0),
#endif
};

/************
 *  Port D  *
 ************/

const struct device_register_init_static_32bit general_purpose_io_d[] = {
    D32(GPIOD, MODER,
        0
        | ! GPIO_MODER_MODER2    /* 00  PD2  (RX2) */
        ),
    /*
     * Output type: Push-pull for all Output.  Default value.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOD, OTYPER, 0),
#endif
    /*
     * See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet.
     * Set the speed to high, as we don't care about power consumption.
     */
    D32(GPIOD, OSPEEDR, ~0),
    /*
     * Let the inputs to float; outputs are driven by push/pull.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOD, PUPDR, 0),
#endif
};

/************
 *  Port F  *
 ************/

const struct device_register_init_static_32bit general_purpose_io_f[] = {
    D32(GPIOF, MODER,
        0
        | ! GPIO_MODER_MODER0    /* 00  PF0 N/C */
        | ! GPIO_MODER_MODER1    /* 00  PF1 N/C */
        | ! GPIO_MODER_MODER4    /* 00  PF4 N/C */
        | ! GPIO_MODER_MODER5    /* 00  PF5 N/C */
        | ! GPIO_MODER_MODER6_1  /* 10  PF6 SCL1 I2C2_SCL */
        | ! GPIO_MODER_MODER7_1  /* 10  PF7 SDA1 I2C2_SDA */
        ),
    /*
     * Output type: Push-pull for all Output.  Default value.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOF, OTYPER, 0),
#endif
    /*
     * See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet.
     * Let the speed to be low, as we don't care about power consumption.
     */
    D32(GPIOF, OSPEEDR, ~0),
    /*
     * Let the inputs to float; outputs are driven by push/pull.
     */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(GPIOF, PUPDR, 0),
#endif
};

/*
 * Timer-counter for Arduino PWM output
 */
const struct device_register_init_static_16bit arduino_pwm_output[] = {
    D16(TIM1, CR1, 0),         /* Disable the counter */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D16(TIM1, CR2, 0),      /* Default value */
    D16(TIM1, SMCR, 0),     /* Default value */
    D16(TIM1, DIER, 0),     /* Default value */
#endif
    D16(TIM1, CCMR1,
        0
        |   TIM_CCMR1_OC2M_PWM1 /* 110 = CH2 PWM Mode 1 */
        |   TIM_CCMR1_OC2PE     /* 1   = CH2 preload register in use */
        |   TIM_CCMR1_OC2FE     /* 1   = CH2 compare fast mode enabled */
        | ! TIM_CCMR1_CC2S      /* 00  = CH2 is output */
        |   TIM_CCMR1_OC1M_PWM1 /* 110 = CH1 PWM Mode 1 */
        |   TIM_CCMR1_OC1PE     /* 1   = CH1 preload register in use */
        |   TIM_CCMR1_OC1FE     /* 1   = CH1 compare fast mode enabled */
        | ! TIM_CCMR1_CC1S      /* 00  = CH1 is output */
        ),
    D16(TIM1, CCMR2,
        0
        |   TIM_CCMR2_OC4M_PWM1 /* 110 = CH4 PWM Mode 1 */
        |   TIM_CCMR2_OC4PE     /* 1   = CH4 preload register in use */
        |   TIM_CCMR2_OC4FE     /* 1   = CH4 compare fast mode enabled */
        | ! TIM_CCMR2_CC4S      /* 00  = CH4 is output */
        |   TIM_CCMR2_OC3M_PWM1 /* 110 = CH3 PWM Mode 1 */
        |   TIM_CCMR2_OC3PE     /* 1   = CH3 preload register in use */
        |   TIM_CCMR2_OC3FE     /* 1   = CH3 compare fast mode enabled */
        | ! TIM_CCMR2_CC3S      /* 00  = CH3 is output */
        ),
    D16(TIM1, CCER,
        0
        | ! TIM_CCER_CC4P       /* 0   = OC4  active high */
        |   TIM_CCER_CC4E       /* 1   = OC4  is actively output */
        | ! TIM_CCER_CC3NP      /* 0   = OC3N active high */
        | ! TIM_CCER_CC3NE      /* 0   = OC3N is not output */
        | ! TIM_CCER_CC3P       /* 0   = OC3  active high */
        |   TIM_CCER_CC3E       /* 1   = OC3  is actively output */
        | ! TIM_CCER_CC2NP      /* 0   = OC2N active high */
        | ! TIM_CCER_CC2NE      /* 0   = OC2N is not output */
        | ! TIM_CCER_CC2P       /* 0   = OC2  active high */
        |   TIM_CCER_CC2E       /* 1   = OC2  is actively output */
        | ! TIM_CCER_CC1NP      /* 0   = OC1N active high */
        | ! TIM_CCER_CC1NE      /* 0   = OC1N is not output */
        | ! TIM_CCER_CC1P       /* 0   = OC1  active high */
        |   TIM_CCER_CC1E       /* 1   = OC1  is actively output */
        ),
    D16(TIM1, BDTR,
        0
        |   TIM_BDTR_MOE        /* 1   = Main output enable */
        |   TIM_BDTR_AOE        /* 1   = Automatic output enable */
        | ! TIM_BDTR_BKP        /* 0   = BRK is active low, default value */
        | ! TIM_BDTR_BKE        /* 0   = Break inputs disabled */
        |   TIM_BDTR_OSSR       /* 1   = OC/OCN outputs are enabled */
        |   TIM_BDTR_OSSI       /* 1   = OC/OCN outputs are forced to idle level when inactive */
        | ! TIM_BDTR_LOCK       /* 00  = No bits are write proteced */
        | ! TIM_BDTR_DTG        /* 0   = No dead time */
        ),

    D16(TIM1, PSC,   383),      /* 48MHz / (383+1) = 125 kHz (125 000 Hz) */
    D16(TIM1, ARR,   255),      /* 125 kHz / 256 = 488 Hz */
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D16(TIM1, CCR1,  0),        /* Boot with 0% duty */
    D16(TIM1, CCR2,  0),        /* Boot with 0% duty */
    D16(TIM1, CCR3,  0),        /* Boot with 0% duty */
    D16(TIM1, CCR4,  0),        /* Boot with 0% duty */
#endif

    D16(TIM1, CR1,
        0
        | ! TIM_CR1_CKD         /* 00 = Clock divide by 1 */
        |   TIM_CR1_ARPE        /* 1  = Auto-reload preload enabled, ARR buffered */
        | ! TIM_CR1_CMS         /* 00 = Edge-aligned mode */
        | ! TIM_CR1_DIR         /* 0  = Upcounter */
        | ! TIM_CR1_OPM         /* 0  = Continuous */
        | ! TIM_CR1_URS         /* 0  = All UEV sources generate interrupt, if enabled */
        | ! TIM_CR1_UDIS        /* 0  = Update events are generated, shadow registers updated */
        |   TIM_CR1_CEN         /* 1  = Enable the counter */
        ),
};

/*********
 *  SPI  *
 *********/

const struct device_register_init_static_16bit spi_enc28j60[] = {
    D16(SPI2, CR2,
        0
        | ! SPI_CR2_RXDMAEN    /* Disable RX DMA */
        | ! SPI_CR2_TXDMAEN    /* Disable TX DMA */
        |   SPI_CR2_SSOE       /* NSS output automatically managed by hardware */
        | ! SPI_CR2_NSSP       /* Default: NO NSS pulse */
        | ! SPI_CR2_FRF        /* SPI Motorola mode */
        | ! SPI_CR2_ERRIE      /* Error interrupt disabled */
        | ! SPI_CR2_RXNEIE     /* RX buffer not empty interrupt disabled */
        | ! SPI_CR2_TXEIE      /* TX buffer empty interrupt disabled */
        /* 0111: 8 bits data size */
        |   SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
        | ! SPI_CR2_FRXTH      /* 0: RXNE goes high with two bytes */
        | ! SPI_CR2_LDMARX     /* N/A: RX DMA even */
        | ! SPI_CR2_LDMATX     /* N/A: TX DMA even */
        ),
    D16(SPI2, CR1,
        0
        | ! SPI_CR1_CPHA       /* Data at first edge */
        | ! SPI_CR1_CPOL       /* Clock low when idle */
        |   SPI_CR1_MSTR       /* Master mode */
        |   SPI_CR1_BR_1       /* Clock divider 8 */
        | ! SPI_CR1_SPE        /* SPI disabled until used */
        | ! SPI_CR1_LSBFIRST   /* MSB first */

        |   SPI_CR1_SSI        /* Internal NSS high, needed for master mode */
        | ! SPI_CR1_SSM        /* Hardware Slave management enabled */
        | ! SPI_CR1_RXONLY     /* 0: Full duplex */
        | ! SPI_CR1_CRCL       /* 0: N/A (8-bit CRC length) */
        | ! SPI_CR1_CRCNEXT    /* 0: Transmit TX buffer, not CERC */
        | ! SPI_CR1_CRCEN      /* 0: CRC disabled */
        |   SPI_CR1_BIDIOE     /* 1: Output enabled */
        | ! SPI_CR1_BIDIMODE   /* 0: 2-Line (uni)directional data */
        ),
};

/*********
 * USART *
 *********/

const struct device_register_init_static_32bit usart_standby[] = {
    D32(USART1, CR1,
        0
        | ! USART_CR1_EOBIE    /* 0: No smartcard end of block interrupts */
        | ! USART_CR1_RTOIE    /* 0: No receiver timeout interrupts */
        | ! USART_CR1_DEAT     /* 0: RS-485 driver enable not used, no assertion time */
        | ! USART_CR1_DEDT     /* 0: RS-485 driver enable not used, no deassertion time */
        | ! USART_CR1_OVER8    /* 0: Oversampling by 16 */
        | ! USART_CR1_CMIE     /* 0: No character matching interrupts */
        | ! USART_CR1_MME      /* 0: Receiver in active mode permanently */
        /* The Arduino default is 8 data bits, no parity, one stop bit. */
        | ! USART_CR1_M        /* 0: 1 Start bit, 8 data bits */
        | ! USART_CR1_WAKE     /* 0: Not used, active mode permanently */
        | ! USART_CR1_PCE      /* 0: No parity */
        | ! USART_CR1_PS       /* 0: Not used, parity not used */
        | ! USART_CR1_PEIE     /* 0: Not used, parity not used */
        |   USART_CR1_TXEIE    /* 1: Enable transmit interrupts */
        | ! USART_CR1_TCIE     /* 0: Disable transmission complete interrupts */
        |   USART_CR1_RXNEIE   /* 1: Enable receive interrupts */
        | ! USART_CR1_IDLEIE   /* 0: Disable idel interrupt */
        | ! USART_CR1_TE       /* 0: Disable transmitter, XXX TBD */
        |   USART_CR1_RE       /* 1: Enable receiver, effective only once UE will be asserted */
        | ! USART_CR1_UESM     /* 0: Wakeup from stop disabled */
        | ! USART_CR1_UE       /* 0: Do not enable the USART, only configure it at this time */
        ),
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    D32(USART1, CR2,
        0
        | ! USART_CR2_ADD      /* 0x00: Default value, addressing not used */
        | ! USART_CR2_RTOEN    /* 0:    Default value, Receiver timeout disabled */
        | ! USART_CR2_ABRMOD   /* 00:   Default value, autobaud not used */
        | ! USART_CR2_ABREN    /* 0:    Autobaud rate detection disabled */
        | ! USART_CR2_MSBFIRST /* 0:    Default value, LSB first */
        | ! USART_CR2_DATAINV  /* 0:    Default value, normal logic */
        | ! USART_CR2_TXINV    /* 0:    Default value, TX standard logic levels */
        | ! USART_CR2_RXINV    /* 0:    Default value, RX standard logic levels */
        | ! USART_CR2_SWAP     /* 0:    Default value, RX and TX normally connected */
        | ! USART_CR2_LINEN    /* 0:    Default value, LIN mode disabled */
        | ! USART_CR2_STOP     /* 00:   1 stop bit */
        | ! USART_CR2_CLKEN    /* 0:    Default value, SCLK pin disabled */
        | ! USART_CR2_CPOL     /* 0:    Default value, SCLK not used */
        | ! USART_CR2_CPHA     /* 0:    Default value, SCLK not used */
        | ! USART_CR2_LBCL     /* 0:    Default value, SCLK not used */
        | ! USART_CR2_LBDIE    /* 0:    Default value, LIN mode not used */
        | ! USART_CR2_LBDL     /* 0:    Default value, LIN mode not used */
        | ! USART_CR2_ADDM7    /* 0:    Default value, addressing not used */
        ),
#endif
    D32(USART1, CR3,
        0
        | ! USART_CR3_WUFIE    /* 0:    Defalut value, wakeup from stop not used */
        | ! USART_CR3_WUS      /* 00:   Default value, wakeup from stop not used */
        | ! USART_CR3_SCARCNT  /* 000:  Default value, smartcart mode not used */
        | ! USART_CR3_DEP      /* 0:    Default value, driver enable not used */
        | ! USART_CR3_DEM      /* 0:    Default value, driver enable not used */
        | ! USART_CR3_DDRE     /* 0:    Default value, DMA not used */
        |   USART_CR3_OVRDIS   /* 1:    Overrun functionality disabled XXX TBD */
        | ! USART_CR3_ONEBIT   /* 0:    Default value, three sample bit method */
        | ! USART_CR3_CTSIE    /* 0:    Default value, CTS not used */
        | ! USART_CR3_CTSE     /* 0:    Default value, CTS not used */
        | ! USART_CR3_RTSE     /* 0:    Default value, RTS not used */
        | ! USART_CR3_DMAT     /* 0:    Default value, DMA not used */
        | ! USART_CR3_DMAR     /* 0:    Default value, DMA not used */
        | ! USART_CR3_SCEN     /* 0:    Default value, Smartcard mode not used */
        | ! USART_CR3_NACK     /* 0:    Default value, Smartcard mode not used */
        | ! USART_CR3_HDSEL    /* 0:    Default value, halfduplex not used */
        | ! USART_CR3_IRLP     /* 0:    Default value, IrDA not used */
        | ! USART_CR3_IREN     /* 0:    Default value, IrDA not used */
        | ! USART_CR3_EIE      /* 0:    Disable error interrupts, XXX TBD */
        ),
    /* Default to 115200 bauds at 48 MHz clock, see Table 48 on page 589 of RM0091 */
    D32(USART1, BRR, 0x1A1),
#ifdef ENABLE_EXPLICIT_DEFAULT_VALUES
    /* Guard time and prescaler not needed as IrDA nor smartcard are used */
    D32(USART1, GTPR, 0),
    /* Receiver timeout not used */
    D32(USART1, RTOR, 0),
#endif
};

/*********
 *  ADC  *
 *********/
const struct device_register_init_static_32bit adc_start_calibrate[] = {
    D32(ADC1, CR,
        0
        |   ADC_CR_ADCAL       /* 1: Start calibration now */
        | ! ADC_CR_ADSTP       /* 0: Do not attempt to stop ADC conversion  */
        | ! ADC_CR_ADSTART     /* 0: Do not attempt to start ADC conversion  */
        | ! ADC_CR_ADDIS       /* 0: Do not attempt to disable ADC */
        | ! ADC_CR_ADEN        /* 0: Do not attempt to enable ADC */
        ),
};

/* Wait for the ADC calibrate to finish */
const struct device_register_init_waited_32bit adc_wait_for_calibrate[] = {
    W32(ADC1, CR, ADC_CR_ADCAL, 0),
};

const struct device_register_init_static_32bit adc_init[] = {
    D32(ADC1, IER,
        0
        | ! ADC_IER_AWDIE      /* 0: No analog watchdog interrupts XXX */
        | ! ADC_IER_OVRIE      /* 0: No overrun interrupts */
        | ! ADC_IER_EOSEQIE    /* 0: No end of sequence interrupst XXX */
        | ! ADC_IER_EOCIE      /* 0: No end of conversion interrupts */
        | ! ADC_IER_EOSMPIE    /* 0: No end of sampling flag interrupts */
        | ! ADC_IER_ADRDYIE    /* 0: No ADC ready interrupts */
        ),
    D32(ADC1, CR,
        0
        | ! ADC_CR_ADCAL       /* 0: Do not attempt to calibrate */
        | ! ADC_CR_ADSTP       /* 0: Do not attempt to stop ADC conversion  */
        | ! ADC_CR_ADSTART     /* 0: Do not attempt to start ADC conversion  */
        | ! ADC_CR_ADDIS       /* 0: Do not attempt to disable ADC */
        |   ADC_CR_ADEN        /* 1: Still enable ADC, yet */
        ),
    D32(ADC1, CFGR1,
        0
        | ! ADC_CFGR1_AWDCH    /* 00000: Default value */
        | ! ADC_CFGR1_AWDEN    /* 0: Analog watchdog disable */
        | ! ADC_CFGR1_AWDSGL   /* 0: Analog watchdog on all channels */
        | ! ADC_CFGR1_DISCEN   /* 0: Discontinuous mode disabled */
        | ! ADC_CFGR1_AUTOFF   /* 0: Auto-off mode disabled */
        | ! ADC_CFGR1_WAIT     /* 0: Wait conversion mode off */
        | ! ADC_CFGR1_CONT     /* 0: Single shot mode */
        |   ADC_CFGR1_OVRMOD   /* 1: ADC_DR is overwritten with latest value */
        | ! ADC_CFGR1_EXTEN    /* 00: Conversion started by software */
        | ! ADC_CFGR1_EXTSEL   /* 000: Default value */
        | ! ADC_CFGR1_ALIGN    /* 0: Right alignment */
        |   ADC_CFGR1_RES_0    /* 01: 10-bits resolution, Arduino default */
        | ! ADC_CFGR1_SCANDIR  /* 0: Upward scan */
        | ! ADC_CFGR1_DMACFG   /* 0: N/A, DMA not used */
        | ! ADC_CFGR1_DMAEN    /* 0: DMA disabled */
        ),
    D32(ADC1, CFGR2,
        0
        |   ADC_CFGR2_JITOFFDIV4 /* 1: Remove jitter when ADC is driven by PCLK/4 */
        | ! ADC_CFGR2_JITOFFDIV2 /* 0: No jitter removal on PCLK/2 */
        ),
    D32(ADC1, SMPR, 0),          /* 1.5 ADC clock cycles sampling time */
    D32(ADC1, CHSELR,
        0
        | ! ADC_CHSELR_CHSEL17 /* 0: Channel 17 not selected for conversion */
        | ! ADC_CHSELR_CHSEL16 /* 0: Channel 16 not selected for conversion */
        | ! ADC_CHSELR_CHSEL15 /* 0: Channel 15 not selected for conversion */
        | ! ADC_CHSELR_CHSEL14 /* 0: Channel 14 not selected for conversion */
        | ! ADC_CHSELR_CHSEL13 /* 0: Channel 13 not selected for conversion */
        | ! ADC_CHSELR_CHSEL12 /* 0: Channel 12 not selected for conversion */
        | ! ADC_CHSELR_CHSEL11 /* 0: Channel 11 not selected for conversion */
        | ! ADC_CHSELR_CHSEL10 /* 1: Channel 10 not selected for conversion */
        | ! ADC_CHSELR_CHSEL9  /* 1: Channel  9 not selected for conversion */
        | ! ADC_CHSELR_CHSEL8  /* 1: Channel  8 not selected for conversion */
        | ! ADC_CHSELR_CHSEL7  /* 0: Channel  7 not selected for conversion */
        | ! ADC_CHSELR_CHSEL6  /* 0: Channel  6 not selected for conversion */
        | ! ADC_CHSELR_CHSEL5  /* 0: Channel  5 not selected for conversion */
        | ! ADC_CHSELR_CHSEL4  /* 0: Channel  4 not selected for conversion */
        | ! ADC_CHSELR_CHSEL3  /* 0: Channel  3 not selected for conversion */
        | ! ADC_CHSELR_CHSEL2  /* 0: Channel  2 not selected for conversion */
        | ! ADC_CHSELR_CHSEL1  /* 0: Channel  1 not selected for conversion */
        | ! ADC_CHSELR_CHSEL0  /* 0: Channel  0 not selected for conversion */
        ),
};

const struct device_register_init_static_32bit adc_init2[] = {
    D32(ADC, CCR,
        0
        | ! ADC_CCR_VBATEN     /* 0: VBAT channel disabled */
        | ! ADC_CCR_TSEN       /* 0: Temperature sensor disabled */
        | ! ADC_CCR_VREFEN     /* 0: VREFINT channel disable */
        ),
};

/*
 * Master initialisation table
 */
const device_register_init_descriptor_t dri_tables[] = {
    DRI_DESCRIPTOR_MASKED_32BIT(RCC,   reset_and_clock_control),
    DRI_DESCRIPTOR_MASKED_32BIT(NVIC,  nvic),
    DRI_DESCRIPTOR_STATIC_32BIT(ADC1,  adc_start_calibrate),
    DRI_DESCRIPTOR_STATIC_16BIT(SPI2,  spi_enc28j60),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOA, general_purpose_io_a),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOB, general_purpose_io_b),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOC, general_purpose_io_c),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOD, general_purpose_io_d),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOF, general_purpose_io_f),
    DRI_DESCRIPTOR_STATIC_16BIT(TIM1,  arduino_pwm_output),
    DRI_DESCRIPTOR_STATIC_16BIT(TIM2,  arduino_pwm_output),
    DRI_DESCRIPTOR_STATIC_16BIT(TIM3,  arduino_pwm_output),
    DRI_DESCRIPTOR_STATIC_16BIT(TIM14, arduino_pwm_output),
    DRI_DESCRIPTOR_STATIC_16BIT(TIM15, arduino_pwm_output),

#if 0
    DRI_DESCRIPTOR_STATIC_32BIT(USART1, usart_standby),
    DRI_DESCRIPTOR_STATIC_32BIT(USART2, usart_standby),
#endif
    DRI_DESCRIPTOR_WAITED_32BIT(ADC1,  adc_wait_for_calibrate),
    DRI_DESCRIPTOR_STATIC_32BIT(ADC1,  adc_init),
    DRI_DESCRIPTOR_STATIC_32BIT(ADC,   adc_init2),
};

/*
 * The APB bus appears to be a 32-bit bus, always using 32-bit access.
 * There are four sets of evidence for this.  Firstly, using the
 * 32-bit writes below work even for 16-bit registers.  Secondly, all
 * the 16-bit registers are defined as the high-order 16 bits being
 * reserved.  Thirdly, at least some of the APB peripherial
 * descriptions say the following:
 *
 * RM0091, Doc ID 018940 Rev 2, page 332:
 *
 * The peripheral registers can be accessed by half-words (16-bit)
 * or words (32-bit).
 *
 * Finally, when describing the AHB2APB bridge and then the DMA
 * controller access to the APB devices, the manual says the following:
 *
 * RM0091, Doc ID 018940 Rev 2, page 36:
 *
 * Note: When a 16- or 8-bit access is performed on an APB register,
 * the access is transformed into a 32-bit access: the bridge
 * duplicates the 16- or 8-bit data to feed the 32-bit vector.
 *
 * RM0091, Doc ID 018940 Rev 2, page 149:
 *
 * Assuming that the AHB/APB bridge is an AHB 32-bit slave peripheral
 * that does not take the HSIZE data into account, it will transform
 * any AHB byte or halfword operation into a 32-bit APB operation in
 * the following manner:
 * - an AHB byte write operation of the data "0xB0" to 0x0 (or to 0x1,
 *   0x2 or 0x3) will be converted to an APB word write operation of
 *   the data "0xB0B0B0B0" to 0x0
 * - an AHB halfword write operation of the data "0xB1B0" to 0x0 (or
 *   to 0x2) will be converted to an APB word write operation of the
 *   data "0xB1B0B1B0" to 0x0
 *
 * Consequently, our current assumption is that it is safe to write
 * even the 16-bit APB registers with 32-bit writes, and we do so.
 */

static inline void
Config_Static16(void *device, const device_register_init_static_16bit_t *values, const int count) {
    const device_register_init_static_16bit_t *p;

    for (p = values; p < values + count; p++) {
        *((uint32_t *)(((char *)device) + p->offset)) = p->value;
    }
}

static inline void
Config_Static32(void *device, const device_register_init_static_32bit_t *values, const int count) {
    const device_register_init_static_32bit_t *p;

    for (p = values; p < values + count; p++) {
        *((uint32_t *)(((char *)device) + p->offset)) = p->value;
    }
}

static inline void
Config_Masked32(void *device, const device_register_init_masked_32bit_t *values, const int count) {
    const device_register_init_masked_32bit_t *p;

    for (p = values; p < values + count; p++) {
        register uint32_t *const reg               /* Pointer to the device register */
            = (uint32_t *)(((char *)device) + p->offset);

        // XXX: assert that no bits in p->value are outside of p->mask

        register uint32_t tmp;

        tmp  = *reg;
        tmp &= ~p->mask;
        tmp |=  p->value;
        *reg = tmp;
    }
}

/*
 * Wait for the given words to have the given masked values
 */
static inline void
Config_Waited32(void *device, const device_register_init_masked_32bit_t *values, const int count) {
    const device_register_init_masked_32bit_t *p;

    for (p = values; p < values + count; p++) {
        register const uint32_t value = p->value;  /* Waited for value */
        register uint32_t * const reg              /* Pointer to the device register */
            = (uint32_t *)(((char *)device) + p->offset);

        register uint32_t tmp;

        do {
            tmp  = *reg;
            tmp &= ~p->mask;
        } while (value != tmp);
    }
}

void
Peripheral_Init(void) {
    for (unsigned int i = 0; i < COUNT_OF(dri_tables); i++) {
        const device_register_init_descriptor_t *dri = &dri_tables[i];
        switch (dri->dri_type) {
        case DRI_STATIC_16BIT:
            Config_Static16(dri->dri_device, dri->u.dri_static_16bit, dri->dri_count);
            break;
        case DRI_STATIC_32BIT:
            Config_Static32(dri->dri_device, dri->u.dri_static_32bit, dri->dri_count);
            break;
        case DRI_MASKED_32BIT:
            Config_Masked32(dri->dri_device, dri->u.dri_masked_32bit, dri->dri_count);
            break;
        case DRI_WAITED_32BIT:
            Config_Waited32(dri->dri_device, dri->u.dri_masked_32bit, dri->dri_count);
            break;
        default:
            abort();
            break;
        }
    }
}

