
#include "Arduino.h"
#include <thread_switch.h>

static uint32_t interrupt_stack[256];

static void *volatile main_thread_psp;

extern void print(char *s);

char buffer[128];

void another_thread(int pin) {
    for (;;) {

        digitalWrite(pin, 1);  /* Blue on */
        delay(1000);

#if 0
        snprintf(buffer, 128, "other=0x%x, other+1=0x%x, lr_exc_return=0x%x\r\n", 
                 __thread_other_execution_context, 
                 __thread_other_execution_context + 1, 
                 __thread_other_execution_context->lr_exc_return);
        print(buffer);

        snprintf(buffer, 128, "other=0x%x, lr_exc_return=0x%x\r\n", 
                 __thread_other_execution_context, 
                 __thread_other_execution_context->lr_exc_return);
        print(buffer);
#endif

#if 0
        if ((__thread_other_execution_context + 1) != main_thread_psp) {
            /* Mangled stack pointer */
            digitalWrite(12, 0); /* Turn off blue */
            digitalWrite(13, 0); /* Turn off  red */
            for (;;)
                ;
        }

        if (__thread_other_execution_context->lr_exc_return != EXC_RETURN_PSP) {
            /* Mangled stack, LR wrong */
            digitalWrite(12, 1); /* Turn on blue */
            digitalWrite(13, 0); /* Turn off red */
            for (;;)
                ;
        }
#endif

        digitalWrite(pin, 0);  /* Blue off */
        delay(1000);

#if 0
        __thread_switch();
        __asm__ volatile(
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            );
#endif
    }
}

static THREAD_DEFINE_EXECUTION_CONTEXT(test, 512, another_thread, 
                                       __thread_exit, 12, 0, 0, 0);
static THREAD_DEFINE_CONTEXT(test);

void setup() {
    pinMode(13, OUTPUT);  /* Red led */
    pinMode(12, OUTPUT);  /* Blue led */
    digitalWrite(13, 1);

    __thread_init(interrupt_stack, 
                  THREAD_INITIAL_EXECUTION_CONTEXT(__thread_test_context.tc_stack_bottom));

    if (__thread_other_execution_context->lr_exc_return != EXC_RETURN_PSP) {
        /* Mangled stack, LR wrong */
        digitalWrite(12, 1); /* Turn on blue */
        for (;;)
            ;
    }

    digitalWrite(13, 0); /* Turn off red */
}

void loop() {

    digitalWrite(13, 1); /* Red on */
    delay(1000);
    digitalWrite(13, 0); /* Red off */
    delay(1000);

#if 0
    main_thread_psp = __get_PSP();

    snprintf(buffer, 128, "test_stack_bottom=0x%x other=0x%x, main_psp=0x%x\n", 
             __thread_test_context.tc_stack_bottom,
             __thread_other_execution_context, main_thread_psp);
    print(buffer);
#endif

#if 0
    __thread_switch();
    __asm__ volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        );
#endif
}


