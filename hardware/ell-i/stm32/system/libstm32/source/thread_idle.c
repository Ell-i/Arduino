/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

#include <thread_switch.h>

/**
 * Simplest possible idle execution context.
 *
 * This module implements the simplest possible idle execution context
 * that simply sleeps at wfi.  This execution context has been carefully
 * crafted so that it can be switched with itself.
 * 
 * The context starts with a small trampoline for allowing an exiting
 * thread to switch to this idle execution context, jumping to the
 * __thread_idle_entry lable.  The trampoline enables interrupts, 
 * at which point there may be a context switch (if there was a pending
 * one).  The saved context will then continue from the wfi instruction.
 * If there is a context switch any later, the saved context will continue
 * from the unconditional branch instruction.  However, this difference 
 * doesn't matter, and therefore the context is idempotent in practise.
 */

// Make it naked to avoid any compiler-inserted preamble, which
// might exceed the minimal stack we are using.
__attribute__ ((naked)) void 
__thread_idle_entry() {
    // The trampoline, enabling interrupts and allowing a context switch
    __asm__ volatile("cpsie i");

    // The actual idle thread
    for (;;) {
        __asm__ volatile ("wfi");
    }
}

extern void 
HardFault_Handler();

/**
 * The actual idle execution context.  
 *
 * As an (unnecessary) safety belt, if the context ever returns (which
 * should be impossible), fall back to the CMSIS HardFault_Handler(),
 * which is usually only called on hardware faults.
 */
static 
THREAD_DEFINE_EXECUTION_CONTEXT(idle, THREAD_STACK_SIZE_BYTES_MINIMUM, 
                                __thread_idle_entry, 
                                HardFault_Handler, 
                                0, 0, 0, 0);
/*global*/ 
THREAD_DEFINE_CONTEXT(idle);

