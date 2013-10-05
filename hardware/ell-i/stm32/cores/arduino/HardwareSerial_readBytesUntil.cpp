/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>

size_t HardwareSerial::readBytesUntil(
    int terminator, char *retbuf, size_t length) const {

    uint32_t timeout = millis() + buffer->_getTimeout();

    // XXX Refactor, ugly code below
    int count;
    for (count = 0; count < length; count++) {
        int c = read();
        while (c < 0) {
            if (millis() >= timeout)
                goto out;
            yield();
            c = read();
        }
        *retbuf++ = c;
        if (c == terminator)
            break;
    }
out:
    // Terminate the string if there is space
    if (count < length)
        retbuf[0] = '\0';
    return count;
}
