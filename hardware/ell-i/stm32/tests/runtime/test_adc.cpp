
#include "Arduino.h"

void setup() {
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, 1);
    digitalWrite(12, 0);
    Serial.begin(57600);
    Serial.setTimeout(100);
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

void round_test(int duty, int time) {
    analogWrite(0, duty);
    delay(time);
    analogWrite(0, 0);
    analogWrite(1, duty);
    delay(time);
    analogWrite(1, 0);
    analogWrite(2, duty);
    delay(time);
    analogWrite(2, 0);
}

void print_adc(int count) {
    while (count--) {
        Serial.print(micros());
        Serial.print(':');
        Serial.println(analogRead(A0));
    }
}

int pwm = 0;
int test_pwm = 0;

void loop() {
    char buffer[80];

    Serial.println("Give a command!");
    int n;
    do {
        buffer[0] = 0;
        n = Serial.readBytesUntil('\n', buffer, sizeof(buffer));
        if (test_pwm)
            round_test(test_pwm, 100);
    } while (n <= 0);
#if 1
    Serial.print(buffer);
#endif
    pwm = atoi(buffer + 1);
    if (pwm < 0 || pwm > 120)
        pwm = 0;
    switch (buffer[0]) {
    case 'p': 
        set_pwm(pwm);
        print_adc(50);
        break;
    case 'e': 
        test_pwm = pwm;
        break;
    }
}

extern "C" {
    extern void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    analogWrite(0, 1 /* XXX */); 
    analogWrite(1, 1);
    analogWrite(2, 1);
    TIM1->SR   &= ~TIM_SR_BIF;
    TIM1->BDTR |=  TIM_BDTR_MOE;
}
