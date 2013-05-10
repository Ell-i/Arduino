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
static const uint8_t A0   = 54;
static const uint8_t A1   = 55;
static const uint8_t A2   = 56;
static const uint8_t A3   = 57;
static const uint8_t A4   = 58;
static const uint8_t A5   = 59;
static const uint8_t A6   = 60;
static const uint8_t A7   = 61;
static const uint8_t A8   = 62;
static const uint8_t A9   = 63;
static const uint8_t A10  = 64;
static const uint8_t A11  = 65;
static const uint8_t DAC0 = 66;
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
