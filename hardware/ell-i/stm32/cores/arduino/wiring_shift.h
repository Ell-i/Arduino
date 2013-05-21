/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_SHIFT_H
#define ELLDUINO_WIRING_SHIFT_H

#ifdef __cplusplus
extern "C"{

enum BitOrder {
    LSBFIRST = 0,
    MSBFIRST = 1,
};

static inline
void shiftOut(const pin_t dataPin, const pin_t clockPin, const enum BitOrder bitOrder,
              uint32_t data, uint8_t count = 8, const uint32_t ms = 0) {
    const uint32_t mask = bitOrder == MSBFIRST? 1 << (count - 1) : 1;

    while (count-- > 0) {
        digitalWrite(dataPin,  data & mask);
        digitalWrite(clockPin, HIGH);
        data = (bitOrder == MSBFIRST? data << 1: data >> 1);
        digitalWrite(clockPin, LOW);
        delay(ms);
    }
}

static inline
uint32_t shiftIn(const pin_t dataPin, const pin_t clockPin, const enum BitOrder bitOrder,
                 uint8_t count = 8, const uint32_t ms = 0) {
    uint32_t data = 0;
    uint32_t mask = bitOrder == MSBFIRST? 1 << (count - 1) : 1;

    while (count-- > 0) {
        digitalWrite(clockPin, HIGH);
        delay(ms);
        if (digitalRead(dataPin))
            data |= mask;
        digitalWrite(clockPin, LOW);
        /* 
         * XXX: This is delay long enough for the LOW time for all chips?  
         * Should we have delay here too?
         */
        mask = (bitOrder == MSBFIRST? mask >> 1: mask << 1);
    }
}

} // extern "C"
#endif

#endif /* ELLDUINO_WIRING_SHIFT_H */
