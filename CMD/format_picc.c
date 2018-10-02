#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"
#include "vuiso14443.h"

void format_picc(ISOAPDU * papdu_command) 
{
    uint16_t resp;
		unsigned char masterkeyval = 0x0F;

	if(cmd_status.authentication == 2)
	{


		if((VDET & 0x40) == 0x40){ //mode contact
			//flash_write((unsigned char*)0x8e00,0x120,&masterkeyval,1);
			flash_format(app_buff, SIZE_APP_BUFF);
			flash_format(file_buff, SIZE_FILE_BUFF);
			flash_format(key_buff, SIZE_KEY_BUFF); 
			flash_format(master_key, SIZE_MASTER_KEY_BUFF);
			flash_byte_set(master_key_setting, 0x0F);
			
	
				resp = (DESFIRE_SW1<<8) | OPERATION_OK;
					iso14443sendresp(resp);
		}
		else{
				cmd_status.prev_command = papdu_command->pheader->INS;
				iso14443waitreq();				
		}
	}
	else
	{
        resp = (DESFIRE_SW1<<8) | AUTHENTICATION_ERROR;
        iso14443sendresp(resp);
	
	}

}
