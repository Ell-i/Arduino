/*
 * Copyright (c) 2013, Matthias Kovatsch
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
 *      Erbium (Er) REST Engine example (with CoAP-specific code)
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "autostart.h"

#include "variant.h"


#include "erbium.h"

/* For CoAP-specific example: not required for normal RESTful Web service. */
#if WITH_COAP == 3
#include "er-coap-03.h"
#elif WITH_COAP == 7
#include "er-coap-07.h"
#elif WITH_COAP == 12
#include "er-coap-12.h"
#elif WITH_COAP == 13
#include "er-coap-13.h"
#else
#warning "Erbium example without CoAP-specifc functionality"
#endif /* CoAP-specific example */

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*
 * Resources are defined by the RESOURCE macro.
 * Signature: resource name, the RESTful methods it handles, and its URI path (omitting the leading slash).
 */
RESOURCE(helloworld, METHOD_GET, "hello", "title=\"Hello world: ?len=0..\";rt=\"Text\"");

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
void
helloworld_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const char *len = NULL;
  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
  char const * const message = "Hello World! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy";
  int length = 12; /*           |<-------->| */

  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
  if (REST.get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if (length<0) length = 0;
    if (length>REST_MAX_CHUNK_SIZE) length = REST_MAX_CHUNK_SIZE;
    memcpy(buffer, message, length);
  } else {
    memcpy(buffer, message, length);
  }

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *) &length, 1);
  REST.set_response_payload(response, buffer, length);
}



/* Sets the LED intencity. */

RESOURCE(leds, 
         METHOD_POST | METHOD_PUT , 
         "actuators/leds", 
         "title=\"LEDs: ?color=r|g|b, POST/PUT intensity=number\";rt=\"Control\"");

volatile uint32_t pwm[3]; /* XXX */

void
leds_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *color = NULL;
  const char *intensity = NULL;
  uint8_t led = 0;
  int success = 1;

  if ((len = REST.get_query_variable(request, "color", &color))) {
    PRINTF("Color %.*s\n", len, color);

    switch (color[0]) {
    case 'r': led = 0;     break;
    case 'g': led = 1;     break;
    case 'b': led = 2;     break;
    default : success = 0; break;
    }
  }

  if (success && (len = REST.get_post_variable(request, "intensity", &intensity))) {
    PRINTF("Intensity %s\n", intensity);

    int value = atoi(intensity);
    if (value < 0 || value > 255) {
        success = 0;
    } else {

        pwm[led] = (32*value)/256; /* XXX */
    }
  }

  if (!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}

/* Returns the reading from CPU ID a simple etag */
RESOURCE(cpuid, METHOD_GET, "sensors/cpuid", "title=\"CPU-ID\";rt=\"CPU-ID\"");
void
cpuid_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint16_t *accept = NULL;
  const char *format_string = "";
  uint32_t cpuid[3 /* XXX */];

  int num = REST.get_header_accept(request, &accept);

  getStmUniqueId(cpuid);
  
  if ((num==0) || (num && accept[0]==REST.type.TEXT_PLAIN)) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    format_string = "%08x-%08x-%08x";
  } else if (num && (accept[0]==REST.type.APPLICATION_JSON)) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    format_string = "{\"cpu-id\":\"%08x-%08x-%08x\"}";
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    format_string = "Supporting content-types text/plain and application/json";
  }
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, format_string, cpuid[0], cpuid[1], cpuid[2]);
  REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

/* Returns the reading from temperature sensor a simple etag */
RESOURCE(temperature, METHOD_GET, "sensors/temperature", "title=\"Chip internal temperature\";rt=\"Temperature\"");
void
temperature_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint16_t *accept = NULL;
  const char *format_string = "";

  int num = REST.get_header_accept(request, &accept);

  uint32_t temperature = -1;
  
  if ((num==0) || (num && accept[0]==REST.type.TEXT_PLAIN)) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    format_string = "%d";
  } else if (num && (accept[0]==REST.type.APPLICATION_JSON)) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    format_string = "{\"temperature\":\"%d\"}";
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    format_string = "Supporting content-types text/plain and application/json";
  }
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, format_string, temperature);
  REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

PROCESS(rest_server_example, "Erbium Example Server");
AUTOSTART_PROCESSES(&rest_server_example);

PROCESS_THREAD(rest_server_example, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("Starting Erbium Example Server\n");

  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */
  rest_activate_resource(&resource_helloworld);
  rest_activate_resource(&resource_leds);
  rest_activate_resource(&resource_cpuid);
  rest_activate_resource(&resource_temperature);

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  } 

  PROCESS_END();
}
