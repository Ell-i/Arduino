/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_ANALOG_H
#define ELLDUINO_WIRING_ANALOG_H

#ifdef __cplusplus
extern "C" {
#endif

enum analog_reference_t {
    DEFAULT,
    INTERNAL1V1,
    INTERNAL2V56,
    INTERNAL,
    EXTERNAL,
};

#define analogReference(reference) analogReference ## reference ()
static inline 
void analogReferenceDEFAULT() {
    // Do nothing
}
static inline
void analogReferenceINTERNAL() {
    // Do nothing
}
#define analogReferenceINTERNAL1V1  error: not supported
#define analogReferenceINTERNAL2V56 error: not supported
#define analogReferenceEXTERNAL     error: not supported

/*
  Look at bits 1 and 2 of res:
     6 = 0110B => 11B -> 11B => t[3] = 3
     8 = 1000B => 00B -> 10B => t[0] = 2
    10 = 1010B => 01B -> 01B => t[1] = 1
    12 = 1100B => 10B -> 00B => t[2] = 0
*/
static const uint8_t analogReadResolutionMapping[] = { 2, 1, 0, 3 };

static inline
void analogReadResolution(int res) {
    /* With a constant argument, evalutes as a constant expression  */
    const register unsigned int mode = analogReadResolutionMapping[(res & 0x6) >> 1];
    
    ADC1->CFGR1 &= ~ADC_CFGR1_RES;
    ADC1->CFGR1 |=  ADC_CFGR1_RES_0 * mode; /* Should become optimised to a shift. */
}

static inline
uint32_t analogRead(pin_t pin) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    const stm32_pinDescription_t * const pinp = &stm32_pinDescription[pin];

    /* Configure the I/O pin to analog mode.  Idempontent. */
    *(pinp->pin_port_moder) |=  (GPIO_MODER_MODER0 << (pin_number * 2));

    /* Configure the selection channel */
    ADC1->CHSELR = 1 << pinp->pin_adc_channel;

    /* Wait for ADC to be ready */
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
#ifdef DEBUG
      GPIOC->ODR ^= GPIO_ODR_9;
      GPIOC->ODR ^= GPIO_ODR_7;
#endif
      yield();
    }

    /* Start conversion */
    ADC1->CR |= ADC_CR_ADSTART;

    /* Wait for the ADC to complete conversion */
    while (!(ADC1->ISR & ADC_ISR_EOC)) {
#ifdef DEBUG
      GPIOC->ODR ^= GPIO_ODR_9;
      GPIOC->ODR ^= GPIO_ODR_6;
#endif
      yield();
    }

    /* Return the data */
    return ADC1->DR;
}

/* 
 * The default PWM mode, initialised in stm32fxxx_init.c, is to use 125 kHz timer
 * frequency, which gives 490 Hz at 256 cycles, which is the Arduino default.
 * The inline analogWrite assume this, simply writing the value to CCR.
 *
 * Hence, when the resolution is changed, we have to change the prescaler on
 * all used PWM timers.  
 *
 * Our resolution frequency of 48000000 / 98304 = 488 Hz.  With this base
 * frequency, the prescaler is set as follows for the different resolutions.
 *   Resolution    PSC    ARR
 *       0       98304      1
 *       1       49152      2
 *       2       24576      4
 *       3       12288      8
 *       4        6144     16
 *       5        3072     32
 *       6        1536     64
 *       7         768    128
 *       8         384    256
 *       9         192    512
 *      10          96   1024
 *      11          48   2048
 *      12          24   4096
 *      13          12   8192
 *      14           6  16384
 *      15           3  32768
 *
 * Note that the resolution and frequency setting is usually performed from setup().
 * Hence, we ignore the potential race condition which may happen if the resolution
 * is changed while the timer is running, i.e. if PSC is updated first and the
 * target value only afterwards.
 */

const static uint32_t BASE_FREQUENCY = (F_CPU / 125000) << 8; /* 384 << 8 == 98 304*/

static inline
void analogWriteResolution(uint32_t resolution) {
    for (int i = 0; i < COUNT_OF(stm32_arduino_pwm_timers); i++) {
        stm32_arduino_pwm_timers[i]->PSC  = (BASE_FREQUENCY >> resolution) - 1;
        stm32_arduino_pwm_timers[i]->ARR  = 1 << resolution;
    }
}

static inline
void setPWMFrequency(pin_t pin, uint32_t pwm_frequency, uint32_t resolution) {
    const stm32_pinDescription_t *const pinp = &stm32_pinDescription[pin];
    
    const uint32_t counter_top = 1 << resolution;
    const uint32_t resolution_frequency = pwm_frequency * counter_top;
    const uint32_t prescaler = F_CPU / resolution_frequency - 1;

    pinp->pin_pwm_timer->PSC = prescaler;
    pinp->pin_pwm_timer->ARR = counter_top;
}

static inline
void setPWMResolution(pin_t pin, uint32_t resolution) {
    setPWMFrequency(pin, 488, resolution);
}

/*
 * Wiring API prescaler values
 */
static const uint32_t CLOCK_NO_PRESCALE   = 0;
static const uint32_t CLOCK_PRESCALE_8    = 8-1;
static const uint32_t CLOCK_PRESCALE_32   = 32-1;
static const uint32_t CLOCK_PRESCALE_64   = 64-1;
static const uint32_t CLOCK_PRESCALE_128  = 128-1;
static const uint32_t CLOCK_PRESCALE_256  = 256-1;
static const uint32_t CLOCK_PRESCALE_1024 = 1024-1;

static inline
void setPWMPrescale(pin_t pin, uint32_t prescaler) {
    const stm32_pinDescription_t *const pinp = &stm32_pinDescription[pin];

    pinp->pin_pwm_timer->PSC = prescaler;
}

static inline 
void analogOutputInit() {} /* Arduino Due API compatibility.  XXX Check if needed */

static inline
void analogWrite(pin_t pin, uint32_t value) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    const stm32_pinDescription_t * const pinp = &stm32_pinDescription[pin];

    /* Place the GPIO in the Alternate Function mode.  Idempotent. */
    *(pinp->pin_port_moder) &= ~(GPIO_MODER_MODER0_0 << (pin_number * 2));
    *(pinp->pin_port_moder) |=  (GPIO_MODER_MODER0_1 << (pin_number * 2));

    /* Channel counter value */
    *(pinp->pin_pwm_ccr) = value;
}

static inline
void noAnalogWrite(pin_t pin) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    const stm32_pinDescription_t * const pinp = &stm32_pinDescription[pin];

    /* Place the GPIO back to the default mode. */
    *(pinp->pin_port_moder) &= ~(GPIO_MODER_MODER0 << (pin_number * 2));
}

#ifdef __cplusplus
}
#endif

#endif /* ELLDUINO_WIRING_ANALOG_H */
