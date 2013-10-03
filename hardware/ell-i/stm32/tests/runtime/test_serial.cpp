
#include "Arduino.h"

void setup() {
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, 1);
    digitalWrite(12, 0);
    Serial.begin(57600);
#if 0
    Serial.println(Serial.buffer->head);
    Serial.println(Serial.buffer->tail);
    Serial.flush();
    Serial.println(Serial.buffer->head);
    Serial.println(Serial.buffer->tail);
#endif
}

void loop() {
    Serial.print("Hel");
    digitalWrite(13, 1);
    digitalWrite(12, 1);
    Serial.print("lo, ");
    delay(1000);
    digitalWrite(13, 0);
    digitalWrite(12, 0);
    Serial.println("World!");
    delay(1000);    
#if 0
    Serial.println(Serial.buffer->empty());
    int c = Serial.buffer->get();
    Serial.println(c);
#endif
#if 1
    char buffer[80];
    buffer[0] = 0;
    int n = Serial.readBytesUntil('\n', buffer, sizeof(buffer));
    if (n != 0) {
        Serial.print(n);
        Serial.print(buffer);
    }
#endif
}
