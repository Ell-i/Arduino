/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_DIGITAL_H
#define ELLDUINO_WIRING_DIGITAL_H

#ifdef __cplusplus
extern "C" {
#endif

/** XXX Hacky.  Replace */

#define pinMode(pin, mode) pinMode_ ## mode(pin)
#define pinMode_uint32_t // EMPTY

inline
void pinMode_INPUT(uint8_t pin) {
    /* Input mode */
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    *(stm32_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0   << (pin_number * 2));
}

inline
void pinMode_OUTPUT(uint8_t pin) {
    /* Output mode */
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    *(stm32_pinDescription[pin].pin_port_moder) |=  (GPIO_MODER_MODER0_0 << (pin_number * 2));
    *(stm32_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0_1 << (pin_number * 2));
}

inline
int digitalRead(uint32_t pin) {
    return *(stm32_pinDescription[pin].pin_port_input) & stm32_pinDescription[pin].pin_mask?
        1 /*HIGH*/: 0 /*LOW*/;
}

inline
void  digitalWrite(uint32_t pin, uint32_t val) {
    if (val)
        *(stm32_pinDescription[pin].pin_port_bsrr) = stm32_pinDescription[pin].pin_mask;
    else
        *(stm32_pinDescription[pin].pin_port_brr)  = stm32_pinDescription[pin].pin_mask;
}

#ifdef __cplusplus
}
#endif

#endif /* ELLDUINO_WIRING_DIGITAL_H */
