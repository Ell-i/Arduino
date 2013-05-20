/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_ANALOG_H
#define ELLDUINO_WIRING_ANALOG_H

#ifdef __cplusplus
extern "C" {
#endif

inline
void analogWrite(uint32_t pin, uint32_t value) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    const stm32_pinDescription_t * const pinp = &stm32_pinDescription[pin];

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

inline
uint32_t analogRead(uint32_t pin) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    const stm32_pinDescription_t * const pinp = &stm32_pinDescription[pin];

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

#ifdef __cplusplus
}
#endif

#endif /* ELLDUINO_WIRING_ANALOG_H */
