/*
 * Copyright (c) 2013 Ell-i.org.  All rights reserved.
 */

#if 1
#define DEBUG_SET_LED0(x) DEBUG_SET_LED(C, 6, (x))
#define DEBUG_SET_LED1(x) DEBUG_SET_LED(C, 7, (x))
#define DEBUG_SET_LED2(x) DEBUG_SET_LED(C, 8, (x))
#define DEBUG_SET_LED3(x) DEBUG_SET_LED(C, 9, (x))
#define DEBUG_SET_LED4(x) DEBUG_SET_LED(A, 8, (x))
#define DEBUG_SET_LED5(x) DEBUG_SET_LED(A, 9, (x))
#define DEBUG_SET_LED6(x) DEBUG_SET_LED(A, 10, (x))
#else
#define DEBUG_SET_LED0(x)
#define DEBUG_SET_LED1(x)
#define DEBUG_SET_LED2(x)
#define DEBUG_SET_LED3(x)
#define DEBUG_SET_LED4(x)
#define DEBUG_SET_LED5(x)
#define DEBUG_SET_LED6(x)

#endif

#define DEBUG_SET_LED(port, pin, value) \
    (GPIO ## port->BSRR = (value)? GPIO_BSRR_BS_ ## pin: GPIO_BSRR_BR_ ## pin)

