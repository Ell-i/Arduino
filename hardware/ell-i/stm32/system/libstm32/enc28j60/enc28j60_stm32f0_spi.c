/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

#include "stm32f0xx.h"

#include "enc28j60.h"
#include "enc28j60_spi.h"

#include "debug.h"

#define ENC_SPI SPI2

/*
 * 051 SPI DR is defined in the headers as a 16-bit register, even
 * when we are using 8-bit words in the SPI.  Now, when the RX and TX
 * FIFOs are used, each 16-bit read or write accesses two bytes in the
 * FIFO.  When writing, the LSB is the one sent first, and when
 * reading, LSB is the one that was received first.
 *
 * Here we attempt to keep the SPI busy all the time, even if there
 * was slight jitter caused by other threads (not implemented yet).
 * Hence, we first fill the TX FIFO with the command byte and the
 * first word from the buffer, i.e. altogether three bytes.
 *
 * The main loop waits for there to be space available in the TX FIFO,
 * i.e. there being space for at least two bytes (and at most two bytes)
 * in the FIFO, filling then the FIFO with the next two bytes.  At
 * that point the TX FIFO typically has four bytes in it temporarily.
 *
 * At the RX side, we first wait there to be two bytes in the RX FIFO,
 * and then read them, often emptying the FIFO.  After that we go back
 * to the writing side.
 *
 * Once there is nothing left for writing, read the last two bytes.
 *
 * XXX The plan is to replace this with DMA, but that hasn't been
 * implemented yet.
 */

volatile uint8_t * const DR8 = (volatile uint8_t *)&ENC_SPI->DR;

/**
 * Activates the SPI bus, writes a command on the SPI bus, writes and
 * reads one or two bytes depending on third_byte, and deactivates the
 * SPI bus.
 * @param cmd command byte sent first after SPI activation.
 * @param value a byte sent after the comand byte
 * @param third_byte if set, writes a third dummy byte on the SPI bus
 * and returns the byte received during that byte.
 * @returns the second or third byte read, depending on third_byte
 */

int
enc_spi_xfer(enc_spi_op_t cmd, enc_reg_value_t value, int third_byte) {
    int word;
    int dummy __attribute__((unused));

    DEBUG_SET_LED0(0);
    DEBUG_SET_LED2(1);
    /* Read any pertaining data from the FIFO and throw it away */
    while ((ENC_SPI->SR & SPI_SR_FRLVL) != 0) {
        dummy = ENC_SPI->DR;
        DEBUG_SET_LED1(0);
    }
    DEBUG_SET_LED2(0);

    /* Enable SPI, lowering the NSS */
    ENC_SPI->CR1 |= SPI_CR1_SPE;

    DEBUG_SET_LED1(0);
    /* The FIFO can hold three bytes, so write right away */
    ENC_SPI->DR = value << 8 | cmd;
    if (third_byte) {
        *DR8 = 0x00;   /* Dummy write for reading in the third byte */
    }
    DEBUG_SET_LED1(1);

    /* See RM0091 page 648 */
    /* Wait until the transmission is done */
    while ((ENC_SPI->SR & SPI_SR_BSY))
        ; // XXX: Add event wait and waking through interrupt

    /* Ensure we have at least 2 bytes in the input FIFO */
    while ((ENC_SPI->SR & SPI_SR_RXNE) == 0)
        ; // XXX: Add event wait and waking through interrupt

    /*
     * Read the second received byte.  See RM0091 page 649.
     */
    word = (ENC_SPI->DR >> 8) & 0xff;

    if (third_byte) {
        /* Ensure the third byte has been received. */
        while ((ENC_SPI->SR & SPI_SR_FRLVL) == 0)
            ; // XXX: Add event wait and waking through interrupt

        /* Read the third received byte. */
        word = *DR8;
    }

    /* Read any pertaining data from the FIFO and throw it away. */
    while ((ENC_SPI->SR & SPI_SR_FRLVL) != 0)
        dummy = ENC_SPI->DR;

    /* Disable SPI, raising the NSS. */
    ENC_SPI->CR1 &= ~SPI_CR1_SPE;

    DEBUG_SET_LED0(1);
    DEBUG_SET_LED1(0);

    return word;
}

/**
 * Transfers or received the given buffer to/from the ENC28J60
 * buffer space.  The RD or WD pointer has to be set correctly
 * when calling this function.
 *
 * @param len buffer length, must be even
 */
void
enc_spi_xfer_buffer(enc_spi_op_t cmd, enc_buf_value_t *buffer,
                    enc_buf_len_t len, int read) {
    register uint16_t word;
    int dummy __attribute__((unused));
    register uint16_t *wp, *rp;
    wp = rp = (uint16_t *)buffer;

    DEBUG_SET_LED0(0);
    DEBUG_SET_LED2(1);
    /* Read any pertaining data from the FIFO and throw it away. */
    while ((ENC_SPI->SR & SPI_SR_FRLVL) != 0)
        dummy = ENC_SPI->DR;
    DEBUG_SET_LED2(0);

    /* Enable SPI, lowering the NSS. */
    ENC_SPI->CR1 |= SPI_CR1_SPE;

    /* Send the command byte. */
    *DR8 = cmd;

    /* Write the first word to the FIFO. */
    ENC_SPI->DR = *wp++;

    DEBUG_SET_LED1(0);
    /* Wait for the received byte corresponding to the command. */
    while ((ENC_SPI->SR & SPI_SR_FRLVL) == 0)
        ; /* XXX.  Let other threads run. */
    dummy = *DR8; /* Throw the first received byte away. */
    DEBUG_SET_LED1(1);
    DEBUG_SET_LED0(1);

    /*
     * Write any next word, and read previous word.
     *
     * The strategy is that we keep the write FIFO
     * all the time at least two bytes long while
     * trying to keep the read FIFO empty.
     */
    for (len -=2; len > 0; len -= 2) {
        /* Ensure we can write to the output FIFO;
           though this is always true on the first round. */
        DEBUG_SET_LED1(0);
        while (!(ENC_SPI->SR & SPI_SR_TXE))
            ; /* XXX.  Let other threads run. */
        DEBUG_SET_LED1(1);

        ENC_SPI->DR = *wp++;

        /* Ensure we have at least 2 bytes in the input FIFO */
        DEBUG_SET_LED1(0);
        while ((ENC_SPI->SR & SPI_SR_RXNE) == 0)
            ; /* XXX.  Let other threads run. */
        DEBUG_SET_LED1(1);

        word = ENC_SPI->DR;
        if (read)
            *rp++ = word;
    }

    /* See RM0091 page 648 */
    /* Wait until the transmission is done */
    DEBUG_SET_LED2(1);
    while ((ENC_SPI->SR & SPI_SR_FTLVL) != 0)
        ; // XXX

    while ((ENC_SPI->SR & SPI_SR_BSY))
        ; // XXX

    /* Read the last word */
    /* Ensure we have at least 2 bytes in the input FIFO */
    while ((ENC_SPI->SR & SPI_SR_RXNE) == 0)
        ; // XXX

    word = ENC_SPI->DR;
    if (read)
        *rp++ = word;

    /* Read any pertaining data from the FIFO and throw it away. */
    while ((ENC_SPI->SR & SPI_SR_FRLVL) != 0)
        dummy = ENC_SPI->DR;
    DEBUG_SET_LED2(0);

    /* Disable SPI, lowering NSS */
    ENC_SPI->CR1 &= ~SPI_CR1_SPE;
}
