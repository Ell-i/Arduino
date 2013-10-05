
#include "Arduino.h"

void setup() {
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, 1);
    digitalWrite(12, 0);
    Serial.begin(57600);
    Serial.setTimeout(1000);
    analogReadResolution(12);
    /* 46875 * 1024 = 48000000 */
    setPWMFrequency(0, 46875, 10);
    Serial.println(stm32_pinDescription[0].pin_pwm_timer->PSC);
    Serial.println(stm32_pinDescription[0].pin_pwm_timer->ARR);
    setPWMFrequency(1, 46875, 10);
    setPWMFrequency(2, 46875, 10);
    Serial.println(stm32_pinDescription[0].pin_pwm_timer->PSC);
    Serial.println(stm32_pinDescription[0].pin_pwm_timer->ARR);
}

void set_pwm(int duty) {
    analogWrite(0, duty);
    analogWrite(1, duty);
    analogWrite(2, duty);
}

void print_adc(int count) {
    while (count--) {
        Serial.print(micros());
        Serial.print(':');
        Serial.println(analogRead(A0));
    }
}

void loop() {
    char buffer[80];

    Serial.println("Give a command!");
    int n;
    do {
        buffer[0] = 0;
        n = Serial.readBytesUntil('\n', buffer, sizeof(buffer));
    } while (n <= 0);
#if 1
    Serial.print(buffer);
#endif
    switch (buffer[0]) {
    case 'p': 
        int pwm = atoi(buffer + 1);
        if (pwm < 0 || pwm > 120)
            pwm = 0;
        set_pwm(pwm);
        print_adc(50);
    }
}
