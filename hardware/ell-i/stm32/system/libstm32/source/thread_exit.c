/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

#include <thread_switch.h>

/**
 * Terminate the currently running execution context.
 *
 * Usually called by the runtime when a threaded funtion returns.  At
 * that point, the stack pointer (psp) is pointing to the bottom of
 * the stack, and r0..r3 contain the return value of the function.
 *
 * Must be called from a non-interrupt context, running on the psp.
 * May be called with interrupts disabled (but will disable them
 * itself anyway).
 *
 * The main thread and the idle thread never call this, as the main thread
 * continues to run its own code even after having called thread_init,
 * and the idle thread remains in the infinite wfi loop.
 *
 * This function has not been designed to be directly called by the
 * execution thread other than through falling through the end of the
 * threaded function itself.  For terminating the currently running
 * thread, use the routines defined at the scheduler layer.
 *
 * Here we simply store the topmost registers to the stack, allowing
 * the return value (if any) to be examined by external code and
 * switch to the idle thread context.  Typically the caller disables
 * interrupts and requests a context switch before jumping here, causing
 * the execution to continue in the other context once this context
 * has been replaced with the idle context.
 */

__attribute__ ((naked)) void
__thread_exit(uint32_t return_value) {
    /*
     * Start critical section -- disable context switches.
     *
     * We start the critical section already here to avoid
     * the unlikely but possible event that the thread hasax
     * the minimum size stack and someone makes a context switch.
     * If that were to happen, the stack would leak.
     *
     * An alternative design would not use the psp for pushing
     * but use some other register in the range of r4..r7
     * as a temporary stack pointer.  However, I am not sure what
     * would happen then to the exiting thread; this design
     * feels safer, and most probably this is not a piece of
     * code that needs to be heavily optimised anyway.
     */
    __asm__ volatile ("cpsid i");

    /*
     * Dump the return values to the stack.
     *
     * Use registers r7...r4 as fillers to fill in the xpsr, pc,
     * lr_thread and r12 positions in the stack, allowing r0..r3 to be
     * read from their "proper" positions.
     */
    __asm__ volatile (
        "push {r0, r1, r2, r3, r4, r5, r6, r7}"
        : : : "memory"
        );

    /*
     * Prepare for replacing the currently executing context with the
     * idle thread context.  As the idle thread has been designed to
     * be idempotent, it can be reused for any number of (terminating)
     * threads as long as the idempotency is preserved.
     */
    register void *idle_thread_stack_bottom = __thread_idle_context.tc_stack_bottom;
    extern void HardFault_Handler();
    register void *hardfault_handler = HardFault_Handler;

    /*
     * Explicitly replace the current execution context with to the
     * idle thread execution context.
     *
     * As idle thread entry point does not assume any arguments, it is
     * sufficient to set up the stack and the link registers.
     */
    __asm__ volatile (
        "mov  sp, %0\n\t"
        "mov  lr, %1\n\t"
        :
        : "r" (idle_thread_stack_bottom), "r" (hardfault_handler)
        : "sp"
        );

    /*
     * Now jump to the idle thread entry point.  The idle thread entry
     * point enables interrupts, causing an immediate context switch
     * to the other execution context, if so requested before coming
     * here.
     */
    register void * thread_idle_entry = __thread_idle_entry;
    __asm__ volatile (
        "mov pc, %0"
        :
        : "r" (thread_idle_entry)
        : "r0", "pc"
        );

    /* NOTREACHED */

    /*
     * In the pathological case that also the other execution context
     * is the idle thread, the other execution context stack top will
     * be at the idle thread execution context top.  In that case, the
     * execution context created above will be stored into the idle
     * thread stack and the immediately popped back from there.  This
     * works due to the idempotency of the idle thread execution
     * context.
     *
     * As a result, every context switch will simply cause the idle
     * thread to be executed again, until the scheduler gets a chance
     * to run and change the other execution context.
     */
}


