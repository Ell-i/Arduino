
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void init( void ) {
    /*
      In STM32F, SystemInit and __libc_init_array are called from the Reset_Handler,
      before main() gets called, and there is no need to call them again from here.
    */
}

uint32_t GetTickCount(void) {
    return 0;
}

#ifdef __cplusplus
}
#endif
