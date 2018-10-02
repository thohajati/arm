#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void iso_select(ISOAPDU * papdu_command)
{	
	uint8_t i;
						
	if ((papdu_command->pheader->P1 == 0x04) && (papdu_command->pheader->LEN == 0x07))
	{ 
		i=0;
		while(i<MAX_APP_NUM)
		{
			
			if ((app[i].tag_ID[0] == papdu_command->papdudata[0]) && (app[i].tag_ID[1] == papdu_command->papdudata[1]) &&
				(app[i].tag_ID[2] == papdu_command->papdudata[2]) && (app[i].tag_ID[3] == papdu_command->papdudata[3]) &&
				(app[i].tag_ID[4] == papdu_command->papdudata[4]) && (app[i].tag_ID[5] == papdu_command->papdudata[5]) &&
				(app[i].tag_ID[6] == papdu_command->papdudata[6]))
			{
				app_idx = i;
				i = MAX_APP_NUM+1;
			}
			else
			{
				i++;
			}

		}
		
		if (i == MAX_APP_NUM+1)
		{
			iso14443sendresp(SW_OK); 
		}
		else
		{
			iso14443sendresp(SW_NOT_FOUND); 
		} 

	} 
	else if((papdu_command->pheader->P1 == 0x00) && (papdu_command->pheader->LEN == 0x02))
	{
		i=0;
		while(i<MAX_FILE_NUM)
		{
	
		   if((app[app_idx].app_file[i].ISO_file_ID[0] == papdu_command->papdudata[1]) && 
			  (app[app_idx].app_file[i].ISO_file_ID[1] == papdu_command->papdudata[0]))
		   {
			   file_idx = i;
			   i = MAX_FILE_NUM+1;
		   }
		   else
		   {
			   i++;
		   }
		}

		if(i == MAX_FILE_NUM+1)
		{
			iso14443sendresp(SW_OK); 
		}
		else
		{
			iso14443sendresp(SW_NOT_FOUND);
		} 						  
	}
	else
	{
		iso14443sendresp(SW_NOT_FOUND);
	}							
}
