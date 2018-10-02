//#include "../System_Dispatcher.h"

#ifndef __XSIM_HARD_API_H__
#define __XSIM_HARD_API_H__

#define TRUE 0x01
#define FALSE 0x00
#define SUCCESSFULL 0x7F
#define FAIL 0x00

//********* ISO Function *********
//Initialize IO
//Baud rate	at 3.57 MHz
#define FD_9600    0x11
#define FD_56000   0x94
#define FD_112000  0x95
#define FD_115000  0x18
#define FD_224000  0x96

void Int_Init(void);
	
void IoInit(unsigned char FIDI);

//ISO Data send
void iso_tx(unsigned char tx_data);

//ISO Data get
unsigned char iso_rx(void);

//Flash read and write
unsigned char flash_rd(unsigned char *addr);
void flash_wr(unsigned char *addr, unsigned char val);

//extern void rx_buffer_wr(PCAPDU pAPDU);
extern unsigned char rx_buffer_rd(unsigned char i);
extern void SendRsp(unsigned short nSW);
void get_rand(unsigned char len, unsigned char*);
//void veriu_crc32(unsigned char* datain, unsigned char* crc32, unsigned int dinlen);
//unsigned char verify_veriu_crc32(unsigned char* datain, unsigned short dinlen);

#endif /*__XSIM_HARD_API_H__*/

