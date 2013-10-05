/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

#include <stdint.h>
#include <thread_switch.h>
#include <thread_rr.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * A very simple round robin scheduler.
 */

/*
 * The interrupt stack used with this scheduler.
 */
static uint32_t interrupt_stack[1024]; /* XXX Size? */

extern const uint32_t _end;

static const struct thread_context thread_main_context = {
    .tc_stack_bottom  = 0x20002000, /* XXX Replace with a linker reference! */
    .tc_stack_ceiling = &_end,      /* XXX This may be wrong.  Check against the linker script. */
};

static struct thread thread_main = {
    .t_context   = &thread_main_context,
    .t_stack_top = 0,           /* INitialised in the scheduler after first context switch */
    .t_next      = NULL,
    .t_flags     = 0,
};

// XXX Idle context not represented as a thread, no?

/*
 * The scheduler needs to be framed with a context of three threads:
 *  o The previous thread is the one that was running when the
 *    scheduler was last called, that was swapped out, and whose
 *    state we need to save on entry
 *  o The current thread is the one that is running now when
 *    the scheduler is called.  If the scheduler is called by
 *    the thread itself, we are actually running in the thread's
 *    execution context, i.e. using its stack.  On the other hand,
 *    if the scheduler is called from an interrupt context, the
 *    thread's stack contains the registers pushed by the ARM
 *    hardware but we are now using at the interrupt stack.
 *  o The next thread is the one that the scheduler selects
 *    to be ran next.  We don't have any explicit variable for that;
 *    the "curr" variable holds that value for a while after the
 *    scheduling decision has been made but the context switch
 *    has not yet happened.
 *
 * XXX Add framing discussion here.
 */

static struct {
    struct thread *prev, *curr;
} g = {
    .prev = NULL,
    .curr = NULL,               /* Prevents from the scheduler to run before initialised */
};

/**
 * Initialise the round robin scheduler.
 * 
 * XXX Document correctly.
 */
void thread_rr_init() {
    /*
     * Initialise the execution context.
     */
    thread_init(interrupt_stack, 
                THREAD_INITIAL_EXECUTION_CONTEXT(__thread_idle_context.tc_stack_bottom));

    /*
     * Setup the threading data structures.
     */
    thread_main.t_stack_top = NULL; /* Will be stored at the scheduler */
    thread_main.t_next      = &thread_main;
    thread_main.t_flags     = 0;

    /*
     * Set up the prev and curr correctly for the first scheduler call.
     * 
     * That is, there is no previous thread whose state needs to be
     * saved, and after the next context switch the main thread will
     * be the previous one.
     */
    g.prev = NULL;              /* Do not store execution context at first entry */
    g.curr = &thread_main;      /* Store our execution context at next entry */
}

/**
 * Add the thread to the ones scheduled. 
 * 
 * Must be called with the SysTick and any other scheduling interrupts
 * disabled.
 */
void thread_rr_start_locked(struct thread *t) {
    assert(t->t_next == NULL);
    assert(g.prev->t_next == g.curr);

    t->t_next = g.curr->t_next;
    g.curr->t_next = t;
}

/**
 * Selects the next thread to be run.
 *
 * Must be called with the SysTick and any other scheduling interrupts
 * disabled.
 *
 * Usually followed with an immediate call to thread_switch().
 */
void thread_rr_schedule_locked(void) {
    /*
     * Store the previous thread's execution context.  The
     * PendSV_Handler stores the top of the stack of the suspended
     * thread to the global variable, and here we record it at the
     * right place.
     */
    if (g.prev)
        g.prev->t_stack_top = __thread_other_execution_context;

    /*
     * Check if the scheduler is running.  If not, simply return.
     */
    if (!g.curr)
        return;

    /*
     * Shuffle the pointers to reflect the situation as it will be
     * once the thread switching will have already taken place.
     *
     * That is, after this point, up to the context switch,
     *  g.prev is the currently running thread
     *  g.curr is the next thread to run
     *
     * Then, after the context switch,
     *  g.prev is the thread that was switched out, i.e.,
     *         the still now currently running one
     *  g.curr is the thread that was switched in, i.e.,
     *         from the present point of view, the next thread to run 
     */
    g.prev = g.curr;
    g.curr = g.curr->t_next;

#ifdef notyet
    /*
     * Check if the newly scheduled thread is suspended, and
     * if so, find the first one that is not.
     *
     * If all threads are suspended, schedule the idle thread.
     */
    if (g.curr->t_flags & THREAD_SUSPENDED) {
        // XXX Check the following, I'm afraid of corner cases, e.g. only one suspended thread
        for (const struct thread *first = g.curr, g.curr = g.curr->t_next;
             g.curr->t_flags & THREAD_SUSPENDED;
             g.curr = g.curr->t_next) {
            if (g.curr == first) {
                // All threads are suspended
                g.curr = &thread_idle;
                break;
            }
        }
    }
#endif

    /*
     * Set the next thread's execution context for context switching.
     * The PendSV_Handler will pick this up from the global variable
     * when switching the execution contexts.
     */
    __thread_other_execution_context = g.curr->t_stack_top;
}

/**
 * Called by the current thread to terminate itself.
 *
 * May be called explicity or may be used as the
 * fallback return routine when initialising a thread. XXXX really ???
 * 
 * Must be called from a non-interrupt context.
 */
#ifdef notyet
void thread_rr_exit(void *return_value) {
    // Start critical section
    __asm__ volatile ("cpsid i");

    /*
     * First mark the thread as suspended and exited
     */
    g.curr->t_flags |= THREAD_SUSPENDED | THREAD_EXITED;

    /*
     * Now try to remove it form the circular list
     */
    // Optimise for the most common case
    if (g.prev && g.prev->t_next == g.curr)
        g.prev->t_next = g.curr->t_next;
    else {
        // g.prev either doens't exist or no longer points to us,
        // find the previous pointer the hard way.
        XXX;
    }

    /*
     * Signal any other threads waiting for this thread
     * to terminate.
     */
    XXX;

    /*
     * Call the scheduler to select the next thread to run.
     */
    thread_rr_schedule_locked();

    /*
     * Prevent the next scheduler call from recording
     * anything on this thread's context.
     */
    g.prev = NULL;

    /*
     * Request a context switch to take place after __thread_exit().
     */
    __thread_switch();

    /* 
     * Restore the thread stack to its bottom and replace the thread
     * with the idle thread, thereby causing it to disappear at the
     * following context switch.
     */

    register void *stack_bottom = g.curr->t_context->tc_stack_bottom;
    __asm__ volatile (
        "mov sp,%0"
        : 
        : "r"(stack_bottom)
        );
    __thread_exit();
    /* NOTREACHED */
}
#endif /* notyet */
