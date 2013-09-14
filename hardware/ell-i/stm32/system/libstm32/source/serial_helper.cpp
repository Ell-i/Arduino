
#include <Arduino.h>

extern "C" {
    void print(char *s);
}

static bool inited = false;

void print(char *s) {
    if (!inited) {
        Serial.begin(56700);
        inited = true;
    }

    Serial.println(s);
}
