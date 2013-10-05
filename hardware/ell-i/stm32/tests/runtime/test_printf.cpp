
#include "Arduino.h"
#include <stdio.h>

void setup() {
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, 1);
    digitalWrite(12, 0);
    Serial.begin(57600);
}

void loop() {
    Serial.print("Hello, ");
    digitalWrite(13, 1);
    digitalWrite(12, 1);
    delay(1000);
    digitalWrite(13, 0);
    digitalWrite(12, 0);
    printf("World!\n");
    delay(1000);    
}
