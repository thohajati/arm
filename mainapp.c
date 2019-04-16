#include "vuiso14443.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_Hard_Api.h"
#include "System_Dispatcher.h"
#include "System_SendATR.h"
#include "HAL_Init.h"
#include "cmd.h"


ISOAPDU  isoapdu;
//uint8_t  app_buff_main[1] __attribute__((at(0xa000)));//_at_ 0xa000;

int system_dispatcher(ISOAPDU  *pisoapdu);
int main()
{      
    //uint16_t   rxlen;
		CLKCON |= 0x02;
	
		Int_Init();
	
		if(master_key[0] == 0xFF)
		{
	     master_key_format();	  
		} 

		vuiso14443_init();

		if((VDET & 0x40) == 0x40){ //mode contact
			  HAL_Init();
			  System_SendATR();
				System_Dispatcher();
		}else{

			while(1){
					iso14443receive(&isoapdu);
					system_dispatcher(&isoapdu);
			
			}
	}
}

int system_dispatcher(ISOAPDU  *pisoapdu)
{
	    if((isoapdu.pheader->CLA == 0x90) || (isoapdu.pheader->CLA == 0xC0))
		{
		
			firmware_response(&isoapdu); 	
		}
		else if(isoapdu.pheader->CLA == 0x00)
		{
			firmware_iso7816_response(&isoapdu);
		}
		else if(isoapdu.pheader->CLA == 0x51)
		{
			firmware_xirka_response(&isoapdu);
		}
		else
		{			
			iso14443sendresp(0x6E00);
		}
	return 0;
}
