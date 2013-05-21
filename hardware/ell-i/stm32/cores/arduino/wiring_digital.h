/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_DIGITAL_H
#define ELLDUINO_WIRING_DIGITAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define pinMode(pin, mode) pinMode ## mode(pin)

static inline
void pinModeINPUT(pin_t pin) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    /* Input mode */
    *(stm32_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0   << (pin_number * 2));
}

static inline
void pinModeOUTPUT(pin_t pin) {
    const uint32_t pin_number = stm32_pinDescription[pin].pin_number;
    /* Output mode */
    *(stm32_pinDescription[pin].pin_port_moder) |=  (GPIO_MODER_MODER0_0 << (pin_number * 2));
    *(stm32_pinDescription[pin].pin_port_moder) &= ~(GPIO_MODER_MODER0_1 << (pin_number * 2));
}

static inline
void pinModeINPUT_PULLUP(pin_t pin) {
    //XXX TBD;
}

static inline
int digitalRead(pin_t pin) {
    return *(stm32_pinDescription[pin].pin_port_input) & stm32_pinDescription[pin].pin_mask?
        1 /*HIGH*/: 0 /*LOW*/;
}

static inline
void  digitalWrite(pin_t pin, uint32_t val) {
    if (val)
        *(stm32_pinDescription[pin].pin_port_bsrr) = stm32_pinDescription[pin].pin_mask;
    else
        *(stm32_pinDescription[pin].pin_port_brr)  = stm32_pinDescription[pin].pin_mask;
}

#ifdef __cplusplus
}
#endif

#endif /* ELLDUINO_WIRING_DIGITAL_H */
