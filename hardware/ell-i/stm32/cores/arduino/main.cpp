/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define ARDUINO_MAIN
#include "Arduino.h"
#include "debug.h"

#define NET 1

/*
 * Cortex-M3 Systick IT handler
 */
/*
extern void SysTick_Handler( void )
{
  // Increment tick count each ms
  TimeTick_Increment() ;
}
*/

extern "C" {
extern void init(void);
#if NET
extern void net_init(void);
extern void net_loop(void);
#endif
}

/*
 * \brief Main entry point of Arduino application
 */
int main( void )
{
    init();

#if DEBUG
    DEBUG_SET_LED0(1);
    delay(100);
    DEBUG_SET_LED0(0);
#endif

#if NET
    net_init();
#endif

    setup();

    DEBUG_SET_LED0(1);

    for (;;) {
        loop();
        if (serialEventRun) serialEventRun();
#if NET
        net_loop();
#endif
    }

    return 0;
}
