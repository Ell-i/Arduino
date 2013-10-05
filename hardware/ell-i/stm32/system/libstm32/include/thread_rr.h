/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

/*
 * Round robin scheduled threads
 */

enum thread_flags {
    THREAD_SUSPENDED = 0x01,
    THREAD_EXITED    = 0x02,
};

typedef struct thread {
    const struct thread_context *const t_context;
    struct thread_execution_context *t_stack_top;
    struct thread_rr *t_next;
    enum thread_flags t_flags;
} thread_t;

#define THREAD_DEFINE(name)                                       \
    struct thread __thread_ ## name = {                           \
        .t_context = &__thread_ ## name ## _context,              \
        .t_stack_top                                              \
           = THREAD_INITIAL_CONTEXT(&__thread_ ## name ## _stack) \
        .t_next = NULL,                                           \
    }

/*
 * Suspend a thread.
 *
 * Must be called with the SysTick and any other scheduling
 * interrupts disabled.  If called for the currently running
 * thread, suspends only after the next context switch.
 */
static inline void thread_rr_suspend_locked(struct thread *t) {
    t->t_flags |= THREAD_SUSPENDED;
}

/*
 * Resume a thread.
 *
 * Must be called with the SysTick and any other scheduling
 * interrupts disabled.
 */
static inline void thread_rr_resume_locked(struct thread *t) {
    t->t_flags &= ~THREAD_SUSPENDED;
}

