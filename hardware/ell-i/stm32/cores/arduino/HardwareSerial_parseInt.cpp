/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>


long HardwareSerial::parseInt() const {
    int negative = 1;
    int value = 0;

    int c = read();
    if (c == '-') {
        negative = -1;
        c = read();
    }
    while (c >= '0' && c <= '9') {
        value = value * 10 + c - '0';
        c = read();
    }
    pushBack(c);
    return negative * value;
}
