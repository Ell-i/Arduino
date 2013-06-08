/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Code for tunnelling uIP packets over the Rime mesh routing module
 *
 * \author  Adam Dunkels <adam@sics.se>\author
 * \author  Mathilde Durvy <mdurvy@cisco.com> (IPv6 related code)
 * \author  Julien Abeille <jabeille@cisco.com> (IPv6 related code)
 */

#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN + uip_ext_len])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

process_event_t tcpip_event;

/* Periodic check of active connections. */
static struct etimer periodic;

#if UIP_TCP
/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  uint16_t port;
  struct process *p;
};

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  struct process *p;
} s;
#endif

enum {
  TCP_POLL,
  UDP_POLL,
  PACKET_INPUT
};


static uint8_t (* outputfunc)(void);
uint8_t
tcpip_output(void)
{
  if(outputfunc != NULL) {
    return outputfunc();
  }
  UIP_LOG("tcpip_output: Use tcpip_set_outputfunc() to set an output function");
  return 0;
}

void
tcpip_set_outputfunc(uint8_t (*f)(void))
{
  outputfunc = f;
}
#endif

PROCESS(tcpip_process, "TCP/IP stack");

/*---------------------------------------------------------------------------*/
static void
start_periodic_tcp_timer(void)
{
  if(etimer_expired(&periodic)) {
    etimer_restart(&periodic);
  }
}
/*---------------------------------------------------------------------------*/
static void
check_for_tcp_syn(void)
{
  /* This is a hack that is needed to start the periodic TCP timer if
     an incoming packet contains a SYN: since uIP does not inform the
     application if a SYN arrives, we have no other way of starting
     this timer.  This function is called for every incoming IP packet
     to check for such SYNs. */
#define TCP_SYN 0x02
  if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
     (UIP_TCP_BUF->flags & TCP_SYN) == TCP_SYN) {
    start_periodic_tcp_timer();
  }
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  if(uip_len > 0) {
    check_for_tcp_syn();
    uip_input();
    if(uip_len > 0) {
      PRINTF("tcpip packet_input output len %d\n", uip_len);
      tcpip_output();
    }
  }
}
/*---------------------------------------------------------------------------*/
#if UIP_TCP
#if UIP_ACTIVE_OPEN
struct uip_conn *
tcp_connect(uip_ipaddr_t *ripaddr, uint16_t port, void *appstate)
{
  struct uip_conn *c;

  c = uip_connect(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  c->appstate.p = PROCESS_CURRENT();
  c->appstate.state = appstate;

  tcpip_poll_tcp(c);

  return c;
}
#endif /* UIP_ACTIVE_OPEN */
/*---------------------------------------------------------------------------*/
void
tcp_unlisten(uint16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == port &&
       l->p == PROCESS_CURRENT()) {
      l->port = 0;
      uip_unlisten(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_listen(uint16_t port)
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = port;
      l->p = PROCESS_CURRENT();
      uip_listen(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_attach(struct uip_conn *conn,
	   void *appstate)
{
  register uip_tcp_appstate_t *s;

  s = &conn->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;
}

#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
udp_attach(struct uip_udp_conn *conn,
	   void *appstate)
{
  register uip_udp_appstate_t *s;

  s = &conn->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, uint16_t port, void *appstate)
{
  struct uip_udp_conn *c;
  uip_udp_appstate_t *s;

  c = uip_udp_new(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  s = &c->appstate;
  s->p = PROCESS_CURRENT();
  s->state = appstate;

  return c;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_broadcast_new(uint16_t port, void *appstate)
{
  uip_ipaddr_t addr;
  struct uip_udp_conn *conn;

  uip_ipaddr(&addr, 255,255,255,255);
  conn = udp_new(&addr, port, appstate);
  if(conn != NULL) {
    udp_bind(conn, port);
  }
  return conn;
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
static void
eventhandler(process_event_t ev, process_data_t data)
{
#if UIP_TCP
  static unsigned char i;
  register struct listenport *l;
#endif /*UIP_TCP*/
  struct process *p;

  switch(ev) {
    case PROCESS_EVENT_EXITED:
      /* This is the event we get if a process has exited. We go through
         the TCP/IP tables to see if this process had any open
         connections or listening TCP ports. If so, we'll close those
         connections. */

      p = (struct process *)data;
#if UIP_TCP
      l = s.listenports;
      for(i = 0; i < UIP_LISTENPORTS; ++i) {
        if(l->p == p) {
          uip_unlisten(l->port);
          l->port = 0;
          l->p = PROCESS_NONE;
        }
        ++l;
      }

      {
        register struct uip_conn *cptr;

        for(cptr = &uip_conns[0]; cptr < &uip_conns[UIP_CONNS]; ++cptr) {
          if(cptr->appstate.p == p) {
            cptr->appstate.p = PROCESS_NONE;
            cptr->tcpstateflags = UIP_CLOSED;
          }
        }
      }
#endif /* UIP_TCP */
#if UIP_UDP
      {
        register struct uip_udp_conn *cptr;

        for(cptr = &uip_udp_conns[0];
            cptr < &uip_udp_conns[UIP_UDP_CONNS]; ++cptr) {
          if(cptr->appstate.p == p) {
            cptr->lport = 0;
          }
        }
      }
#endif /* UIP_UDP */
      break;

    case PROCESS_EVENT_TIMER:
      /* We get this event if one of our timers have expired. */
      {
        /* Check the clock so see if we should call the periodic uIP
           processing. */
        if(data == &periodic &&
           etimer_expired(&periodic)) {
#if UIP_TCP
          for(i = 0; i < UIP_CONNS; ++i) {
            if(uip_conn_active(i)) {
              /* Only restart the timer if there are active
                 connections. */
              etimer_restart(&periodic);
              uip_periodic(i);
              if(uip_len > 0) {
		PRINTF("tcpip_output from periodic len %d\n", uip_len);
                tcpip_output();
		PRINTF("tcpip_output after periodic len %d\n", uip_len);
              }
            }
          }
#endif /* UIP_TCP */
        }

      }
      break;

#if UIP_TCP
    case TCP_POLL:
      if(data != NULL) {
        uip_poll_conn(data);
        if(uip_len > 0) {
	  PRINTF("tcpip_output from tcp poll len %d\n", uip_len);
          tcpip_output();
        }
        /* Start the periodic polling, if it isn't already active. */
        start_periodic_tcp_timer();
      }
      break;
#endif /* UIP_TCP */
#if UIP_UDP
    case UDP_POLL:
      if(data != NULL) {
        uip_udp_periodic_conn(data);
        if(uip_len > 0) {
          tcpip_output();
        }
      }
      break;
#endif /* UIP_UDP */

    case PACKET_INPUT:
      packet_input();
      break;
  };
}
/*---------------------------------------------------------------------------*/
void
tcpip_input(void)
{
  process_post_synch(&tcpip_process, PACKET_INPUT, NULL);
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
tcpip_poll_udp(struct uip_udp_conn *conn)
{
  process_post(&tcpip_process, UDP_POLL, conn);
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_TCP
void
tcpip_poll_tcp(struct uip_conn *conn)
{
  process_post(&tcpip_process, TCP_POLL, conn);
}
#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
void
tcpip_uipcall(void)
{
  register uip_udp_appstate_t *ts;

#if UIP_UDP
  if(uip_conn != NULL) {
    ts = &uip_conn->appstate;
  } else {
    ts = &uip_udp_conn->appstate;
  }
#else /* UIP_UDP */
  ts = &uip_conn->appstate;
#endif /* UIP_UDP */

#if UIP_TCP
 {
   static unsigned char i;
   register struct listenport *l;

   /* If this is a connection request for a listening port, we must
      mark the connection with the right process ID. */
   if(uip_connected()) {
     l = &s.listenports[0];
     for(i = 0; i < UIP_LISTENPORTS; ++i) {
       if(l->port == uip_conn->lport &&
	  l->p != PROCESS_NONE) {
	 ts->p = l->p;
	 ts->state = NULL;
	 break;
       }
       ++l;
     }

     /* Start the periodic polling, if it isn't already active. */
     start_periodic_tcp_timer();
   }
 }
#endif /* UIP_TCP */

  if(ts->p != NULL) {
    process_post_synch(ts->p, tcpip_event, ts->state);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcpip_process, ev, data)
{
  PROCESS_BEGIN();

#if UIP_TCP
 {
   static unsigned char i;

   for(i = 0; i < UIP_LISTENPORTS; ++i) {
     s.listenports[i].port = 0;
   }
   s.p = PROCESS_CURRENT();
 }
#endif

  tcpip_event = process_alloc_event();
  etimer_set(&periodic, CLOCK_SECOND / 2);

  uip_init();

  while(1) {
    PROCESS_YIELD();
    eventhandler(ev, data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
