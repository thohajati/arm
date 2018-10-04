#ifndef VU_ISO14443_H
#define VU_ISO14443_H

#include "vutype.h"
//#include "vusfr.h"

#define ISO14443_SUCCESS 0x9100

//status  select application
#define PICC_LEVEL                 1
#define APPLICATION_LEVEL          2

typedef struct block {
  unsigned char  NAD;
  unsigned char  PCB;
  unsigned char  LEN;
  unsigned char  INF[256];
  unsigned char  LRC;
} BLOCK, *PBLOCK;

typedef struct apduheader{
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;
    uint8_t LEN;
}APDUHEADER,*PAPDUPHEADER;

typedef struct isoapdu{
    APDUHEADER*pheader;
    uint8_t* papdudata;
}ISOAPDU, *PISOAPDU;

typedef struct {
	unsigned char prev_block;
  unsigned char prev_command; 
  unsigned char	sel_app;
  unsigned char get_version;
  unsigned char authentication;
  unsigned char read_data;	 
  unsigned char write_data;	 
  unsigned char change_record;		 
  unsigned char change_value;
	unsigned char prev_length; 

}command_status;

uint16_t
iso14443receive(ISOAPDU* pisoapdu);

void
iso14443send(uint8_t* payload, uint16_t len, uint8_t status);
void iso7816send(uint8_t* payload, uint16_t len, uint8_t status);

void iso14443sendresp(uint16_t resp);

void 
vucalc_crc(uint8_t* pdatain,
           uint8_t* pcrcmsb, 
           uint8_t* pcrclsb,
           uint16_t len);

void vutransmit(uint8_t length);
unsigned long veriu_crc32(unsigned char* datain, unsigned char* crc32, unsigned int dinlen);
uint8_t verify_veriu_crc32(uint8_t* datain, unsigned int dinlen);
char xstsm212_verify_crc(unsigned char* datain, unsigned int dinlen);
void vuiso14443_init(void);
void iso14443waitreq(void);

#endif
