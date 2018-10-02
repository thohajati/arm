#ifndef VU_TDES_H
#define VU_TDES_H

void xstsm212_tdes16_encrypt_tx(unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen);
void xstsm212_tdes16_encrypt_rx(unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen);
void tdes16_encrypt_rx(unsigned char *data_init, unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen);
void tdes16_encrypt_tx(unsigned char *data_init, unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen);
void xstsm212_tdes_encrypt(unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned int dinlen);
#endif
