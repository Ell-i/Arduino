/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

extern int
enc_spi_xfer(enc_spi_op_t cmd, enc_reg_value_t value, int nbytes);

extern void
enc_spi_xfer_buffer(enc_spi_op_t cmd, enc_buf_value_t *buffer, enc_buf_len_t len, int read);

