/*
  Copyright (c) 2013 Ell-i.  All rights reserved.
 */

#include <stm32f0xx.h>

#include <assert.h>
#include <stddef.h>

#include <uip.h>
#include <uip_arp.h>

#include <enc28j60.h>

#if 0
#include <core/sys/pt.h>
#include <core/sys/process.h>
#include <core/net/tcpip.h>
#include <core/sys/etimer.h>
#include <apps/webserver/webserver-nogui.h>
#include <core/sys/clock.h>
#endif

#include "debug.h"

extern volatile uint32_t millisecondCount;

void net_init() {
    uip_init();
    uip_ipaddr_t addr;
    uip_ipaddr(&addr, 10,0,0,2);
    uip_sethostaddr(&addr);
}

void net_loop() {
    static clock_time_t prevTime;

    if (prevTime == 0)
        prevTime = millisecondCount;

    /*
     * Process Ethernet packets
     */
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
    DEBUG_SET_LED3(1);
    DEBUG_SET_LED1(1);
    uip_len = enc_packet_receive(uip_buf, sizeof(uip_buf));
    DEBUG_SET_LED1(0);
    if (uip_len > 0) {
        if (BUF->type == UIP_HTONS(UIP_ETHTYPE_IP)) {
            uip_arp_ipin();
            DEBUG_SET_LED4(1);
            uip_input();
            DEBUG_SET_LED4(0);
            if (uip_len > 0) {
                uip_arp_out();
            }
        } else if (BUF->type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
            uip_arp_arpin();
        }
        if (uip_len > 0) {
            DEBUG_SET_LED5(1);
            enc_packet_send(uip_buf, uip_len);
            DEBUG_SET_LED5(0);
        }
    }
    DEBUG_SET_LED3(0);

#if 0
    /*
     * Process upper-layer processes
     */
    DEBUG_SET_LED2(1);  // Yellow
    while (process_run() > 0)
        ;
    DEBUG_SET_LED2(0);
#endif
    if (millisecondCount > prevTime + 50 || millisecondCount < prevTime) {
        /*
         * Process TCP connections every 50ms
         */
        DEBUG_SET_LED3(1);
        for (int i = 0; i < UIP_CONNS; ++i) {
            GPIOC->ODR ^= GPIO_ODR_8;
            uip_periodic(i);
            GPIOC->ODR ^= GPIO_ODR_8;
            if (uip_len > 0) {
                uip_arp_out();
                enc_packet_send(uip_buf, uip_len);
            }
        }
        DEBUG_SET_LED3(0);  // Green
        prevTime = millisecondCount;
    }
}
