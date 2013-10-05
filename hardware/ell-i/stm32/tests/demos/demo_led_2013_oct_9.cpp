
#include "Arduino.h"

/**
 * The basic colours.
 *
 * These values act both as indices and pin numbers.
 */

typedef int colour_t;
#define RED      0
#define GREEN   (RED+1)
#define BLUE    (GREEN+1)
#define COLOURS (BLUE+1)


void setup() {
    Serial.println("LED demo starting.");

    /**
     * Debug leds
     */
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, 1);
    digitalWrite(12, 1);

    /**
     * Debug console
     */
    Serial.begin(57600);
    Serial.setTimeout(100);

    /**
     * PWMs.  For now, comparators are set automatically from init.
     */
    /* 46875 * 1024 = 48000000 */
    setPWMFrequency(GREEN, 46875, 10);
    setPWMFrequency(BLUE,  46875, 10);
    setPWMFrequency(RED,   46875, 10);

    /**
     * Allow the network thread to run and initialise itself
     */
    yield();                        

    Serial.println("LED demo started.");
}

#define PWM_LIMIT 32

extern "C" {
    extern volatile uint32_t pwm[COLOURS];
};

void loop() {
    for (colour_t c = RED; c < COLOURS; c++) {
        uint32_t v = pwm[c];
        if (v > PWM_LIMIT)
            pwm[c] = v = PWM_LIMIT;
        analogWrite(c, v);
    }
    yield();
}

extern "C" {
    extern void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
}

/**
 * Called when the comparator detects an overrun
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    /* Shutdown the PWMs */
    for (colour_t c = RED; c < COLOURS; c++) {
        pwm[c] = 0;
        analogWrite(c, 0); 
    }

    /* Acknowledge the interrupt and restart PWM output */
    TIM1->SR   &= ~TIM_SR_BIF;
    TIM1->BDTR |=  TIM_BDTR_MOE;
}
