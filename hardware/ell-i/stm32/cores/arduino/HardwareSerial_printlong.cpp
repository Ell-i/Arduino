/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>

/*
 * NB.  This may not be the most efficient way of printing numbers
 *      but properly optimised it produces quite compact code and
 *      uses a minimum of memory.
 */
size_t HardwareSerial::print(unsigned long value, enum base base, bool isUnsigned) const {
    int n = 0;

    if (!isUnsigned && ((long)value) < 0) {
        n += write('-');
        value = -((long)value);
    }
    /*
     * Compute how many digits are needed to print out the value,
     * i.e. what is the highest multiplier.
     */
    unsigned long mul = 1;
    for (unsigned long h = value; h > base; h /= base) {
        mul *= base;
    }
    /* Write out the digits, starting from most significant one. */
    for (mul; mul >= 1; mul /= base) {
        const int digit = value / mul;
        n += write(digit + (digit < 10? '0': 'A'));
        value = value % mul;
    }

    return n;
}

