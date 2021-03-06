/*
  Copyright (c) 2012 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// some libraries and sketches depend on this
// AVR stuff, assuming Arduino.h or WProgram.h
// automatically includes it...
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "binary.h"

/*
 * From http://stackoverflow.com/questions/1598773/\
 * is-there-a-standard-function-in-c-that-would-return-the-length-of-an-array:
 *
 * In this version if a pointer is mistakenly passed as the argument,
 * the compiler will complain in some cases - specifically if the
 * pointer's size isn't evenly divisible by the size of the object the
 * pointer points to. In that situation a divide-by-zero will cause
 * the compiler to error out. Actually at least one compiler
 * gives a warning instead of an error.
 *
 * That macro doesn't close the door on using it erroneously, but it
 * comes close in straight C.
 */

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

// Includes Atmel CMSIS
#include <chip.h>

#include "wiring_constants.h"

#define clockCyclesPerMicrosecond() ( SystemCoreClock / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (SystemCoreClock / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (SystemCoreClock / 1000000L) )

void yield(void);

/* sketch */
extern void setup( void ) ;
extern void loop( void ) ;

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
#define digitalPinToPort(P)        ( g_APinDescription[P].pPort )
#define digitalPinToBitMask(P)     ( g_APinDescription[P].ulPin )
#define digitalPinToTimer(P)       (  )
//#define analogInPinToBit(P)        ( )
#define portOutputRegister(port)   ( &(port->PIO_ODSR) )
#define portInputRegister(port)    ( &(port->PIO_PDSR) )
//#define portModeRegister(P)        (  )

//#define NOT_A_PIN 0  // defined in pio.h/EPioType
#define NOT_A_PORT           0

typedef enum _EExt_Interrupts
{
  EXTERNAL_INT_0=0,
  EXTERNAL_INT_1=1,
  EXTERNAL_INT_2=2,
  EXTERNAL_INT_3=3,
  EXTERNAL_INT_4=4,
  EXTERNAL_INT_5=5,
  EXTERNAL_INT_6=6,
  EXTERNAL_INT_7=7,
  EXTERNAL_NUM_INTERRUPTS
} EExt_Interrupts ;

typedef void (*voidFuncPtr)( void ) ;

/* Define attribute */
#if defined   ( __CC_ARM   ) /* Keil uVision 4 */
    #define WEAK (__attribute__ ((weak)))
#elif defined ( __ICCARM__ ) /* IAR Ewarm 5.41+ */
    #define WEAK __weak
#elif defined (  __GNUC__  ) /* GCC CS */
    #define WEAK __attribute__ ((weak))
#endif

typedef uint8_t pin_t;

/* Definitions and types for pins */
typedef enum _EAnalogChannel
{
  NO_ADC=-1,
  ADC_CH0=0,
  ADC_CH1,
  ADC_CH2,
  ADC_CH3,
  ADC_CH4,
  ADC_CH5,
  ADC_CH6,
  ADC_CH7,
  ADC_CH8,
  ADC_CH9,
  ADC_CH10,
  ADC_CH11,
  ADC_CH12,
  ADC_CH13,
  ADC_CH14,
  ADC_CH15,
  DA_CH0,
  DA_CH1
} EAnalogChannel ;

#define ADC_CHANNEL_NUMBER_NONE 0xffffffff

// Definitions for PWM channels
typedef enum _EPWMChannel
{
  NOT_ON_PWM=-1,
  PWM_CH0=0,
  PWM_CH1,
  PWM_CH2,
  PWM_CH3,
  PWM_CH4,
  PWM_CH5,
  PWM_CH6,
  PWM_CH7
} EPWMChannel ;

// Definitions for TC channels
typedef enum _ETCChannel
{
  NOT_ON_TIMER=-1,
  TC0_CHA0=0,
  TC0_CHB0,
  TC0_CHA1,
  TC0_CHB1,
  TC0_CHA2,
  TC0_CHB2,
  TC1_CHA3,
  TC1_CHB3,
  TC1_CHA4,
  TC1_CHB4,
  TC1_CHA5,
  TC1_CHB5,
  TC2_CHA6,
  TC2_CHB6,
  TC2_CHA7,
  TC2_CHB7,
  TC2_CHA8,
  TC2_CHB8
} ETCChannel ;

/**
 * Pin Attributes to be OR-ed
 */
#define PIN_ATTR_COMBO         (1UL<<0)
#define PIN_ATTR_ANALOG        (1UL<<1)
#define PIN_ATTR_DIGITAL       (1UL<<2)
#define PIN_ATTR_PWM           (1UL<<3)
#define PIN_ATTR_TIMER         (1UL<<4)

#ifdef __cplusplus
} // extern "C"

#include "RingBuffer.h"
#include "WCharacter.h"
#include "WString.h"
#include "HardwareSerial.h"
#include "wiring_pulse.h"

#endif // __cplusplus

// Include board variant
#include "variant.h"

#include "wiring_time.h"
#include "wiring_digital.h"
#include "wiring_analog.h"
#include "wiring_tone.h"
#include "wiring_math.h"
#include "wiring_shift.h"
#include "WInterrupts.h"

#if defined(__SAM3X8E__)

// USB Device
#define USB_VID            0x2341 // arduino LLC vid
#define USB_PID_LEONARDO   0x0034
#define USB_PID_MICRO      0x0035
#define USB_PID_DUE        0x003E
#include "USB/USBDesc.h"
#include "USB/USBCore.h"
#include "USB/USBAPI.h"

#endif /* __SAM3X8E__ */

#endif // Arduino_h
