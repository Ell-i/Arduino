/*

  Copyright (c) 2013 Ell-i.  All rights reserved.
*/

#ifndef ELLDUINO_RINGBUFFER_H
#define ELLDUINO_RINGBUFFER_H

#ifndef RINGBUFFER_SIZE
#define RINGBUFFER_SIZE 64
#endif

class RingBuffer {
private:
    uint8_t buffer[RINGBUFFER_SIZE];
    uint8_t head;
    uint8_t tail;

    int nextIndex(int index) const {
        return (index + 1) % RINGBUFFER_SIZE;
    }

    int prevIndex(int index) const {
        return (RINGBUFFER_SIZE + index - 1) % RINGBUFFER_SIZE;
    }

public:
    /* XXX constexpr */ RingBuffer() : head(0), tail(0) {}

    void flush(void) {
        head = tail = 0;
    }

    bool empty() const {
        return head == tail;
    }

    bool full() const {
        return nextIndex(head) == tail;
    }

    int peek() const {
        return empty()? -1: buffer[tail];
    }

    void put(uint8_t c) {
        if (!full()) {
            buffer[head] = c;
            head = nextIndex(head);
        }
    }

    int get() {
        int c = peek();
        if (!empty()) {
            tail = nextIndex(tail);
            return c;
        }
        return -1;
    }

    int push(uint8_t c) {
        /*
         * We must protect against interrupts here
         * in order to avoid a race condition with put,
         * which may be called from an interrupt routine.
         */
        noInterrupts();
        tail = prevIndex(tail);
        if (tail == head) {
            /* OOPS.  The buffer was full.  Junk the newest. */
            head = prevIndex(head);
        }
        buffer[tail] = c;
        interrupts();
    }
};

#endif /* ELLDUINO_RINGBUFFER_H */
