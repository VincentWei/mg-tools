#ifndef _PRODUCT_ID_H
#define _PRODUCT_ID_H

typedef struct _product_id {
    /* Fixed 8-byte code, help us to locate the struct in .so file */
    unsigned char prefix[8];
    /* the ID of the customer */
    int customer_id;
    /* svn version */
    int version;
    /* When does we compile the .so, in seconds */
    int compile_date;
    /* The size of the .so file */
    int file_size;
    /* The check sum of the .so file */
    unsigned char checksum[16];
} product_id_t;

#define PRODUCT_ID_PREFIX 0xca, 0x3f, 0x2b, 0x43, 0x00, 0x33, 0xb0, 0xc3

#endif
