#include "reg.h"
#include "XSIM_hard_api.h"
#include "System_SendATR.h"
#include "firmware.h"

const unsigned char s_abATR[]={0x3B,0x92,0x96,0x81,0x11,0xFE,0x33,0x44,0x1D};

void System_SendATR(void) {
   unsigned char iATR;
   unsigned char aATR[4];  

   // Send ATR
   for(iATR=0;iATR<sizeof(s_abATR);++iATR){
      iso_tx(s_abATR[iATR]);
   }


   //PPS
   for (iATR = 0; iATR < 4; ++iATR) {
		aATR[iATR] = iso_rx();	// PPS
   }

	 	delay(); //to connect with black reader acs contact
	 
   //Accept All PPS
   for(iATR = 0; iATR < 4; ++iATR) {
	  iso_tx(aATR[iATR]);		
   }

   //Change baud rate according to TA11
   IoInit(aATR[2]);
}
