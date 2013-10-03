/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>

static volatile int foo;

void HardwareSerial::irqHandler() const {
    buffer->put(usart->RDR);
}
