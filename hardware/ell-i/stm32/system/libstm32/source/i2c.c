/*
 * Copyright 2013 Pekka Nikander.  See NOTICE for licensing information.
 */

extern int
i2c_init(const struct *i2c_master) {
    
}

extern int
i2c_write(const struct *i2c_master, uint8_t addr, uint8_t *data, uint8_t len) {
    i2c_master->
}

extern int
i2c_write_then_read(const struct *i2c_master, uint8_t addr, 
                    uint8_t *write_data, uint8_t write_len,
                    uint8_t *read_buffer, uint8_t buffer_len) {
}

