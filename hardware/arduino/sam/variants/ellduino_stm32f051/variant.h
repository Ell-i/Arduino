/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include "Arduino.h"
#ifdef __cplusplus
#include "USARTClass.h"
#endif

#define VARIANT_MCK 48000000

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

#ifdef __cplusplus

extern "C" {
extern USARTClass Serial;
extern USARTClass Serial1;
};

#endif
