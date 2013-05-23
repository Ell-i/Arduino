/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>
#include <string.h>

bool HardwareSerial::find(char *target) const {
    return find(target, strlen(target));
}

bool HardwareSerial::findUntil(char *target, char *terminator) const{
    return findUntil(target, strlen(target), terminator, strlen(terminator));
}

size_t HardwareSerial::write(const uint8_t *s, size_t len) const {
    int n = 0;
    while (len--)
        n += write(*s++);
    return n;
}

size_t HardwareSerial::write(const char *s) const {
    int n = 0;
    while (*s != '\0')
        n += write(*s++);
    return n;
}

