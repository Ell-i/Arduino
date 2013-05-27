/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

/*
 * ENC28J60 RX/TX buffer assignment
 *
 * The RX buffer should start at zero. See Rev. B4 Silicon Errata.
 *
 * Leave one Ethernet packet worth for the TX buffer, 0x600 = 1536 bytes
 */
#define MAX_PACKET_SIZE 0x0600

#define RX_BUFFER_START (ENC_BUFFER_START)
#define RX_BUFFER_END   (TX_BUFFER_START - 1)
#define TX_BUFFER_START (ENC_BUFFER_START + ENC_BUFFER_SIZE - MAX_PACKET_SIZE)
#define TX_BUFFER_END   (ENC_BUFFER_START + ENC_BUFFER_SIZE)

