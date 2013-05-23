/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_MATH_H
#define ELLDUION_WIRING_MATH_H

#ifdef __cplusplus

#include <stdlib.h>

static const double PI         = 3.1415926535897932384626433832795;
static const double HALF_PI    = 1.5707963267948966192313216916398;
static const double TWO_PI     = 6.283185307179586476925286766559;
static const double DEG_TO_RAD = 0.017453292519943295769236907684886;
static const double RAD_TO_DEG = 57.295779513082320876798154814105;

/* Calculation */

static inline long abs(long x) { return ((x)>0?(x):-(x)); }

static inline long constrain(long value, long min, long ma) {
    return ((value)<(min)?(min):((value)>(ma)?(ma):(value)));
}
static inline int   map(int   value, int   low1, int   high1, int   low2, int   high2) {
    return ((value - low1) * ((high2 - low2) / (high1 - low1))) + low2;
}
static inline long  map(long  value, long  low1, long  high1, long  low2, long  high2) {
    return ((value - low1) * ((high2 - low2) / (high1 - low1))) + low2;
}
static inline float map(float value, float low1, float high1, float low2, float high2) {
    return ((value - low1) * ((high2 - low2) / (high1 - low1))) + low2;
}
static inline long max(long a, long b) { return ((a)>(b)?(a):(b)); }
static inline long min(long a, long b) { return ((a)<(b)?(a):(b)); }

static inline long round(float x) { return ((x)>=0?(long)((x)+0.5):(long)((x)-0.5)); }
static inline long  sq(long  x) { return (x)*(x); }
static inline float sq(float x) { return (x)*(x); }

/* Trigonometry */

static inline float radians(float deg) { return (deg)*DEG_TO_RAD; }
static inline float degrees(float rad) { return (rad)*RAD_TO_DEG; }

/* Random */

static inline long random(long min, long max) { 
    return (lrand48() % (max - min)) + min;
}
static inline long random(long max) { return random(0, max); }
static inline void randomSeed(long seed) { srand48(seed); }
static inline float random(float min, float max) { 
    return (drand48() * (max - min)) + min;
}
static float random(float max) { return random(0.0, max); }

/* Bit, byte and word handling */

static inline long bit(uint8_t b)                     { return (1UL << b); }
static inline long bitRead(long value, uint8_t bit)   { return ((value >> bit) & 1); }

static inline long bitSet(long &value, uint8_t bit)   { return (value |=  (1UL << bit)); }
static inline long bitClear(long &value, uint8_t bit) { return (value &= ~(1UL << bit)); }
static inline long bitWrite(long &value, uint8_t bit, uint8_t bitvalue) {
    return (bitvalue ? bitSet(value, bit) : bitClear(value, bit));
}

static inline uint8_t lowByte(uint16_t w)  { return ((uint8_t) ((w) & 0xff)); }
static inline uint8_t highByte(uint16_t w) { return ((uint8_t) ((w) >> 8)); }

static inline word makeWord(uint16_t w) { return w; }
static inline word makeWord(uint8_t h, uint8_t l ) { return h << 8 + l; }

#endif /* __cplusplus */

#endif /* ELLDUINO_WIRING_MATH_H */
