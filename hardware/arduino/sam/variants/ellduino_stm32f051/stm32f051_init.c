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

#define D16(d, r, v)    DEVICE_REGISTER_INIT_STRUCT_VALUE16((d), r, (v))
#define D32(d, r, v)    DEVICE_REGISTER_INIT_STRUCT_VALUE32((d), r, (v))
#define M32(d, r, m, v) DEVICE_REGISTER_INIT_STRUCT_MASK_VALUE32((d), r, (m), (v))
#define W32(d, r, m, v) DEVICE_REGISTER_INIT_STRUCT_WAIT_VALUE32((d), r, (m), (v))


/* XXX: Do we really need mask here?   */
const struct device_register_init_masked_32bit reset_and_clock_control[] = {
    M32(RCC, APB1ENR,
        RCC_APB1ENR_TIM2EN   | RCC_APB1ENR_TIM3EN,
        RCC_APB1ENR_TIM2EN   | RCC_APB1ENR_TIM3EN),
    M32(RCC, APB2ENR,
        RCC_APB2ENR_TIM1EN   | RCC_APB2ENR_SYSCFGEN | RCC_APB2ENR_SPI1EN | RCC_APB2ENR_ADC1EN |
        RCC_APB2ENR_TIM15EN  | RCC_APB2ENR_TIM16EN  | RCC_APB2ENR_TIM17EN,
        RCC_APB2ENR_TIM1EN   | RCC_APB2ENR_SYSCFGEN | RCC_APB2ENR_SPI1EN | RCC_APB2ENR_ADC1EN |
        RCC_APB2ENR_TIM15EN  | RCC_APB2ENR_TIM16EN  | RCC_APB2ENR_TIM17EN),
    M32(RCC, AHBENR,
        RCC_AHBENR_GPIOAEN   | RCC_AHBENR_GPIOBEN   | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOFEN,
        RCC_AHBENR_GPIOAEN   | RCC_AHBENR_GPIOBEN   | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOFEN),
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
    ENABLE_INTERRUPT(TIM3_IRQn, 3),
    ENABLE_INTERRUPT(SPI1_IRQn, 3), /* Lowest priority */
};

