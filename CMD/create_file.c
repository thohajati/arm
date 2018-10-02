#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"		
#include "instruction.h"
#include "cmd.h"

void create_file(ISOAPDU * papdu_command)
{
    uint32_t i,j,k;
    uint32_t resp,data_offset;
    uint8_t  buffer_flash[60];
    backup_file  *app_backup_file;
    value_file  *app_value_file;
	record_file  *app_record_file;

    static uint8_t file_idx = 0;
    static uint16_t file_addr;
    static uint16_t file_mirror_addr;	


    if((papdu_command->pheader->INS != CREATE_STD_DATA_FILE) &&
	   (papdu_command->papdudata[0] > 0x7))
		{
            resp = (DESFIRE_SW1<<8)|COUNT_ERROR;
            iso14443sendresp(resp);			
		}
		else if(cmd_status.sel_app == APPLICATION_LEVEL)
		{					   
			i = 0;

			// check if there is file with same file_ID
			if(papdu_command->papdudata[0] == 0)
			{
				while(i<MAX_FILE_NUM) 
				{
					if(app[app_idx].app_file[i].file_ID == papdu_command->papdudata[0])
					{
						data_offset = (unsigned int) &app[app_idx].app_file[i];
						j = (uint32_t)app_buff;
						data_offset	-= j;
						
						k = 0;
						for(j=0; j<sizeof(file); j++)
						{
							if(app_buff[data_offset+j] == 0)
							  k++;
						}
						
						if(k != sizeof(file))
						{
							i = MAX_FILE_NUM+1;
						}
					} 
					else
					  i++;
					 
				}
			}
			else
			{
				while(i<MAX_FILE_NUM) 
				{
					if(app[app_idx].app_file[i].file_ID == papdu_command->papdudata[0])
					{
						i = MAX_FILE_NUM+1;
					} 
					else
					  i++;
					 
				}
			}

			// check if there is file with same ISO_ID
			if((papdu_command->pheader->INS == CREATE_STD_DATA_FILE) &&
			   (i == MAX_FILE_NUM))
			{
				i = 0;
				while(i<MAX_FILE_NUM) 
				{
					if((app[app_idx].app_file[i].ISO_file_ID[0] == papdu_command->papdudata[1]) &&
					   (app[app_idx].app_file[i].ISO_file_ID[1] == papdu_command->papdudata[2]))
					{
						i = MAX_FILE_NUM+1;
					} 
					else
					  i++;
					 
				}
			}
		
			if(i == MAX_FILE_NUM)
			{   

				i = 0;
				while(i<MAX_FILE_NUM) 
				{

					data_offset = (unsigned int) &app[app_idx].app_file[i];
					j = (uint32_t)app_buff;
					data_offset	-= j;
					
					k = 0;
					for(j=0; j<sizeof(file); j++)
					{
						if(app_buff[data_offset+j] == 0)
						  k++;
					}
					
					if(k == sizeof(file))
					{
						file_idx = i;
						i = MAX_FILE_NUM+1;
					}
					else
					  i++;
					 
				}

				//calculate file address
				if(file_idx == 0)
				{
					if(app_idx == 0)
						file_addr = 0;	
					else
					{
						i = 0;
						j = 0;
						//search active file from previous application
						while(i<MAX_FILE_NUM)
						{
							if(app[app_idx-1].app_file[i].file_ID != 0)
							  j++; //number active file
							
							i++;
					
						}

						j--;

						file_addr = app[app_idx-1].app_file[j].file_addr;
						i = ((app[app_idx-1].app_file[j].file_lenght[1])<<8) | 
							 (app[app_idx-1].app_file[j].file_lenght[0]);	
						file_addr += i;
					}
				}
				else
				{
					switch (app[app_idx].app_file[file_idx-1].file_type)
					{
						case STD_DATA_FILE:
						{
							file_addr = app[app_idx].app_file[file_idx-1].file_addr;
							j = ((app[app_idx].app_file[file_idx-1].file_lenght[1])<<8) | 
								 (app[app_idx].app_file[file_idx-1].file_lenght[0]);	
							file_addr += j;
							break;
						}
						case BACKUP_DATA_FILE:
						{
							app_backup_file = (backup_file*) &app[app_idx].app_file[file_idx-1];
							
							if(app_backup_file->file_mirror_addr > app_backup_file->file_addr)
							  file_addr = app_backup_file->file_mirror_addr;
							else
							  file_addr = app_backup_file->file_addr;

							j = ((app_backup_file->file_lenght[1])<<8) | 
								 (app_backup_file->file_lenght[0]);	
							file_addr += j;
							file_mirror_addr = file_addr + j;
							break;
						}
						case VALUE_FILE:
						{
							app_value_file = (value_file*) &app[app_idx].app_file[file_idx-1];
							file_addr = app_value_file->file_addr;
							break;
						} 
						case LINEAR_RECORD_FILE:
						case CYCLIC_RECORD_FILE:
						{
							app_record_file = (record_file*) &app[app_idx].app_file[file_idx-1];

							if(app_backup_file->file_mirror_addr > app_backup_file->file_addr)
							  file_addr = app_backup_file->file_mirror_addr;
							else
							  file_addr = app_backup_file->file_addr;

							for(i=0; i<app_record_file->max_record_num[0]; i++)
							{
								j = ((app_record_file->record_size[1])<<8) | 
									 (app_record_file->record_size[0]);	
								file_addr += j;
								file_mirror_addr = file_addr + j;
							}
							break;
						}
						default:
							break;
					}

				}

				switch (papdu_command->pheader->INS)
				{
					case CREATE_STD_DATA_FILE:
					{
						if(papdu_command->pheader->LEN == 7)//without ISO FileID
						{
							buffer_flash[0] = STD_DATA_FILE;
							buffer_flash[1] = papdu_command->papdudata[0];
							buffer_flash[2]	= 0;
							buffer_flash[3]	= 0;
							for(j=1; j<7; j++)
							  buffer_flash[j+3] = papdu_command->papdudata[j];
					
							buffer_flash[j+3] = file_addr>>8;
							buffer_flash[j+4] = file_addr & (0xFF);	
					
							flash_write(app_buff, data_offset, buffer_flash, j+5);
						}
						else
						{
							buffer_flash[0] = STD_DATA_FILE;
							for(j=0; j<papdu_command->pheader->LEN; j++)
							  buffer_flash[j+1] = papdu_command->papdudata[j];
					
							buffer_flash[j+1] = file_addr>>8;
							buffer_flash[j+2] = file_addr & (0xFF);	
					
							flash_write(app_buff, data_offset, buffer_flash, j+3);
						}
						break;
					}
					case CREATE_BACKUP_DATA_FILE:
					{
						buffer_flash[0] = BACKUP_DATA_FILE;	
						for(j=0; j<papdu_command->pheader->LEN; j++)
						  buffer_flash[j+1] = papdu_command->papdudata[j];
				
						buffer_flash[j+1] = file_addr>>8;
						buffer_flash[j+2] = file_addr & (0xFF);
				
						buffer_flash[j+3] = file_mirror_addr>>8;
						buffer_flash[j+4] = file_mirror_addr & (0xFF);	
				
						flash_write(app_buff, data_offset, buffer_flash, j+3);
						break;
					}
					case CREATE_VALUE_FILE:
					{
						buffer_flash[0] = VALUE_FILE;
						for(j=0; j<papdu_command->pheader->LEN; j++)
						  buffer_flash[j+1] = papdu_command->papdudata[j];
				
						buffer_flash[j+1] = file_addr>>8;
						buffer_flash[j+2] = file_addr & (0xFF);	
				
						flash_write(app_buff, data_offset, buffer_flash, j+3);
						break;
					} 
					case CREATE_LINEAR_RECORD_FILE:
					{
						buffer_flash[0] = LINEAR_RECORD_FILE;	
						for(j=0; j<papdu_command->pheader->LEN; j++)
						  buffer_flash[j+1] = papdu_command->papdudata[j];
				
						buffer_flash[j+1] = file_addr>>8;
						buffer_flash[j+2] = file_addr & (0xFF);
				
						buffer_flash[j+3] = file_mirror_addr>>8;
						buffer_flash[j+4] = file_mirror_addr & (0xFF);	
				
						flash_write(app_buff, data_offset, buffer_flash, j+3);
						break;
					}  
					case CREATE_CYCLIC_RECORD_FILE:
					{
						buffer_flash[0] = CYCLIC_RECORD_FILE;
						for(j=0; j<papdu_command->pheader->LEN; j++)
						  buffer_flash[j+1] = papdu_command->papdudata[j];
				
						buffer_flash[j+1] = file_addr>>8;
						buffer_flash[j+2] = file_addr & (0xFF);
				
						buffer_flash[j+3] = file_mirror_addr>>8;
						buffer_flash[j+4] = file_mirror_addr & (0xFF);	
				
						flash_write(app_buff, data_offset, buffer_flash, j+3);
						break;
					}
					default:
						break;
				}				
				 
                resp = (DESFIRE_SW1<<8)|OPERATION_OK;
                iso14443sendresp(resp);
				
		
			}
			else
			{
                resp = (DESFIRE_SW1<<8)|DUPLICATE_ERROR;
                iso14443sendresp(resp);
				
			}
		}
		else
		{
            resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
            iso14443sendresp(resp);
			
		}
					
}
