/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_TONE_H
#define ELLDUINO_WIRING_TONE_H

#ifdef __cplusplus
extern "C" {

static inline
void tone(pin_t pin, unsigned int frequency, unsigned long duration=0) {
    setPWMFrequency(pin, frequency, 8);
    analogWrite(pin, 128);
    // XXX Duration not handled
}

static inline
void noTone(pin_t pin) {
    setPWMResolution(pin, 8);
    noAnalogWrite(pin);
}

}
#endif /* __cplusplus */

#endif /* ELLDUINO_WIRING_TONE_H */
