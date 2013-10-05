/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#ifndef ELLDUINO_HARDWARESERIAL_H
#define ELLDUINO_HARDWARESERIAL_H

#include "RingBuffer.h"

enum base {
    DEC = 10,
    HEX = 16,
    OCT = 8,
    BIN = 2,
};

class HardwareSerial /* XXX : public Stream */
{
public: /* XXX: Let these be public for now, making this class POD.  Once we get into full C++11 with constexpr, revise */
    USART_TypeDef *const usart;
    RingBuffer *const buffer;
    __IO uint32_t &gpio_afr;
    __IO uint32_t &gpio_moder;
    const uint32_t usart_afr_mask,     usart_afr_value;
    const uint32_t usart_moder_mask,   usart_moder_value;
    const uint32_t default_afr_mask,   default_afr_value;
    const uint32_t default_moder_mask, default_moder_value;

    void irqHandler() const;

public:
    /* XXX constexpr HardwareSerial(USART_TypeDef *const u, ...) : usart(u), ... {}; */
    void begin(unsigned long)      const;
    /* XXX: Should we really support end().  Is it ever called in practise? */
    void end()                     const;
    int available(void)            const;
    int peek(void)                 const;
    int read(void)                 const;
    void pushBack(int c)           const;
    void flush(void)               const;
    size_t write(uint8_t c)        const;

    size_t write(const String &s)  const { return write((uint8_t*)s.c_str(), s.length()); }
    size_t write(const char s[])   const;

    size_t write(const uint8_t[], size_t) const;

    operator bool()                const { return true; };

    /* Stream functions */
    void setTimeout(unsigned long timeout) const;

    bool find(char *target) const;
    bool find(char *target, size_t length) const;
    bool findUntil(char *target, char *terminator) const;
    bool findUntil(char *target, size_t targetLen, char *terminator, size_t termLen) const;

    long  parseInt() const;
    float parseFloat() const;

private:
    const static int NOT_A_CHAR = -1;

public:
    size_t readBytes(char *buffer, size_t length) const {
        return readBytesUntil(NOT_A_CHAR, buffer, length);
    }
    size_t readBytesUntil(int terminator, char *buffer, size_t length) const;

    String readString() const { return readStringUntil(NOT_A_CHAR); }
    String readStringUntil(int terminator) const;

    /* Print functions */
    size_t println(void) const { return write("\r\n"); }

    size_t print(unsigned long value, enum base base=DEC, bool u = 1) const;
    size_t print(double value, int decimals=2) const;

    /* Inlined print functions */

    size_t print(unsigned char value, enum base base=DEC) const { return print(value, base, 1); }
    size_t print(unsigned int  value, enum base base=DEC) const { return print(value, base, 1); }

    size_t print(int           value, enum base base=DEC) const { return print(value, base, 0); }
    size_t print(long          value, enum base base=DEC) const { return print(value, base, 0); }

    size_t print(const char c)     const { return write(c); }
    size_t print(const char s[])   const { return write(s); }
    size_t print(const String &s)  const { return write(s); }

    size_t println(unsigned long value, enum base base=DEC, unsigned u = 1) const {
        return print(value, base, u) + println();
    }
    size_t println(double value) const {
        return print(value) + println();
    }

    size_t println(unsigned char value, enum base base=DEC) const { return println(value, base, 1); }
    size_t println(unsigned int  value, enum base base=DEC) const { return println(value, base, 1); }

    size_t println(int           value, enum base base=DEC) const { return println(value, base, 0); }
    size_t println(long          value, enum base base=DEC) const { return println(value, base, 0); }

    size_t println(const char c)       const { int n = print(c); n += println(); return n; }
    size_t println(const char s[])     const { int n = print(s); n += println(); return n; }
    size_t println(const String &s)    const { int n = print(s); n += println(); return n; }
};

extern void serialEventRun(void) __attribute__((weak));

inline
void HardwareSerial::begin(unsigned long baudRate) const {
    /* Change the GPIO pins to the USART mode */
    gpio_afr &= usart_afr_mask;
    gpio_afr |= usart_afr_value;

    gpio_moder &= usart_moder_mask;
    gpio_moder |= usart_moder_value;

    /* Set the baud rate -- use 16 bit oversampling */
    usart->BRR  = SystemCoreClock / baudRate;

#if DEBUG
    if (usart == USART1) {
        GPIOC->ODR   |=  GPIO_ODR_6;
    }
#endif

    /* Enable the transmitter and the USART */
    usart->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

inline
void HardwareSerial::setTimeout(unsigned long timeout) const {
    buffer->_setTimeout(timeout);
}

inline
int HardwareSerial::available(void) const {
    return !buffer->empty();
};

inline
int HardwareSerial::peek(void) const {
    return buffer->peek();
};

inline
int HardwareSerial::read(void) const {
    return buffer->get();
};

inline
void HardwareSerial::pushBack(int c) const {
    buffer->push(c);
}

inline
void HardwareSerial::flush(void) const {
    buffer->flush();
};

inline
size_t HardwareSerial::write(uint8_t c) const {
    usart->TDR = c;

    while ((usart->ISR & USART_ISR_TXE) == 0) {
        //yield();
    }
};

#endif /* ELLDUINO_HARDWARESERIAL_H */
