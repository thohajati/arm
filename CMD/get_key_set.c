#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void get_key_set(void)
{
	uint8_t  buffer[2];
	uint16_t resp;

	if(cmd_status.sel_app == PICC_LEVEL)
	{
		buffer[0] = master_key_setting[0];
		buffer[1] = 0x01; 
		
		iso14443send(buffer,2,OPERATION_OK);
	}
	else if(cmd_status.sel_app == APPLICATION_LEVEL)
	{ 
		buffer[0] = app[app_idx].key_setting[0];
		buffer[1] = app[app_idx].key_setting[1] & 0xF; 
		
		iso14443send(buffer,2,OPERATION_OK);
	}
	else
	{	 
		resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
        iso14443sendresp(resp);
	}
}
