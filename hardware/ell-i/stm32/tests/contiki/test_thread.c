
#include <thread.h>
#include <thread_contiki.h>

PROCESS(test_process, "test");

PROCESS_THREAD(test_process, ev, data) {
    static struct etimer _timer;
    const static struct etimer *timer = &_timer;
    
    PROCESS_BEGIN();

    etimer_set(timer, CLOCK_CONF_SECOND);

    for (;;) {
        PROCESS_WAIT_EVENT();

        etimer_reset(timer);
    }

    PROCESS_END();
}