/* Port A */
const struct device_register_init_static_32bit general_purpose_io_a[] = {
    D32(GPIOA, MODER,
        0
        | ! GPIO_MODER_MODER0    /* 00: PA0  Input  USER */
        | ! GPIO_MODER_MODER1    /* 00: PA1  Input  INT */
        |   GPIO_MODER_MODER2_1  /* 10: PA2  AF     TIM15 CH1 */
        |   GPIO_MODER_MODER3_0  /* 01: PA3  Output RESET */
        |   GPIO_MODER_MODER4_1  /* 10: PA4  AF     SPI NSS */
        |   GPIO_MODER_MODER5_1  /* 10: PA5  AF     SPI CLK */
        |   GPIO_MODER_MODER6_1  /* 10: PA6  AF     SPI MISO */
        |   GPIO_MODER_MODER7_1  /* 10: PA7  AF     SPI MOSI */
        |   GPIO_MODER_MODER9_1  /* 10: PA9  AF     USART1 TX */
        |   GPIO_MODER_MODER10_1 /* 10: PA10 AF     USART1_RX */
        |   GPIO_MODER_MODER13_1 /* 10: PA13 AF     SWDAT, reset-time value */
        |   GPIO_MODER_MODER14_1 /* 10: PA14 AF     SWCLK, reset-time value */
        ),
    D32(GPIOA, OTYPER,
        0
        | ! GPIO_OTYPER_OT_2     /* PA2 AF output   Push-pull */
        | ! GPIO_OTYPER_OT_3     /* PA3 GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_4     /* PA4 SPI NSS     Push-pull */
        | ! GPIO_OTYPER_OT_5     /* PA5 SPI CLK     Push-pull */
        | ! GPIO_OTYPER_OT_7     /* PA7 SPI MOSI    Push-pull */
        | ! GPIO_OTYPER_OT_9     /* PA9 USART TX    Push-pull */
        ),
    /* See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet */
    D32(GPIOA, OSPEEDR,
        0
        |   GPIO_OSPEEDER_OSPEEDR2
        |   GPIO_OSPEEDER_OSPEEDR3 
        |   GPIO_OSPEEDER_OSPEEDR4
        |   GPIO_OSPEEDER_OSPEEDR5
        |   GPIO_OSPEEDER_OSPEEDR7
        |   GPIO_OSPEEDER_OSPEEDR9
        |   GPIO_OSPEEDER_OSPEEDR13 /* Reset-time value */
        | ! GPIO_OSPEEDER_OSPEEDR14 /* Reset-time value */
        ),
    D32(GPIOA, PUPDR,
        0
        |   GPIO_PUPDR_PUPDR0_1  /* PA0  <- USER    pull down */
        |   GPIO_PUPDR_PUPDR1_0  /* PA1  <- INT     pull up */
        | ! GPIO_PUPDR_PUPDR2    /* PA2  -> Blue    no pull resistors */
        | ! GPIO_PUPDR_PUPDR3    /* PA3  -> RESET   no pull resistors */
        | ! GPIO_PUPDR_PUPDR4    /* PA4  -> CS      no pull resistors */
        | ! GPIO_PUPDR_PUPDR5    /* PA5  -> SCK     no pull resistors */
        |   GPIO_PUPDR_PUPDR6_1  /* PA6  <- MISO    pull down */
        | ! GPIO_PUPDR_PUPDR7    /* PA7  -> MOSI    no pull resistors */
        | ! GPIO_PUPDR_PUPDR9    /* PA9  -> TX      no pull resistors */
        |   GPIO_PUPDR_PUPDR10_1 /* PA10 <- RX      pull down */
        |   GPIO_PUPDR_PUPDR13_0 /* PA13 <> SWDAT   pull up,   reset-time value */
        |   GPIO_PUPDR_PUPDR14_1 /* PA10 <- SWCLK   pull down, reset-time value */
        ),
    D32(GPIOA, AFR[0],
        0
        | ! GPIO_AFRL_AFRL2      /* PA2  AF0 TIM15_CH1 */
        | ! GPIO_AFRL_AFRL4      /* PA4  AF0 SPI */
        | ! GPIO_AFRL_AFRL5      /* PA5  AF0 SPI */
        | ! GPIO_AFRL_AFRL6      /* PA6  AF0 SPI */
        | ! GPIO_AFRL_AFRL7      /* PA7  AF0 SPI */
        ),
    D32(GPIOA, AFR[1],
        0
#define GPIO_AFRL_AFRL1_AF1            ((uint32_t)0x00000010)
#define GPIO_AFRL_AFRL2_AF1            ((uint32_t)0x00000100)
        |   GPIO_AFRL_AFRL1_AF1  /* PA9  AF1 TX */
        |   GPIO_AFRL_AFRL2_AF1  /* PA10 AF1 RX */
        | ! GPIO_AFRL_AFRL5      /* PA13 AF0 SWD, reset-time value */
        | ! GPIO_AFRL_AFRL6      /* PA14 AF0 SWC, reset-time value */
        ),
};

