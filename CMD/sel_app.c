#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"

void sel_app(ISOAPDU * papdu_command) 
{
	uint16_t i;
    uint16_t resp;
    i = 0;

	if ((papdu_command->papdudata[0] == 0x00) &&
		(papdu_command->papdudata[1] == 0x00) &&
		(papdu_command->papdudata[2] == 0x00))
	{
	   i = MAX_APP_NUM+1;
	   cmd_status.sel_app = PICC_LEVEL; //master aplication selected with AID = 0x00
	}
	else
	{ 
		while(i<MAX_APP_NUM)
		{
			
			if ((app[i].AID[0] == papdu_command->papdudata[0]) &&
				(app[i].AID[1] == papdu_command->papdudata[1]) &&
				(app[i].AID[2] == papdu_command->papdudata[2]))
			{
				app_idx = i;
				cmd_status.sel_app = APPLICATION_LEVEL; //aplication selected with AID != 0x00
				i = MAX_APP_NUM+1;
			}
			else
			{
				i++;
			}

		}
	}

	if (i == MAX_APP_NUM+1)
	{
    	resp = (DESFIRE_SW1<<8) | OPERATION_OK;
        iso14443sendresp(resp);

		cmd_status.authentication = 0;

	}
	else
	{
        resp = (DESFIRE_SW1<<8) | APPLICATION_NOT_FOUND;
        iso14443sendresp(resp);
		
	}   

}
