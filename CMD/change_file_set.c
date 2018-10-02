#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void change_file_set(ISOAPDU * papdu_command) 
{
	uint16_t resp,i,j,data_offset;
    uint8_t  buffer_flash[60];
    
    if(cmd_status.sel_app == APPLICATION_LEVEL)
	{
		i=0;
		while(i<MAX_FILE_NUM)
		{
	
			if(app[app_idx].app_file[i].file_ID == papdu_command->papdudata[0])
			{
				file_idx = i;
				i = MAX_FILE_NUM+1;
			}
			else
			  i++;
	
		}
		if (i == MAX_FILE_NUM+1)
		{
			if ((app[app_idx].app_file[file_idx].access_right[0] & 0xF) == 0xE)
			{
				data_offset = (unsigned int) &app[app_idx].app_file[file_idx].com_setting;
				j = (uint32_t)app_buff;
				data_offset	-= j;							
		
				flash_write(app_buff, data_offset, &papdu_command->papdudata[1], 3);

				resp = (DESFIRE_SW1<<8)|OPERATION_OK;
                iso14443sendresp(resp);
                
			}
			else if ((app[app_idx].app_file[file_idx].access_right[0] & 0xF) == 0xF)
			{
				resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
                iso14443sendresp(resp);
                
			}
			else if((cmd_status.authentication == 2) &&
					(key_idx == (app[app_idx].app_file[file_idx].access_right[0] & 0xF))) 
			{
				xstsm212_tdes16_encrypt_rx(&papdu_command->papdudata[1], sesion_key, buffer_flash, 8);

				data_offset = (unsigned int) &app[app_idx].app_file[file_idx].com_setting;
				j = (uint32_t)app_buff;
				data_offset	-= j;							
		
				flash_write(app_buff, data_offset, buffer_flash, 3);

				resp = (DESFIRE_SW1<<8)|OPERATION_OK;
                iso14443sendresp(resp);
                
			}
			else
			{
				resp = (DESFIRE_SW1<<8)|AUTHENTICATION_ERROR;
                iso14443sendresp(resp);
                
			}

		}
		else
		{
			resp = (DESFIRE_SW1<<8)|FILE_NOT_FOUND;
            iso14443sendresp(resp);
            
		}
	}
	else
	{
		resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
        iso14443sendresp(resp);
        
	}
	
}
