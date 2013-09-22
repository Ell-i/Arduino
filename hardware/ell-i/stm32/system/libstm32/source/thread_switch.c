/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

/**
 * Low-level thread switching code.  
 *
 * This module performs the actual context switch.  The context switch
 * is implemented through the ARM recommended PendSV interrupt, which
 * is triggered as the lowest priority interrupt after all other
 * pending interrupts have been served.  The thread_switch() function
 * simply requests a PendSV interrupt.  Hence, if thread_switch() is
 * called from an interrupt context, the context switch takes place
 * when returning to the non-interrupt context.  On the other hand, if
 * thread_switch() is called by a running thread, the switch is
 * immediate.
 *
 * Usually thread_switch() is called by some higher-level threading
 * code, such as the scheduler.
 */

#include <thread_switch.h>

/**
 * The other thread that the context switch routine swaps the current
 * thread with.  This is the main data interface with the scheduler.
 * 
 * A typical scheduler performs the following steps:
 *
 *  - copy the context of the thread that was previously running
 *    from __thread_other_execution_context to the thread data
 *    structure
 *
 *  - select the next thread to run
 *
 *  - copy the context of the next thread from the thread data
 *    structure to __thread_other_execution_context
 *
 *  - call thread_switch(), which then eventually switches the
 *    currently running thread context and the one pointed by
 *    __thread_other_execution_context.
 */

volatile struct thread_execution_context *volatile 
__thread_other_execution_context;

/**
 * XXX
 */

#if 1
void yield(void) {
    if (__thread_other_execution_context)
        __thread_switch();
}
#endif

/**
 * Initialise the low-level threading code.  
 *
 * Initialises the MSP to point to the interrupt_stack and switches
 * the currently executing context to using PSP.  Initialises the
 * other context to the idle_context, thereby making sure that any
 * spurious call to PendSV_Handler does not break havoc.
 */
void 
__thread_init(void *interrupt_stack, struct thread_execution_context *idle_context) {

    __thread_other_execution_context = idle_context;
    __asm__ volatile (
        // Copy the current stack pointer to psp
        "mov r2, sp        \n\t"
        "msr psp, r2       \n\t"
        "mov r2, #2        \n\t" // Flag for active stack pointer in control register
        // Start critical section
        "cpsid i           \n\t"
        // Switch to use the psp
        "mrs r3, control   \n\t"
        "orr r3, r2        \n\t"
        "msr control, r3   \n\t" //
        // Load the interrupt stack pointer to msp
        "msr msp, %0       \n\t" 
        "cpsie i           \n\t"
        // End critical section
        : 
        : "r" (interrupt_stack)
        : "r2", "r3"
        );
}

/** 
 * Switch to the other execution context.
 * 
 * Typically called by the scheduler.  
 *
 * Note that this routine is always called in the Handler mode,
 * either as such (if PendSV was scheduled from a thread) or through
 * tail chaining.  Hence, the stack already contains the following
 * registers when this routine is called:
 *   psr, pc, lr, r12, r3, r2, r1, r0
 *
 * Our code assumes that PendSV is the the lowest priority interrupt
 * handler, always called as the *last* handler before returning from
 * a chain of tail chained handlers.
 * 
 * See the following page for more discussion:
 * http://www.coactionos.com/embedded-design/36-context-switching-on-the-cortex-m3.html
 *
 * We assume that the thread does need to be switched and optimise for
 * that.  That is, in the implementation, we assume that
 * PendSV_Handler is only called if there *is* a need to change the
 * thread.  Hence, we do not check if the other thread to run is
 * different from the current one, but simply make the switch.
 * For most threads, this will cause trouble if it ever happens that
 * the currently executing context and the __thread_other_execution_context 
 * point within the same stack space.  However, the separate
 * idle_thread has been carefully crafted so that it *can* be switched
 * with itself.  (See thread_idle.c for more information.)
 */
