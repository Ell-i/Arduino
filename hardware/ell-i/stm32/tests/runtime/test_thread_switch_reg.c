
#include <stdint.h>
#include "Arduino.h"
#include <thread_switch.h>

static uint32_t interrupt_stack[256];

static void *volatile main_thread_psp;

extern void print(char *s);

char buffer[128];

__attribute__ ((naked))
void another_thread(int pin) {

    for (;;) {
        __asm__ volatile(
            "cpsid i           \n\t"
            );
        __thread_switch();
        
        __asm__ volatile(
            "mov r0, #12       \n\t"
            "mov r12, r0       \n\t"
            "mov r0, #0        \n\t"
            "mov r1, #1        \n\t"
            "mov r2, #2        \n\t"
            "mov r3, #3        \n\t"
            "cpsie i           \n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            );
    }

}

static THREAD_DEFINE_EXECUTION_CONTEXT(test, 512, another_thread, 
                                       __thread_exit, 0, 1, 2, 3);
static THREAD_DEFINE_CONTEXT(test);

void setup() {
    pinMode(13, OUTPUT);  /* Red led */
    pinMode(12, OUTPUT);  /* Blue led */
    digitalWrite(13, 1);  /* Red on */

    __thread_init(interrupt_stack, 
                  THREAD_INITIAL_EXECUTION_CONTEXT(__thread_test_context.tc_stack_bottom));

    __thread_other_execution_context->r10 = 10;
    __thread_other_execution_context->r11 = 11;
    __thread_other_execution_context->r4  =  4;
    __thread_other_execution_context->r5  =  5;
    __thread_other_execution_context->r6  =  6;
    __thread_other_execution_context->r7  =  7;
    __thread_other_execution_context->r8  =  8;
    __thread_other_execution_context->r9  =  9;
    __thread_other_execution_context->r12 = 12;

    if (__thread_other_execution_context->lr_exc_return != EXC_RETURN_PSP) {
        /* Mangled stack, LR wrong */
        digitalWrite(12, 1); /* Turn on blue */
        for (;;)
            ;
    }

    digitalWrite(13, 0); /* Turn off red */
}

static void *ctx;

void loop() {

    digitalWrite(13, 1); /* Red on */
    delay(1000);
    digitalWrite(13, 0); /* Red off */
    digitalWrite(12, 1); /* Blue on */
    delay(1000);

    ctx = __thread_other_execution_context;

    __thread_switch();
    __asm__ volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        );

    if (ctx != __thread_other_execution_context) {
        digitalWrite(13, 0);  /* Red off */
        digitalWrite(12, 0);  /* Blue off */
    }

    if (
        __thread_other_execution_context->r12 != 12 ||
        __thread_other_execution_context->r0  !=  0 ||
        __thread_other_execution_context->r1  !=  1 ||
        __thread_other_execution_context->r2  !=  2 ||
        __thread_other_execution_context->r3  !=  3 ||
        __thread_other_execution_context->r10 != 10 ||
        __thread_other_execution_context->r9  !=  9 ||
        __thread_other_execution_context->r8  !=  8 ||
        false) {

        digitalWrite(13, 1);  /* Red on */
        digitalWrite(12, 0);  /* Blue off */
        for (;;)
            ;
    }

    if (
        __thread_other_execution_context->r11 != 11 ||
        __thread_other_execution_context->r7  !=  7 ||
        __thread_other_execution_context->r4  !=  4 ||
        __thread_other_execution_context->r5  !=  5 ||
        __thread_other_execution_context->r6  !=  6 ||
        false) {

        digitalWrite(13, 0);  /* Red off */
        digitalWrite(12, 1);  /* Blue on */
        for (;;)
            ;
    }


success:
    digitalWrite(13, 1);  /* Red on */
    digitalWrite(12, 1);  /* Blue on */
    for (;;)
        ;


}


