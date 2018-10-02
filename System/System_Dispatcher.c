#include <stdio.h>
#include <string.h>

#include "XSIM_hard_api.h"
#include "System_StatusWord.h"
#include "reg.h"
#include "vuiso14443.h"
#include "firmware.h"
#include "flash.h"	  	  
#include "vutdes.h"	   	  
#include "cmd.h"	 
#include "block_handler.h"	
#include "System_Dispatcher.h"

extern BLOCK block;

void init()
{
    // initiate status
   	cmd_status.sel_app = PICC_LEVEL;
	cmd_status.get_version = 0;
	cmd_status.authentication = 0;
	cmd_status.read_data = 0;
	cmd_status.write_data = 0;

	app_idx = 0;
	file_idx = 0; 
	key_idx = 0;

    read_all_flag = 0;
}

int RecvBlockFrame(PBLOCK pblock)
{
	unsigned char i;

	pblock->NAD = iso_rx();
	pblock->PCB = iso_rx(); 
	pblock->LEN = iso_rx(); 

	for(i=0; i<pblock->LEN; i++)
		pblock->INF[i] = iso_rx();
		
  pblock->LRC = iso_rx();	//lrc	
	
	return 0;

}

int BlockHandler(PBLOCK pblock)
{
	ISOAPDU apdu;
	unsigned char  tblock; 
	unsigned char  inst; 
	unsigned char  pbuff[16];
	unsigned char  LRC;
	unsigned short i;	 
	unsigned int addr;

	tblock = pblock->PCB & 0xC0;
	inst = pblock->PCB & 0x3F;
	
	if(tblock == SBLOCK_HEADER) 
	{
		if(inst == IFS_REQ)
		{  							
			pbuff[0]= pblock->NAD;
			pbuff[1]= SBLOCK_HEADER | IFS_RESP;
			pbuff[2]= pblock->LEN;
			pbuff[3]= pblock->INF[0];

			LRC = 0;
			for(i=0;i<4;i++) {
				LRC ^= pbuff[i];
			}
			pbuff[4]= LRC;
			for(i=0;i<5;i++) {
				iso_tx(pbuff[i]);
			}  
		}	   
		else if(inst == RESYNCH_REQ)
		{
			 							
			pbuff[0]= pblock->NAD;
			pbuff[1]= SBLOCK_HEADER | RESYNCH_RESP;
			pbuff[2]= pblock->LEN;

			LRC = 0;
			for(i=0;i<3;i++) {
				LRC ^= pbuff[i];
			}
			pbuff[3]= LRC;
			for(i=0;i<4;i++) {
				iso_tx(pbuff[i]);
			}  
		}
		else if(inst == WTX_RESP)  
		{			
		}
	}
	else if(tblock == RBLOCK_HEADER)
	{
	}
	else  // I_BLOCK
	{

		apdu.pheader = (APDUHEADER  *) &pblock->INF[0];
		apdu.papdudata = &pblock->INF[5];

		if((apdu.pheader->CLA == 0x90) || (apdu.pheader->CLA == 0xC0))
		{
			firmware_response(&apdu); 	
		}
		else if(apdu.pheader->CLA == 0x00)
		{
			firmware_iso7816_response(&apdu);
		}  
		else if(apdu.pheader->CLA == 0x51)
		{
			if(apdu.pheader->INS == 0x02)
			{	
				//Chip Erase
	   			FLASHX = 0x05;	  //Enable erase mode
	   			flash_wr((unsigned char*)0x5555, 0x10);
	   			FLASHX = 0x00;
			}
			if(apdu.pheader->INS == 0x03)
			{
				if(((UID[6] == 0xFF) || (UID[6] == 0x00)) && (apdu.pheader->LEN<=8))
				{
					flash_write(UID, 0, apdu.papdudata, apdu.pheader->LEN);
				}				
				iso14443sendresp(SW_OK);
			}
			else if(apdu.pheader->INS == 0x04)
			{
				addr = (apdu.pheader->P1<<8)  + apdu.pheader->P2;				
				iso14443send((uint8_t *) addr, apdu.pheader->LEN,(SW_OK&0xFF));		
			} 
			else
			{			
				iso14443sendresp(SW_ERR_INS);
			}
		}
		else
		{			
			iso14443sendresp(SW_ERR_CLA);
		}
	}
	return 0;
}


void System_Dispatcher(void) {
    		
  	init();

  	while(1){// (RecvBlockFrame(&block)==0 && BlockHandler(&block)==0) {	
			//__wfe();
			RecvBlockFrame(&block);
			BlockHandler(&block);
  	}

}