__attribute__ ((naked)) void 
PendSV_Handler() {

    /*
     * Save the current (old) execution context.  This is not safety
     * critical; the code tolerates to be be interrupted by a higher
     * priority interrupts, as long as the interrupt routines use the
     * msp, save registers, and do NOT modify the psp.
     */

    // Note that we assume that the state needs to be stored onto
    // the area pointed by the psp, not the current (mps) stack, as this
    // routine is ran in the Handler mode.

    // Use r0 as the stack pointer for the old execution context
    register void *old_stack asm("r0");
    __asm__ volatile ("mrs %0, psp" : "=r" (old_stack));

    // stm in thumb is auto-incrementing (while push is auto-decrementing),
    // therefore we store the registers in reverse order.
    old_stack -= 9 /*XXX*/ * sizeof(uint32_t);

    // Push the unsaved registers to the old stack, using r0
    __asm__ volatile (
        // Take a working copy of r0
        "mov r1,  r0                                 \n\t"
        // "Reverse" push r10, r11, r4, r5, r6, r7
        "mov r2,  r10                                \n\t"
        "mov r3,  r11                                \n\t"
        "stm r1!, {r2, r3, r4, r5, r6, r7}           \n\t"
        // "Reverse" push r8, r9, lr
        "mov r4,  r8                                 \n\t"
        "mov r5,  r9                                 \n\t"
        "mov r6,  lr                                 \n\t"
        "stm r1!, {r4, r5, r6}                       \n\t"
        :
        : "r"  (old_stack)
        : "r1", "r2", "r3", "r4", "r5", "r6", "memory"
        );

    /*
     * At this point, the current process stack contains the following
     * registers, in this order (high-to-low)
     *
     *   psr, pc, lr, r12, r3, r2, r1, r0, r7, r6, r5, r4, lr, r11, r10, r9, r8
     *             ^                                        ^                ^
     *            interrupted return address               exp_return        r0
     * 
     * The following registers haven't been pushed, for the following
     * reasons:
     * - msp     -- master stack pointer, not used in Thread mode, only in Handler mode
     * - primask -- only affects interrupt activation
     * - control -- assumed not to be changed elsewhere by software
     *
     * Hence, at this point, r0 points to the top of the old stack,
     * with all register values pushed there.  All other general
     * purpose registers are free.
     */

    // Use r1 as the stack pointer for the new execution context
    register void *new_stack asm("r1");

    // Load the pointer to the other execution context's top of the stack 
    register struct thread_switch_context **switch_context_p = &__thread_other_execution_context;

    {
        // Start critical section -- disable higher priority interrupts,
        // as they may indirectly change the contents of __thread_other_execution_context.
        __asm__ volatile( "cpsid i");

        // Load the new stack pointer from (*switch_context)
        __asm__ volatile ("ldr %0, [%1]" : "=r" (new_stack): "r" (switch_context_p));
        
        // Store the old stack pointer to (*switch_context)
        __asm__ volatile ("str %0, [%1]" :: "r" (old_stack), "r" (switch_context_p) : "memory");
        
        // End of critical section -- enable higher priority interrupts.
        __asm__ volatile ("cpsie i" : : : "memory");
    }

    /*
     * Re-establish the new execution context.
     * 
     * Pop the explicitly saved registers from the psp stack.
     */

    // Note that we will move the exc_return value to the pc, causing
    // the routine to perform an exception return within the new thread
    // context.  But first load it to r3 so that we can save the top
    // of the stack to the psp before actually returning.
    register void *new_exc_return_value asm("r3");

    // Pop the registers not saved by the interrupt hardware.
    __asm__ volatile (
        // Pop r10, r11, r4, r5, r6, r7
        "ldm %0!, {r2, r3, r4, r5, r6, r7}          \n\t"
        "mov r11, r3                                \n\t"
        "mov r10, r2                                \n\t"
        // Pop r8, r9, lr, leaving lr at r3
        "ldm %0!, {r0, r2, %1}                      \n\t"
        "mov r8, r0                                 \n\t"
        "mov r9, r2                                 \n\t"
        : "=r" (new_stack), "=r" (new_exc_return_value)
        : "0" (new_stack)
        : "r0", "r2", 
          "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11"
        );

    // Restore the psp to its pre-interrupt value 
    __asm__ volatile ("msr psp, %0" : : "r" (new_stack));

    // Branch to the exc_return value at r3, causing an exception return.
    __asm__ volatile ("bx  %0" : : "r" (new_exc_return_value));
    /* NOTREACHED */

    // Note that this routine may be ran immediately again after completing
    // this round, if an intervening higher-priority routine
    // reschedules the threads. 
}
