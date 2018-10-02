#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"
 

void get_file_set(ISOAPDU * papdu_command)
{
    uint16_t resp,i;
    uint8_t buffer[64];
    value_file  *app_value_file;
	record_file  *app_record_file;
    
	if(cmd_status.sel_app == PICC_LEVEL)
	{
        resp = (DESFIRE_SW1<<8)|FILE_NOT_FOUND;
        iso14443sendresp(resp);      
		
	}
	else if(cmd_status.sel_app == APPLICATION_LEVEL)
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
		
		if(i == MAX_FILE_NUM+1)
		{
			switch (app[app_idx].app_file[file_idx].file_type)
			{
				case STD_DATA_FILE:
				case BACKUP_DATA_FILE:
				{
					buffer[1] = app[app_idx].app_file[file_idx].file_type;
					buffer[2] = app[app_idx].app_file[file_idx].com_setting;
					buffer[3] = app[app_idx].app_file[file_idx].access_right[0]; 
					buffer[4] = app[app_idx].app_file[file_idx].access_right[1];
					buffer[5] = app[app_idx].app_file[file_idx].file_lenght[0]; 
					buffer[6] = app[app_idx].app_file[file_idx].file_lenght[1];	
					buffer[7] = app[app_idx].app_file[file_idx].file_lenght[2];
                    
                    iso14443send(&buffer[1],7,OPERATION_OK);					
					break;
				}
				case VALUE_FILE:
				{
					app_value_file = (value_file*) &app[app_idx].app_file[file_idx];
					 
					buffer[1] = app_value_file->file_type;
					buffer[2] = app_value_file->com_setting;
					buffer[3] = app_value_file->access_right[0]; 
					buffer[4] = app_value_file->access_right[1];
					buffer[5] = app_value_file->lower_limit[0]; 
					buffer[6] = app_value_file->lower_limit[1];	
					buffer[7] = app_value_file->lower_limit[2];	
					buffer[8] = app_value_file->lower_limit[3];
					buffer[9] = app_value_file->upper_limit[0]; 
					buffer[10] = app_value_file->upper_limit[1];	
					buffer[11] = app_value_file->upper_limit[2];	
					buffer[12] = app_value_file->upper_limit[3]; 
					buffer[13] = app_value_file->limited_credit_value[0]; 
					buffer[14] = app_value_file->limited_credit_value[1];	
					buffer[15] = app_value_file->limited_credit_value[2];	
					buffer[16] = app_value_file->limited_credit_value[3];	
					buffer[17] = app_value_file->limited_credit_enable;

					iso14443send(&buffer[1],17,OPERATION_OK);
                    break;
				}
				case LINEAR_RECORD_FILE:
				case CYCLIC_RECORD_FILE:
				{
					app_record_file = (record_file*) &app[app_idx].app_file[file_idx];
					 
					buffer[1] = app_record_file->file_type;
					buffer[2] = app_record_file->com_setting;
					buffer[3] = app_record_file->access_right[0]; 
					buffer[4] = app_record_file->access_right[1];
					buffer[5] = app_record_file->record_size[0]; 
					buffer[6] = app_record_file->record_size[1];	
					buffer[7] = app_record_file->record_size[2];	
					buffer[8] = app_record_file->max_record_num[0];
					buffer[9] = app_record_file->max_record_num[1]; 
					buffer[10] = app_record_file->max_record_num[2]; 	
					buffer[11] = app_record_file->current_record[0];
					buffer[12] = app_record_file->current_record[1]; 
					buffer[13] = app_record_file->current_record[2];

					iso14443send(&buffer[1],13,OPERATION_OK);
                    break;									
				}
				default:
					break;
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
        resp = (DESFIRE_SW1)|PERMISSION_DENIED;
        iso14443sendresp(resp);
		
	}

}
