#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"

void delete_app(ISOAPDU * papdu_command) 
{
	uint16_t resp,data_offset,i;
    uint8_t  buffer_flash[60];

    data_offset = 0;	
	i = 0;

	//search application 
	while(i<MAX_APP_NUM)
	{
		
		if ((app[i].AID[0] == papdu_command->papdudata[0]) &&
			(app[i].AID[1] == papdu_command->papdudata[1]) &&
			(app[i].AID[2] == papdu_command->papdudata[2]))
		{
			i = MAX_APP_NUM+1; //AID found
			app_idx = i;
		}
		else
		{
			i++;
			data_offset += sizeof(application);								
		}

	}

	
	if(i == MAX_APP_NUM+1)
	{
		if((cmd_status.authentication == 2) && (key_idx == 0))
		{ 
			for(i=0; i<3/*sizeof(application)*/; i++) //app id = 3 bytes
			  buffer_flash[i] = 0;
			   
			flash_write(app_buff, data_offset, buffer_flash, 3);//sizeof(application));
	
			resp = (DESFIRE_SW1<<8) | OPERATION_OK;
	        iso14443sendresp(resp);         
	
			cmd_status.sel_app = PICC_LEVEL;
		}
		else
		{
        	resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
        	iso14443sendresp(resp);
		}
	}
	else
	{
        resp = (DESFIRE_SW1<<8) | APPLICATION_NOT_FOUND;
        iso14443sendresp(resp);
		
	}						
}
