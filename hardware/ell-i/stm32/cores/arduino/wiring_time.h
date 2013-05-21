/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_WIRING_TIME_H
#define ELLDUINO_WIRING_TIME_H

#ifdef __cplusplus
extern "C"{
#endif

extern volatile uint32_t millisecondCount;

static inline 
uint32_t millis(void) {
    return millisecondCount;
}

static inline
uint32_t micros(void) {

    noInterrupts();
    const uint32_t pending_systicks = SysTick->VAL;
    const uint32_t millseconds = millisecondCount;
    interrupts();

    // By our runtime convention, SysTick->LOAD = SystemCoreClock / 1000 - 1 = F_CPU / 1000 - 1
    static const uint32_t systicks_per_ms       = F_CPU / 1000;                          /* 48000 */
    static const uint32_t systicks_per_us       = F_CPU / 10000000;                      /* 48 */
    static const uint32_t us_per_systick_x2to20 = (1 << 20) / systicks_per_us;           /* 21 825, 15 bits long */

    /* micros_x2to20 range is 0..1048560000 = 0..(2^20 * 1000), 30 bits long */
    const uint32_t micros_x2to20         = ((systicks_per_ms - pending_systicks) * us_per_systick_x2to20);
    const uint32_t micros                = micros_x2to20 >> 20;

    return millseconds * 1000 + micros;
}

static inline void delayMicroseconds(uint32_t us) __attribute__((always_inline, unused));
static inline
void delayMicroseconds(uint32_t us) {
    /* 
     * XXX Temporary implementation, until Chibi/OS or other RTOS gets integrated.
     * Note that this version breaks if the millisecond counter rolls over.
     *
     * Also note that for a constant argument, this becomes heavily optimised,
     * boiling down either to the assembler loop or the other loop.  With a 
     * variable argument it depends on what the compiler is able to infer
     * from the argument.  I don't know GCC well enough if it is able to do
     * variable range optimisations like LLVM.
     */
    const uint32_t cycles4 = (F_CPU / (1000000 * (1+3))) * us; /* 48 cycles per us, 12 loops per us */

    /*
     * Heuristics: If the desired delay is less than 1000 cycles or 250 loops (~20 us),
     * use the busy loop.  Otherwise allow other threads to run, with less accuracy
     *
     * In the assembly, subs takes 1 clock cycle, bge 3 cycles when taken.
     */
    if (cycles4 < 250) {
        asm volatile(
            "L_%=_delayMicroseconds:"       "\n\t"
            "subs   %0, #1"                 "\n\t"
            "bge    L_%=_delayMicroseconds" "\n"
            : /* no outputs */ : "r" (cycles4) 
            );
        return;
    }

    uint32_t timo = micros() + us;

    while (micros() < timo) {
        yield();
    }
}

static inline
void delay(uint32_t ms) {
    delayMicroseconds((ms + 1) * 1000); /* Ensure at least 1 ms delay */
}

static inline
void delayMilliseconds(uint32_t ms) {
    delay(ms);
}
        

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ELLDUINO_WIRING_TIME_H */
