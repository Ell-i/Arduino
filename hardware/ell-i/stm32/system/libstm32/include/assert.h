/*
 * Copyright (c) 2013 Ell-i.org.  All rights reserved.
 */

void abort(void) __attribute__((noreturn));

static inline void assert(int a) {
    if (!a) abort();
}


