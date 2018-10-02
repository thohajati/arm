#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"
 
void get_file_id(void) 
{
    uint16_t i,j,resp;
    uint8_t buffer[MAX_APP_NUM*MAX_FILE_NUM];
    
	if(cmd_status.sel_app == PICC_LEVEL)
	{
        resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
        iso14443sendresp(resp);        
		
	}
	else if(cmd_status.sel_app == APPLICATION_LEVEL)
	{
		i = 0;
		j = 0;

		//search active file
		while(i<MAX_FILE_NUM)
		{
			if(app[app_idx].app_file[i].file_ID != 0){
				buffer[j] = app[app_idx].app_file[i].file_ID;
			  j++; //number active file
			}			
			i++;	
		}	
		  
    iso14443send(buffer,j,OPERATION_OK);		
		
	}
	else
	{
        resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
        iso14443sendresp(resp);
		
	}

}
