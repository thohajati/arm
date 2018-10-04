#include "XSIM_hard_api.h"
//#include "System_Dispatcher.h"
#include "System_StatusWord.h"
#include "reg.h"
#include "firmware.h"

/* Must be 2^y for modulo computation */
#define RX_BUF_SIZE 256

#define LOBYTE(w) ((unsigned char)((w) & 0xFF))
#define HIBYTE(w) LOBYTE((w) >> 8)

//_____ D E F I N I T I O N _____________
unsigned char rx_buffer[RX_BUF_SIZE]; // Rx circular buffer 

__asm void FlashLoader_ASM(void)
{
   MOVS  R0, #0
   LDR   R1,[R0]     ; Get initial MSP value
   MOV   SP, R1
   LDR   R1,[R0, #4] ; Get initial PC value
   BX    R1
}

//Interrupt Init
void Int_Init(void)
{
//	if(((VDET & 0x00000040)!=0x00000040))
//  {
//    // Set remap off ...
//		*(unsigned int *) CMSDK_SYSCTRL_BASE = 0x00000001;
//		
//	  __DSB();
//    __ISB();
//    FlashLoader_ASM();
//  }
		
	// Clear interrupt
//	FLASHX &= 0xFB; //int flash
		NVM_INT = 1; // Clear int flash
	ISO_DONE = 0;   //ISO 7816 
	CL_CON = 0x0;   //ISO 14443
	
	//*((volatile unsigned long *)(0xE000E100)) = 0x0003E100;//Enable interrupt #8, 13, 14, 15, 16, 17
	*((volatile unsigned long *)(0xE000E100)) = 0x0002E100;//Enable interrupt #8, 13, 14, 15, 17
	
	// Sleep SLEEPDEEP
	*(unsigned int*) 0xE000ED10 = 0x00000004;
	
}

//************** ISO Function ***************
//ISO IO Function : INIT, GET, SEND
//*******************************************

//*********** INIT ******************
//Baud rates at 3.57 MHz
void IoInit(unsigned char FIDI)
{  
   //Enable Vote and PEME mode
   ISOCON = 0x05;

   //Note ISO 7816 Initialized by HW with etu 372 (TA1=0x11)
   switch (FIDI)
   {
		 case FD_56000:
       ISO_TMR = 0x3F;
     break;
   
     case FD_112000:
        ISO_TMR = 0x1F;
     break;

     case FD_115000:
       ISO_TMR = 0x1E;
     break;

     case FD_224000:
       ISO_TMR = 0x0F;
     break;

     case FD_9600:
     default:
       ISO_TMR = 0x173;
     break;
   }
}

//************* ISO Data Send ***************
void iso_tx(unsigned char tx_data)
{
//	delay_3();
	// WFI  Test
	do {
	  ISO_DATA = tx_data;__WFI();
	}while ((ISOCON & 0x10) == 0x10);
}

//************* ISO Data Get *****************
unsigned char iso_rx(void)
{
  while(ISO_DONE == 0);
	ISO_DONE = 0;
	
	// WFI  Test
	//__WFI();
	
	/*
	// WFE  Test
	 DEBUG = 0;
	 while(DEBUG==0){
		 __WFE();
	 }
  */
   return(ISO_DATA); 
}

void flash_wr(unsigned char *addr, unsigned char val)
{
  *addr = val;          // addr is a pointer to external data mem
}

unsigned char flash_rd(unsigned char *addr)
{  
  return *addr;  
}

unsigned char rx_buffer_rd(unsigned char i){

  unsigned char rx_data;
  
  rx_data = rx_buffer[i];
	
  return(rx_data);

}

//void rx_buffer_wr(PCAPDU pAPDU){
//  unsigned char i;            
//	
//	//Send Ack
//  iso_tx((pAPDU->INS));
//	
//	for (i = 0; i < (pAPDU->P3); i++) 	{
//		rx_buffer[i] = iso_rx();
//	}
//}

void SendRsp(unsigned short nSW) {

  iso_tx(HIBYTE(nSW));
  iso_tx(LOBYTE(nSW));
}

void get_rand(unsigned char len, unsigned char* rand)
{
	unsigned char i;
	unsigned char seed = 0x99;
	
					//Enable TRG Clock
				CLKCON |= 0x04;
					
				RAND_DATA = seed;
				RAND_CTRL = 0x01;
				while ((RAND_CTRL & 0x02) == 0x00);
			
				for(i=0; i<len; i++)
					rand[i] = RAND_DATA; 

				RAND_CTRL &= 0xFE;
				//Disable TRG Clock
				CLKCON &= 0xFB;		
}

unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{
	ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
	ch = (ch^(ch<<4));
	*lpwCrc = (*lpwCrc >> 8)^((unsigned short)ch << 8)^((unsigned short)ch<<3)^((unsigned short)ch>>4);
	return(*lpwCrc);
}

void ComputeCrc(unsigned char *Data, unsigned short Length,
				unsigned char *TransmitFirst, unsigned char *TransmitSecond)
{
	unsigned char chBlock;
	unsigned short wCrc;

	wCrc = 0x6363; /* ITU-V.41 */

	do {
		chBlock = *Data++;
		UpdateCrc(chBlock, &wCrc);
	} while (--Length);

	*TransmitFirst = (unsigned char) (wCrc & 0xFF);
	*TransmitSecond = (unsigned char) ((wCrc >> 8) & 0xFF);
	return;
}

void 
vucalc_crc(unsigned char * pdatain,
           unsigned char * pcrcmsb, 
           unsigned char * pcrclsb,
           unsigned short len)
{
	int i;
	
	CLKCON |= 0x20;
	CRC_MODE = 0x1;
  CRC_INIT = 0x1;
	CRC_DATA = 0x6363;
	
	CRC_INIT = 0x0;
//	FLASHX = 0xF0;	
	
	for(i=0; i<len; i++){
//		FLASHX = i<<4;
	  CRC_DATA = *(pdatain+i);
	}
 
	*pcrcmsb = (unsigned char) (CRC_DATA>>8);
	*pcrclsb  = (unsigned char) CRC_DATA;
	 
	CLKCON &= 0xDF;

//	ComputeCrc(pdatain, len, pcrclsb, pcrcmsb);	
}

char xstsm212_verify_crc(unsigned char* datain, unsigned int dinlen)
{
	int i, crc_msb, crc_lsb;  


	CRC_MODE = 0x1;
  CRC_INIT = 0x1;
	CRC_DATA = 0x6363;
	
	CRC_INIT = 0x0;		
//	FLASHX = 0xF0;
    
    //Calculate crc
    for (i=0; i<dinlen; i++){
//			FLASHX = i<<4;
	    CRC_DATA = datain[i];    	
    }
    
    //CRC Result
    crc_msb = (unsigned char)CRC_DATA>>8;//crc msb
    crc_lsb = (unsigned char)CRC_DATA;   

	if (crc_msb == 0 && crc_lsb == 0){
		return 0;
	}else{
		return 1;
	}
//	unsigned char crc_msb, crc_lsb;  
// 
//    ComputeCrc(datain, dinlen, &crc_lsb, &crc_msb);

//	if (crc_msb == 0 && crc_lsb == 0){
//		return 0;
//	}else{	   
//		return 1;
//	}


}

//uint8_t verify_veriu_crc32(uint8_t* datain, uint16_t dinlen)
//{
//	unsigned int i;
//	unsigned char xdata crc32[4];

//	CRCMODE = 0x3;
//	CRC0 = 0; //init

//	for(i=0; i<dinlen; i++){
//  		CRC1 = datain[i]; //input data    	
//	}
//    
//    //CRC Result
//    crc32[0] = CRC1; //lsb
//    crc32[1] = CRC0;       
//    crc32[2] = CRC3;
//    crc32[3] = CRC2; //msb

//	for (i=0; i<4; i++)
//	{
//		if (crc32[i] != 0)
//			return 0;		
//	}
//	
//	return 1;
//}

//void veriu_crc32(unsigned char* datain, unsigned char* crc32, unsigned int dinlen)
//{
//	unsigned long i;

//	CRCMODE = 0x3;
//	CRC0 = 0; //init

//	for(i=0; i<dinlen; i++){
//  		CRC1 = datain[i]; //input data    	
//	}
//    
//    //CRC Result
//    crc32[0] = CRC1; //lsb
//    crc32[1] = CRC0;       
//    crc32[2] = CRC3;
//    crc32[3] = CRC2; //msb

//}

void ISO_RX_Handler(void)
{ 
	ISO_DONE = 0;
	//DEBUG++;
}

void ISO_TX_Handler(void)
{ 
	ISO_DONE = 0;
	//DEBUG++;
}

void FLASH_Handler(void)
{ 
//	FLASHX &= 0xFB;
	NVM_INT &= 1; // Clear int flash
}

void ETU_Timer_Handler(void)
{ 
}

void NFC_RX_Handler(void)
{ 
	CL_CON = 0x0;
	//DEBUG++;
}

void NFC_TX_Handler(void)
{ 
	CL_CON = 0x0;
	//DEBUG++;
}

