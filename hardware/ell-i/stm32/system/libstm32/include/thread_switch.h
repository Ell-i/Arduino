/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

#include <stdint.h>
#include <stm32f0xx.h>

#ifdef __cplusplus
extern "C" {
#endif

struct thread_context {
    volatile uint32_t *const tc_stack_bottom;  /* Memory addres *following* the stack */
    volatile uint32_t *const tc_stack_ceiling; /* First address *within* the stack */
};

#ifndef EXC_RETURN_PSP
#define EXC_RETURN_PSP ((void *)0xfffffffd)
#endif /* EXC_RETURN_PSP */

#ifndef EXC_RETURN_MSP
#define EXC_RETURN_MSP ((void *)0xfffffff9)
#endif /* EXC_RETURN_PSP */

#if 0
// Not a constant initialiser, based on CMSIS definitions
const static xPSR_Type XPSR_DEFAULT_VALUE = {
    .b = { 
        .ISR = 0,               /* Thread state */
        .T   = 1, 
        /* All others zero */
    },
};
#else
// A constant initialiser, based on reading the documentation
#define XPSR_DEFAULT_VALUE (1 << 24)
#endif

/**
 * The memory image of an execution context.
 */
struct thread_execution_context {
    uint32_t lr_exc_return;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr_thread;
    uint32_t pc;
    uint32_t xpsr;
};

/**
 * Request a context switch.  Called by the scheduler if it determines
 * that the currently running thread needs to be switched.  
 * 
 * The actual switch is implemented by PendSV_Handler, which is called
 * once all other pending interrupts have been serviced.  If
 * thread_switch() is called from user level code, the switch is
 * imminent. 
 */
static inline void __thread_switch() {
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* See thread_switch.c  */
extern void 
__thread_init(void *interrupt_stack, struct thread_execution_context *idle_context);

/* See thread_exit.c */
extern __attribute__ ((naked)) void 
__thread_exit(uint32_t return_value);

/* See thread_idle.c */
extern const struct thread_context __thread_idle_context;
extern __attribute__ ((naked)) void __thread_idle_entry();

#define THREAD_STACK_SIZE_BYTES_MINIMUM (sizeof(struct thread_execution_context))

/**
 * Statically allocate space for a stack.
 */
#define THREAD_DEFINE_EXECUTION_CONTEXT(name, stack_size_bytes, function, \
                              exit_function, arg0, arg1, arg2, arg3)    \
volatile struct {                                                       \
    uint32_t stack_space[((stack_size_bytes)                            \
                           - sizeof(struct thread_execution_context)    \
                           + (sizeof(uint32_t) - 1))                    \
                         / sizeof(uint32_t)];                           \
    struct thread_execution_context initial_execution_context;          \
} __thread_ ## name ## _stack = {                                       \
    .initial_execution_context = {                                      \
        .xpsr = XPSR_DEFAULT_VALUE,                                     \
        .pc = (uint32_t)(function),                                     \
        .lr_thread = (uint32_t)(exit_function),                         \
        .r0 = (arg0),                                                   \
        .r1 = (arg1),                                                   \
        .r2 = (arg2),                                                   \
        .r3 = (arg3),                                                   \
        .lr_exc_return = EXC_RETURN_PSP,                                \
        /* All others zero */                                           \
    },                                                                  \
}
/**
 * Statically define an execution context, assuming the stack has been allocated.
 */
#define THREAD_DEFINE_CONTEXT(name)                                     \
const struct thread_context __thread_ ## name ## _context = {           \
    .tc_stack_bottom  =                                                 \
         ((char *)&__thread_ ## name ## _stack.initial_execution_context) \
         + sizeof(__thread_ ## name ## _stack.initial_execution_context),\
    .tc_stack_ceiling =                                                 \
         __thread_ ## name ## _stack.stack_space,                       \
}

/**
 * Return the memory image of the initial execution context,
 * given the bottom of its stack.  Usually the top of the stack is
 * used for this.
 *
 * XXX Do we need this?  Should this be removed?
 */
#define THREAD_INITIAL_EXECUTION_CONTEXT(stack_bottom)                  \
    ((struct thread_execution_context *)                                \
     (((char *)(stack_bottom)) - sizeof(struct thread_execution_context)))

extern volatile struct thread_execution_context *volatile 
     __thread_other_execution_context;

#ifdef __cplusplus
} /* extern "C" */
#endif
