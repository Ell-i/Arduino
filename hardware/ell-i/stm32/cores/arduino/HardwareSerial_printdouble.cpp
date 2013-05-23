/*
  Copyright (c) 2013 Ell-i.  All right reserved.
*/

#include <Arduino.h>

/*
 * NB.  This may not be the most efficient way of printing numbers
 *      but it uses a minimum of memory.
 */
size_t HardwareSerial::print(double value, int decimals) const {
    int n;

    if (isnan(value)) return print("NAN");
    if (isinf(value)) return print("INF");

    if (value < 0.0) {
        n += write('-');
        value = -value;
    }

    /* Round up according to the number of decimals */
    double rounding = 0.5;
    for (int i = 0; i < decimals; i++) {
        rounding *= 0.1;
    }
    value += rounding;

    double mul = 1.0;
    for (double h = value; h > 10.0; h /= 10) {
        mul *= 10.0;
    }
    for (mul; mul >= 1.0; mul /= 10.0) {
        const int digit = value / mul;
        n += write(digit + '0');
        value -= digit * mul;
    }

    if (decimals <= 0)
        return n;

    n += write('.');

    for (int i = 0; i < decimals; i++) {
        value *= 10;
        const int digit = value; // Cut off decimals
        n += write(digit + '0');
    }

    return n;
}