/* Port B */
const struct device_register_init_static_32bit general_purpose_io_b[] = {
    D32(GPIOB, MODER,
        0
        |   GPIO_MODER_MODER1_1 | GPIO_MODER_MODER1_0
                                  /* 11: PB1   Analog */
        |   GPIO_MODER_MODER8_1   /* 10: PB8   AF */
        |   GPIO_MODER_MODER9_1   /* 10: PB9   AF */
        |   GPIO_MODER_MODER13_1  /* 10: PB13  AF */
        |   GPIO_MODER_MODER14_1  /* 10: PB14  AF */
        |   GPIO_MODER_MODER15_1  /* 10: PB15  AF */
        ),
    D32(GPIOB, OTYPER,
        0
        | ! GPIO_OTYPER_OT_8      /* PB8  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_9      /* PB9  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_13     /* PB13 GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_14     /* PB14 GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_15     /* PB15 GPIO output Push-pull */
        ),
    /* See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet */
    D32(GPIOB, OSPEEDR,
        0
        | ! GPIO_OSPEEDER_OSPEEDR8
        | ! GPIO_OSPEEDER_OSPEEDR9
        | ! GPIO_OSPEEDER_OSPEEDR13
        | ! GPIO_OSPEEDER_OSPEEDR14
        | ! GPIO_OSPEEDER_OSPEEDR15
        ),
    D32(GPIOB, PUPDR,
        0
        | ! GPIO_PUPDR_PUPDR1   /* PB1  no pull resistors */
        | ! GPIO_PUPDR_PUPDR8   /* PB8  no pull resistors */
        | ! GPIO_PUPDR_PUPDR9   /* PB9  no pull resistors */
        | ! GPIO_PUPDR_PUPDR13  /* PB13 no pull resistors */
        | ! GPIO_PUPDR_PUPDR14  /* PB14 no pull resistors */
        | ! GPIO_PUPDR_PUPDR15  /* PB15 no pull resistors */
        ),
#define GPIO_AFRL_AFRL0_AF2            ((uint32_t)0x00000002)
#define GPIO_AFRL_AFRL1_AF2            ((uint32_t)0x00000020)
#define GPIO_AFRL_AFRL2_AF2            ((uint32_t)0x00000200)
#define GPIO_AFRL_AFRL3_AF2            ((uint32_t)0x00002000)
#define GPIO_AFRL_AFRL4_AF2            ((uint32_t)0x00020000)
#define GPIO_AFRL_AFRL5_AF2            ((uint32_t)0x00200000)
#define GPIO_AFRL_AFRL6_AF2            ((uint32_t)0x02000000)
#define GPIO_AFRL_AFRL7_AF2            ((uint32_t)0x20000000)
    D32(GPIOB, AFR[1],
        0
        |   GPIO_AFRL_AFRL0_AF2 /* PB8  AF2 TIM16_CH1  */
        |   GPIO_AFRL_AFRL1_AF2 /* PB9  AF2 TIM17_CH1  */
        |   GPIO_AFRL_AFRL5_AF2 /* PB13 AF2 TIM1_CH1N */
        |   GPIO_AFRL_AFRL6_AF2 /* PB14 AF2 TIM1_CH2N */
        |   GPIO_AFRL_AFRL7_AF2 /* PB15 AF2 TIM1_CH3N */
        ),
};

/* Port C -- Eval and debug functions, TIM15 */
const struct device_register_init_static_32bit general_purpose_io_eval[] = {
    D32(GPIOC, MODER,
        0
        |   GPIO_MODER_MODER0_1 | GPIO_MODER_MODER0_0
                                 /* 10: PC0  Analog */
        |   GPIO_MODER_MODER6_0  /* 01: PC6  Output Led4 */
        |   GPIO_MODER_MODER7_0  /* 01: PC7  Output Led4 */
        |   GPIO_MODER_MODER8_0  /* 01: PC8  Output Led4 */
        |   GPIO_MODER_MODER9_0  /* 01: PC9  Output Led3 */
        |   GPIO_MODER_MODER10_0 /* 01: PC10 Output Led2 */
        |   GPIO_MODER_MODER11_0 /* 01: PC11 Output Led1 */
        |   GPIO_MODER_MODER12_0 /* 01: PC12 Output Led0 */
        | ! GPIO_MODER_MODER13   /* 00: PC13 Input  WOL  */
        ),
    D32(GPIOC, OTYPER,
        0
        | ! GPIO_OTYPER_OT_6     /* PC6  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_7     /* PC7  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_8     /* PC8  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_9     /* PC9  GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_10    /* PC10 GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_11    /* PC11 GPIO output Push-pull */
        | ! GPIO_OTYPER_OT_12    /* PC12 GPIO output Push-pull */
        ),
    /* See page 74 of DM39193 Doc ID 022265 Rev 3 STD32F051x data sheet */
    D32(GPIOC, OSPEEDR,
        0
        | ! GPIO_OSPEEDER_OSPEEDR6  /* 00: PC6 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR7  /* 00: PC7 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR8  /* 00: PC8 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR9  /* 00: PC9 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR10 /* 00: PC10 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR11 /* 00: PC11 GPIO */
        | ! GPIO_OSPEEDER_OSPEEDR12 /* 00: PC12 GPIO */
        ),
    D32(GPIOC, PUPDR,
        0
        | ! GPIO_PUPDR_PUPDR6    /* PC6  -> Led6    no pull resistors */
        | ! GPIO_PUPDR_PUPDR7    /* PC7  -> Led5    no pull resistors */
        | ! GPIO_PUPDR_PUPDR8    /* PC8  -> Led4    no pull resistors */
        | ! GPIO_PUPDR_PUPDR9    /* PC9  -> Led3    no pull resistors */
        | ! GPIO_PUPDR_PUPDR10   /* PC9  -> Led2    no pull resistors */
        | ! GPIO_PUPDR_PUPDR11   /* PC9  -> Led1    no pull resistors */
        | ! GPIO_PUPDR_PUPDR12   /* PC9  -> Led0    no pull resistors */
        | ! GPIO_PUPDR_PUPDR13_0 /* PC13 <- WOL     pull up*/
        ),
};

