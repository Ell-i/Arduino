/*
  Copyright (c) 2013 Ell-i.  All rights reserved.
 */

#include <stm32f0xx.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include <thread_switch.h>

#include <uip.h>
#include <uip_arp.h>

#include <enc28j60.h>

#include <sys/autostart.h>
#include <sys/pt.h>
#include <net/dhcpc.h>
#include <sys/process.h>
#include <net/tcpip.h>
#include <sys/etimer.h>
//#include <apps/webserver/webserver-nogui.h>
#include <sys/clock.h>

#include <apps/erbium/erbium.h>

#include "debug.h"
#include "net.h"

static uint32_t interrupt_stack[32];

extern volatile uint32_t millisecondCount;

static THREAD_DEFINE_EXECUTION_CONTEXT(net, 1024+256, net_thread, __thread_exit, 0, 0, 0, 0);
static THREAD_DEFINE_CONTEXT(net);

struct uip_eth_addr mac_address = {
    .addr = { 0x00, 0x04, 0xa3, 0, 0, 0 },
};

void net_spawn() {
#if 1
    __thread_init(interrupt_stack,
                  THREAD_INITIAL_EXECUTION_CONTEXT(__thread_net_context.tc_stack_bottom));
#endif
}

extern struct process etimer_process, tcpip_process, dhcp_process, webserver_nogui_process;
extern struct process rest_server_example;

#define AUTOSTART_ENABLE

void net_thread(/* XXX Add parameters, IP address etc */) {
    net_init();

    for (;;) {
        net_loop();
        __thread_switch();
    }
}

static uint8_t net_output(void) {
    printf("NET_OUTPUT: Sending %d bytes\n", uip_len);
    uip_arp_out();
    enc_packet_send(uip_buf, uip_len);
    return 0;
}

PROCESS(dhcp_process, "DHCP");

PROCESS_THREAD(dhcp_process, ev, data)
{
    printf("DHCPC: Starting.\n");

    PROCESS_BEGIN();
    
    dhcpc_init(uip_lladdr.addr, sizeof(uip_lladdr.addr));
  
    while(1) {
        PROCESS_WAIT_EVENT();
    
        if (ev == PROCESS_EVENT_INIT) {
            printf("DHCPC: Sending a request\n");
            dhcpc_request();
        } else if (ev == tcpip_event) {
            dhcpc_appcall(ev, data);
        } else if(ev == PROCESS_EVENT_EXIT) {
            printf("DHCPC: Exiting\n");
            process_exit(&dhcp_process);
        }
    }
    
    PROCESS_END();
}

void dhcpc_configured(const struct dhcpc_state *s) {
    printf("DHCPC: Got IP address %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->ipaddr));
    printf("DHCPC: Got netmask %d.%d.%d.%d\n",	 uip_ipaddr_to_quad(&s->netmask));
    printf("DHCPC: Got DNS server %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->dnsaddr));
    printf("DHCPC: Got default router %d.%d.%d.%d\n",
           uip_ipaddr_to_quad(&s->default_router));
    printf("DHCPC: Lease expires in %ld seconds\n",
           uip_ntohs(s->lease_time[0])*65536ul + uip_ntohs(s->lease_time[1]));

    uip_sethostaddr(&s->ipaddr);
    uip_setnetmask(&s->netmask);
    uip_setdraddr(&s->default_router);
    //resolv_conf(&s->dnsaddr); /* XXX */
}

void dhcpc_unconfigured(const struct dhcpc_state *s) {
    printf("DHCPC: Lost IP address\n");
    dhcpc_request();
}

void net_init() {

    /* SysTick end of count event each 1ms */
    SysTick_Config(RCC_GetHCLKFreq() / 1000); /* CMSIS */

    // clock_init(); // Not implemented / needed

    uint8_t cpuid[3 * sizeof(uint32_t)/* XXX */];

    getStmUniqueId((uint32_t *)cpuid);
    mac_address.addr[3] = cpuid[0]; /* XXX */
    mac_address.addr[4] = cpuid[2]; /* XXX */
    mac_address.addr[5] = cpuid[4]; /* XXX */

    DEBUG_SET_LED1(1);
    enc_init(mac_address.addr);
    DEBUG_SET_LED1(0);

    uip_init();
    tcpip_set_outputfunc(net_output);

    uip_setethaddr(mac_address);

#if 0
    uip_ipaddr_t addr;
    uip_ipaddr(&addr, 10,0,0,2);
    uip_sethostaddr(&addr);
#endif

#if 0
    // clock_init(); /// XXX Not defined nor used yet
#endif
    process_init();
    process_start(&etimer_process, NULL); 
    process_start(&tcpip_process, NULL);
    process_start(&dhcp_process, NULL);
    process_start(&rest_server_example, NULL);

    process_post(&dhcp_process, PROCESS_EVENT_INIT, NULL);
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
            printf("ETH1: Sending %d bytes\n", uip_len);
            enc_packet_send(uip_buf, uip_len);
            DEBUG_SET_LED5(0);
        }
    }
    DEBUG_SET_LED3(0);

    /*
     * Process upper-layer processes
     */
    DEBUG_SET_LED2(1);  // Yellow
    while (process_run() > 0)
        ;
    DEBUG_SET_LED2(0);

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
                printf("ETH2: Sending %d bytes\n", uip_len);
                enc_packet_send(uip_buf, uip_len);
            }
        }
        DEBUG_SET_LED3(0);  // Green
        prevTime = millisecondCount;
    }
}
