/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

/*
 * A data driven approach to initialise STM32F peripherals.
 */

#include <stddef.h>

typedef struct device_register_init_static_16bit {
    uint16_t offset;
    uint16_t value;
} device_register_init_static_16bit_t;

typedef struct device_register_init_static_32bit {
    uint32_t offset;
    uint32_t value;
} device_register_init_static_32bit_t;

typedef struct device_register_init_masked_32bit {
    uint16_t offset;
    uint32_t mask;
    uint32_t value;
} device_register_init_masked_32bit_t;

typedef struct device_register_init_waited_32bit {
    uint16_t offset;
    uint32_t mask;
    uint32_t value;
} device_register_init_waited_32bit_t;

typedef enum device_register_init_type {
    DRI_STATIC_16BIT,
    DRI_STATIC_32BIT,
    DRI_MASKED_32BIT,
    DRI_WAITED_32BIT,
} device_register_init_type_t;

#define REG_OFFSET(device, reg) \
    ((size_t) ( (char *)&((device)->reg) - (char *)(device)))

#define DEVICE_REGISTER_INIT_STRUCT_VALUE16(device, reg, value) \
    { REG_OFFSET((device), reg), (value) }

#define DEVICE_REGISTER_INIT_STRUCT_VALUE32(device, reg, value) \
    { REG_OFFSET((device), reg), (value) }

#define DEVICE_REGISTER_INIT_STRUCT_MASK_VALUE32(device, reg, mask, value) \
    { REG_OFFSET((device), reg), (mask), (value) }

#define DEVICE_REGISTER_INIT_STRUCT_WAIT_VALUE32(device, reg, mask, value) \
    { REG_OFFSET((device), reg), (mask), (value) }

/*
 * NB.  One idea would be to split the tables below into an init.inc file,
 *      then include that twice, with different definitions for the macros.
 *
 *      Essence:
 *        #define D16(d, r, v) v
 *        #include "init.inc"
 *        #undef D16
 *        #define D16(d, r, v) OFFSET_OF(d, r)
 *        #include "init.inc"
 */

/*
 * From http://stackoverflow.com/questions/1598773/\
 * is-there-a-standard-function-in-c-that-would-return-the-length-of-an-array:
 *
 * In this version if a pointer is mistakenly passed as the argument,
 * the compiler will complain in some cases - specifically if the
 * pointer's size isn't evenly divisible by the size of the object the
 * pointer points to. In that situation a divide-by-zero will cause
 * the compiler to error out. Actually at least one compiler
 * gives a warning instead of an error.
 *
 * That macro doesn't close the door on using it erroneously, but it
 * comes close in straight C.
 */

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * XXX
 */

typedef struct device_register_init_descriptor {
    uint32_t                              *dri_device; /* Device register address */
    device_register_init_type_t            dri_type;
    uint16_t                               dri_count;
    union {
        const device_register_init_static_16bit_t *dri_static_16bit;
        const device_register_init_static_32bit_t *dri_static_32bit;
        const device_register_init_masked_32bit_t *dri_masked_32bit;
        const device_register_init_waited_32bit_t *dri_waited_32bit;
    } u;
} device_register_init_descriptor_t;

#define DRI_DESCRIPTOR_MASKED_32BIT(reg, table)  \
    {                                            \
        .dri_device = (uint32_t *)(reg),         \
        .dri_type     = DRI_MASKED_32BIT,        \
        .dri_count    = COUNT_OF(table),         \
        .u.dri_masked_32bit = table,             \
    }
#define DRI_DESCRIPTOR_STATIC_32BIT(reg, table)  \
    {                                            \
        .dri_device = (uint32_t *)(reg),         \
        .dri_type     = DRI_STATIC_32BIT,        \
        .dri_count    = COUNT_OF(table),         \
        .u.dri_static_32bit = table,             \
    }

#define DRI_DESCRIPTOR_STATIC_16BIT(reg, table)  \
    {                                            \
        .dri_device = (uint32_t *)(reg),         \
        .dri_type     = DRI_STATIC_16BIT,        \
        .dri_count    = COUNT_OF(table),         \
        .u.dri_static_16bit = table,             \
    }

#define DRI_DESCRIPTOR_WAITED_32BIT(reg, table)  \
    {                                            \
        .dri_device = (uint32_t *)(reg),         \
        .dri_type     = DRI_WAITED_32BIT,        \
        .dri_count    = COUNT_OF(table),         \
        .u.dri_waited_32bit = table,             \
    }

extern void Peripheral_Init(void);

