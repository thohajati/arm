#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"
#include "vuiso14443.h"

void iso_read(ISOAPDU * papdu_command)
{ 
	uint8_t i, data_length;
	uint16_t file_addr, data_offset, data_cntr;
	uint8_t  buffer[64];
						
	data_length = papdu_command->pheader->LEN;
	file_addr = app[app_idx].app_file[file_idx].file_addr;
	data_offset = papdu_command->pheader->P1 <<8 | papdu_command->pheader->P2;	
	data_offset += file_addr;
	
	if(data_length < 0x3C)
	{	
		for(i=0; i<data_length; i++)
		{
		   buffer[i]= file_buff[i+data_offset];
		}

		iso7816send(buffer,data_length,(SW_OK&0xFF));		 	
	}
	
	else
	{
		data_cntr = 50;		
	
		for(i=0; i<data_cntr; i++)
		{
		   buffer[i]= file_buff[i+data_offset];
		}

		iso7816send(buffer,data_length,(SW_OK&0xFF));	
	}
}
