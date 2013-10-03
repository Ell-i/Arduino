/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>

size_t HardwareSerial::readBytesUntil(
    int terminator, char *buffer, size_t length) const {

    int count;
    for (count = 0; count < length; count++) {
        int c = read();
        if (c < 0) {
            break;
        }
        *buffer++ = c;
        if (c == terminator)
            break;
    }
    // Terminate the string if there is space
    if (count < length)
        buffer[1] = '\0';
    return count;
}
