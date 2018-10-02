#ifndef VUAES_H
#define VUAES_H

#include "vutype.h"

#define AES_MODE_ENCRYPT        0x04 // encryption mode
#define AES_MODE_DECRYPT        0x00 // decryption mode



typedef struct aesinput{
    unsigned int * presult;
		unsigned int * pkey;
    unsigned int * piv;
    unsigned int * pbuffer;
    uint16_t bufferlen;
    uint8_t mode;
}aes_t, *PINPUTAES;


void vuaes_insertkey16byte(unsigned int * pkey);

/* hati hati pointer presult akan bergerak sebesar bufferlen */
void vuaes_process(PINPUTAES pinp);


/* PASTIKAN buffer yg ditunjuk presult panjangnya HARUS 16 byte!!*/
void vuaes_cmac(PINPUTAES pinput);


#endif
