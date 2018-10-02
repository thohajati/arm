#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void get_key_ver(ISOAPDU * papdu_command)
{
	uint8_t key_ver;
	uint16_t resp, key_offset;

	if(cmd_status.sel_app == PICC_LEVEL)
	{		
		iso14443send(&master_key[17],1,OPERATION_OK);
	}
	else if(cmd_status.sel_app == APPLICATION_LEVEL)
	{  		
		key_offset = (app_idx+1)<<8;
		key_offset -= 16;
		key_offset += papdu_command->papdudata[0];	  
		
		key_ver = key_buff[key_offset]; 
		
		iso14443send(&key_ver,1,OPERATION_OK);
	}
	else
	{	 
		resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
        iso14443sendresp(resp);
	}
}
