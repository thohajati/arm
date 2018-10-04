#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"


void create_app(ISOAPDU * papdu_command)
{
    uint16_t data_offset;
    uint16_t i;
    uint16_t resp;
  	uint8_t idx,z;


	data_offset = 0;

	i = 0;
	


   //check if current application number less than MAX_APP_NUM
//	while(i<MAX_APP_NUM)
//	{	
//		if ((app[i].AID[0] == 0) &&
//			(app[i].AID[1] == 0) &&
//			(app[i].AID[2] == 0))
//		{
//			idx = i;
//			i = MAX_APP_NUM + 1; //current application number less than MAX_APP_NUM
//		}
//		else
//		{
//			i++;
//			data_offset += sizeof(application);
//		}

//	}
		z = 0;
			while(i<MAX_APP_NUM)
		{	
			z = (app[i].AID[0] == 0 & app[i].AID[1] == 0 & app[i].AID[2] == 0);
			
			if(z){
				idx = i;
				i = MAX_APP_NUM + 1; //current application number less than MAX_APP_NUM
			}
			else
			{
				i++;
				data_offset += sizeof(application);								
			}
		}

	if (i == MAX_APP_NUM)
	{
        resp = (DESFIRE_SW1<<8)|COUNT_ERROR;
        iso14443sendresp(resp);
		
	}

	//check if there isn't current application ID same with application ID want to be created
	else if(i == MAX_APP_NUM+1)
	{

		if((papdu_command->papdudata[0] == 0) &&
		   (papdu_command->papdudata[1] == 0) &&
		   (papdu_command->papdudata[2] == 0))
		{
			i = MAX_APP_NUM + 2; //DUPLICATE_ERROR
		}
		else
		{
			i = 0;
//			while(i<MAX_APP_NUM)
//			{
//				
//				if ((app[i].AID[0] == papdu_command->papdudata[0]) &&
//					(app[i].AID[1] == papdu_command->papdudata[1]) &&
//					(app[i].AID[2] == papdu_command->papdudata[2]))
//				{
//					i = MAX_APP_NUM + 2; //DUPLICATE_ERROR
//				}
//				else
//				{
//					i++;
//				}

//			}
			z = 0;
			while(i<MAX_APP_NUM)
		{	
			z = (app[i].AID[0] == papdu_command->papdudata[0]) & 
			    (app[i].AID[1] == papdu_command->papdudata[1]) &	
			    (app[i].AID[2] == papdu_command->papdudata[2]);
			
			if(z){
				i = MAX_APP_NUM + 2; //DUPLICATE_ERROR
			}
			else
			{
				i++;					
			}
		}

		}

		if (i == MAX_APP_NUM+2)
		{
            resp = (DESFIRE_SW1<<8)|DUPLICATE_ERROR;
            iso14443sendresp(resp); 
			
		}
		else
		{			
			flash_write(app_buff, data_offset, papdu_command->papdudata, papdu_command->pheader->LEN);	 
			app_idx = idx;
			cmd_status.sel_app = APPLICATION_LEVEL; //aplication selected with AID != 0x00

            resp = (DESFIRE_SW1<<8) | OPERATION_OK;
			iso14443sendresp(resp);
            
		}

	}
}