/*
 * Timer-counter for LED buck converter.
 */
const struct device_register_init_static_16bit rgb_buck_pwm[] = {
    D16(TIM15, CR1, 0),         /* Disable the counter */
    // D16(TIM15, CR2, 0),      /* Default value */
    // D16(TIM16, SMCR, 0),     /* Default value */
    // D16(TIM15, DIER, 0),     /* Default value */
    D16(TIM15, CCMR1,
        0
        |   TIM_CCMR1_OC1M_PWM2 /* 111 = CH1 PWM Mode 2 */
        |   TIM_CCMR1_OC1PE     /* 1   = CH1 preload register in use */
        |   TIM_CCMR1_OC1FE     /* 1   = CH1 compare fast mode enabled */
        | ! TIM_CCMR1_CC1S      /* 00  = CH1 is output */
        ),
    // D16(TIM15, CCMR2, 0),    /* Default value */
    D16(TIM15, CCER,
        0
        | ! TIM_CCER_CC1NP      /* 0   = OC1N active high */
        | ! TIM_CCER_CC1NE      /* 0   = OC1N is not output */
        | ! TIM_CCER_CC1P       /* 0   = OC1  active high */
        |   TIM_CCER_CC1E       /* 1   = OC1  is actively output */
        ),
    D16(TIM15, BDTR,
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

    D16(TIM15, PSC,     4),     /* 48MHz / (4+1) = approx 10 MHz (9.60 MHz) */
    D16(TIM15, ARR,   480),     /* 9.6 MHz / 480 = 20 kHz */
    D16(TIM15, CCR1,  481),     /* Boot with 0% duty */

    D16(TIM15, CR1,
        0
        | ! TIM_CR1_CKD         /* 00 = Clock divide by 1 */
        |   TIM_CR1_ARPE        /* 1  = Auto-reload preload enabled, ARR buffered */
        | ! TIM_CR1_OPM         /* 0  = Continuous */
        | ! TIM_CR1_URS         /* 0  = All UEV sources generate interrupt, if enabled */
        | ! TIM_CR1_UDIS        /* 0  = Update events are generated, shadow registers updated */
        |   TIM_CR1_CEN         /* 1  = Enable the counter */
        ),
};

#if 0
XXX Doesnt work currently, must fix SPI interrupt tolerance first
/*
 * Timer-counter for interrupt timer
 */
const struct device_register_init_static_16bit interrupt_timer[] = {
    D16(TIM3, CR1, 0),        /* Disable the counter */
    D16(TIM3, CR2,
        0
        | ! TIM_CR2_TI1S      /* 0 = CH1 is connected to TI1 */
        | ! TIM_CR2_MMS       /* 000 = "Reset." TRGO from UG */
        | ! TIM_CR2_CCDS      /* 0 = DMA when CCX event */
        ),
    D16(TIM3, SMCR, 0),
    D16(TIM3, DIER,
        0
        |   TIM_DIER_UIE       /* 1 = Update interrupt enabled */
        ),
    D16(TIM3, CCMR1, 0),
    D16(TIM3, CCMR2, 0),
    D16(TIM3, CCER, 0),

    /*
     * Note that TIM3 ARR and CCR registers are 32-bits long, but here we
     * initialize them with only 16-bits long values
     */
    D16(TIM3, ARR,   1000),   /* 1 MHz / 20 = 50 kHz  XXX */
    D16(TIM3, PSC,    47),    /* 48MHz / 48  = 1 MHz */

    D16(TIM3, CR1,
        0
        | ! TIM_CR1_CKD       /* 00 = Clock divide by 1 */
        |   TIM_CR1_ARPE      /* 1  = Auto-reload preload enabled, ARR buffered */
        | ! TIM_CR1_CMS       /* 00 = Edge aligned mode */
        |   TIM_CR1_DIR       /* 1  = Down counter */
        | ! TIM_CR1_OPM       /* 0  = Continuous */
        | ! TIM_CR1_URS       /* 0  = All UEV sources */
        | ! TIM_CR1_UDIS      /* 0  = Enable UEV */
        |   TIM_CR1_CEN       /* Enable the counter */
        ),
};
#endif

/* SPI */
const struct device_register_init_static_16bit spi[] = {
    D16(SPI1, CR2,
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
    D16(SPI1, CR1,
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

/* ADC */
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
        |   ADC_CR_ADEN        /* 0: Enable ADC */
        ),
    D32(ADC1, CFGR1,
        0
        | ! ADC_CFGR1_AWDCH    /* 00000: Default value */
        | ! ADC_CFGR1_AWDEN    /* 0: Analog watchdog disable */
        | ! ADC_CFGR1_AWDSGL   /* 0: Analog watchdog on all channels */
        | ! ADC_CFGR1_DISCEN   /* 0: Discontinuous mode disabled */
        | ! ADC_CFGR1_AUTOFF   /* 0: Auto-off mode disabled */
        | ! ADC_CFGR1_WAIT     /* 0: Wait conversion mode off */
        |   ADC_CFGR1_CONT     /* 1: Continuous conversion mode */
        |   ADC_CFGR1_OVRMOD   /* 1: ADC_DR is overwritten with latest value */
        | ! ADC_CFGR1_EXTEN    /* 00: Conversion started by software */
        | ! ADC_CFGR1_EXTSEL   /* 000: Default value */
        | ! ADC_CFGR1_ALIGN    /* 0: Right alignment */
        | ! ADC_CFGR1_RES      /* 00: 12-bits resolution */
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
        |   ADC_CHSELR_CHSEL10 /* 1: Channel 10 selected for conversion Blue  */
        |   ADC_CHSELR_CHSEL9  /* 1: Channel  9 selected for conversion Green */
        |   ADC_CHSELR_CHSEL8  /* 1: Channel  8 selected for conversion Red   */
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
    DRI_DESCRIPTOR_STATIC_16BIT(SPI1,  spi),
#if 0
    DRI_DESCRIPTOR_STATIC_16BIT(TIM3,  interrupt_timer),
#endif
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOA, general_purpose_io_a),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOB, general_purpose_io_b),
    DRI_DESCRIPTOR_STATIC_32BIT(GPIOC, general_purpose_io_eval),
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
        case DRI_WAITED_32BIT:
            Config_Waited32(dri->dri_device, dri->u.dri_masked_32bit, dri->dri_count);
            break;
        case DRI_MASKED_32BIT:
            Config_Masked32(dri->dri_device, dri->u.dri_masked_32bit, dri->dri_count);
            break;
        case DRI_STATIC_32BIT:
            Config_Static32(dri->dri_device, dri->u.dri_static_32bit, dri->dri_count);
            break;
        case DRI_STATIC_16BIT:
            Config_Static16(dri->dri_device, dri->u.dri_static_16bit, dri->dri_count);
            break;
        default:
            abort();
            break;
        }
    }

    Config_Static16(TIM15, rgb_buck_pwm, COUNT_OF(rgb_buck_pwm));
    /* Start next timers suitably delayed */
    while (TIM15->ARR < 120)
        ;

    Config_Static16(TIM16, rgb_buck_pwm, COUNT_OF(rgb_buck_pwm));
    /* Start next timers suitably delayed */
    while (TIM16->ARR < 120)
        ;

    Config_Static16(TIM17, rgb_buck_pwm, COUNT_OF(rgb_buck_pwm));
}

