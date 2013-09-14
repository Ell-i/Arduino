
#include <stdint.h>
#include "Arduino.h"
#include <thread_switch.h>

static uint32_t interrupt_stack[256];

static void *volatile main_thread_psp;

extern void print(char *s);

char buffer[128];

__attribute__ ((naked))
void another_thread(int pin) {
    register uint32_t r7 asm("r7");
    register uint32_t r8 asm("r8");
    register uint32_t r9 asm("r9");
    register uint32_t r10 asm("r10");
    register uint32_t r11 asm("r11");
    register uint32_t r12 asm("r12");

    {
        register uint32_t r0 asm("r0");
        register uint32_t r1 asm("r1");
        register uint32_t r2 asm("r2");
        register uint32_t r3 asm("r3");
        register uint32_t r4 asm("r4");
        register uint32_t r5 asm("r5");
        register uint32_t r6 asm("r6");
        register uint32_t r7 asm("r7");

        if (r0 != 0 ||
            r1 != 1 ||
            r2 != 2 ||
            r3 != 3 ||
            r4 != 4 ||
            r5 != 5 ||
            r6 != 6 ||
            r7 != 7)
            goto failure;
    }

    if (r7  != 7 ||
        r8  != 8 ||
        r9  != 9 ||        
        r10 != r10 ||        
        r11 != r11 ||        
        r12 != r12)
        goto failure;

success:
    digitalWrite(13, 1);  /* Red on */
    digitalWrite(12, 1);  /* Blue on */
    for (;;)
        ;

failure:
    digitalWrite(13, 1);  /* Red on */
    digitalWrite(12, 0);  /* Blue off */
    for (;;)
        ;

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

void loop() {

    digitalWrite(13, 1); /* Red on */
    delay(1000);
    digitalWrite(13, 0); /* Red off */
    digitalWrite(12, 1); /* Blue on */
    delay(1000);

    __thread_switch();
    __asm__ volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        );
}


