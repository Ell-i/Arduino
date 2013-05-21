/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_CONSTANTS_H
#define ELLDUION_WIRING_CONSTANTS_H

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

enum {
    LOW  = 0, 
    HIGH = 1,
    CHANGE = 2,
    FALLING = 3,
    RISING = 4,
};

#ifndef __cplusplus
enum {
    false = 0,
    true = 1,
};
#endif

enum {
    SERIAL = 0,
    DISPLAY = 1,
};

#define interrupts() __enable_irq()
#define noInterrupts() __disable_irq()

typedef unsigned int word;

// TODO: to be checked
typedef uint8_t boolean ;
typedef uint8_t byte ;


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* ELLDUINO_WIRING_CONSTANTS_H */
