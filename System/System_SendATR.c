#include "reg.h"
#include "XSIM_hard_api.h"
#include "System_SendATR.h"
#include "vuiso14443.h"
#include "firmware.h"

const unsigned char s_abATR[]={0x3B,0x92,0x96,0x81,0x11,0xFE,0x33,0x44,0x1D};
//const unsigned char s_abATR[]={0x3B,0x92,0x11,0x81,0x11,0xFE,0x33,0x44,0x9A};  //slowest 0x9A

unsigned char f_pps = 1;
extern BLOCK block;

void System_SendATR(void) {
   unsigned char iATR;
   unsigned char aATR[4];
	
	 PBLOCK pblock = &block;

   // Send ATR
   for(iATR=0;iATR<sizeof(s_abATR);++iATR){
      iso_tx(s_abATR[iATR]);
   }

	 aATR[0] = iso_rx();
	 
	 if(aATR[0] == 0xFF){ // reader send PPS
		 
					//PPS
			for (iATR = 1; iATR < 4; ++iATR) {
				aATR[iATR] = iso_rx();	// PPS
			}

			delay_2(); //to connect with black reader acs contact

			//Accept All PPS
			for(iATR = 0; iATR < 4; ++iATR) {
				iso_tx(aATR[iATR]);		
			}
		
			//Change baud rate according to TA11
			IoInit(aATR[2]);
			f_pps = 1;
		}
	  else{
			pblock->NAD = aATR[0];
			f_pps = 0;
		}
}
