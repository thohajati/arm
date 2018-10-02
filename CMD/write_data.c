#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void write_data(ISOAPDU * papdu_command){
	
desfire_write_data  *write_data;	
uint16_t resp,i,j,data_offset,data_length,file_addr;
uint16_t data_cntr = 0;
uint8_t buffer_flash[256];//[60];
uint8_t  tdes_init[8];

//if((VDET & 0x40) != 0x40) //mode contactless
//	papdu_command->pheader->INS = cmd_status.prev_command;
	
if(cmd_status.write_data == 0)
	{
		if(cmd_status.sel_app == APPLICATION_LEVEL)
		{
			write_data = (desfire_write_data *) papdu_command->papdudata;

			i=0;
			while(i<MAX_FILE_NUM)
			{
				if((app[app_idx].app_file[i].file_ID == write_data->file_ID) &&
				   ((app[app_idx].app_file[i].file_type == STD_DATA_FILE) || 
					(app[app_idx].app_file[i].file_type == BACKUP_DATA_FILE)))
				{
				 file_idx = i;
				 i = MAX_FILE_NUM+1;
			   }
			   else
				 i++;
		
			}   
		
			if (i == MAX_FILE_NUM+1)
			{
				data_offset = (write_data->offset[1]<<8) | write_data->offset[0];
				data_length = (write_data->length[1]<<8) | write_data->length[0];;

				j = (app[app_idx].app_file[file_idx].file_lenght[1]<<8) | 
					 app[app_idx].app_file[file_idx].file_lenght[0];

				i = data_offset + data_length;
												
				if((write_data->length[2] != 0) ||
				   (write_data->offset[2] != 0) || 
				   (i > j))
				{
					resp = (DESFIRE_SW1<<8)|LENGTH_ERROR;
                    iso14443sendresp(resp);
                }							    
				else if (((app[app_idx].app_file[file_idx].access_right[1] & 0xF) == 0xE) ||
						 ((app[app_idx].app_file[file_idx].access_right[0] & 0xF0) == 0xE0) ||
						 ((app[app_idx].app_file[file_idx].com_setting & 0x03) == 0x00))
				{
					if((data_length + 7) > papdu_command->pheader->LEN)
					{
						file_addr = app[app_idx].app_file[file_idx].file_addr;
						
						data_offset += file_addr;

						data_cntr = papdu_command->pheader->LEN - 7;
						
						flash_write(file_buff, data_offset, write_data->data_to_write, data_cntr);
						
						data_offset += data_cntr;
							
						resp = (DESFIRE_SW1<<8)|ADDITIONAL_FRAME;
                        iso14443sendresp(resp);
                        cmd_status.write_data = 1;
					}
					else if	(((data_length + 7) == papdu_command->pheader->LEN)&&(data_length <= 0xE8))
					{
						file_addr = app[app_idx].app_file[file_idx].file_addr;
						
						data_offset += file_addr;

						flash_write(file_buff, data_offset, write_data->data_to_write, data_length);
		
						resp = (DESFIRE_SW1<<8)|OPERATION_OK;
                        iso14443sendresp(resp);
                    }
					else
					{
                        resp = (DESFIRE_SW1<<8)|LENGTH_ERROR;
                        iso14443sendresp(resp);
						
					}

				}
				else if (((app[app_idx].app_file[file_idx].access_right[1] & 0xF) == 0xF) &&
						 ((app[app_idx].app_file[file_idx].access_right[0] & 0xF0) == 0xF0))
				{ 
					 
					resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
                    iso14443sendresp(resp);
                    
				}
				else if((cmd_status.authentication == 2) &&
						((key_idx == (app[app_idx].app_file[file_idx].access_right[1] & 0xF)) ||
						 ((key_idx<<4) == (app[app_idx].app_file[file_idx].access_right[0] & 0xF0))))
				{
					real_data_length = data_length;

					if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA) ){ // AES
						
						data_length += 4;

						if((data_length % 16) != 0){
						  data_length += (16 -(data_length % 16));
						}
					
						
					}else{ //DES
					
						data_length += 2;

						if((data_length & 0x7) != 0)
						{
						  data_length = (data_length & 0xF8) + 8;
						}
					}
					
	
					if((data_length + 7) > papdu_command->pheader->LEN)
					{
						file_addr = app[app_idx].app_file[file_idx].file_addr;
						
						data_offset += file_addr;

						data_cntr = papdu_command->pheader->LEN - 7; 												
						
						xstsm212_tdes16_encrypt_rx(write_data->data_to_write, sesion_key, buffer_flash, data_cntr);
						
						j = data_cntr - 8;
                        
						for(i=0; i<8; i++)
						  tdes_init[i] = write_data->data_to_write[j+i];
												
						flash_write(file_buff, data_offset, buffer_flash, data_cntr);
		                
						data_offset += data_cntr;
						
						resp = (DESFIRE_SW1<<8)|ADDITIONAL_FRAME;
                        iso14443sendresp(resp);
                                                                   	
						cmd_status.write_data = 2; 																			

					}
					else if	(((data_length + 7) == papdu_command->pheader->LEN)&&(data_length <= 0xE8))
					{
						file_addr = app[app_idx].app_file[file_idx].file_addr;
						
						data_offset += file_addr;
						
						if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA) ) // AES
						{											
							
							iso_14443_aes_write_data(papdu_command);//,write_data,real_data_length);
							
						} else { //DES
	
							xstsm212_tdes16_encrypt_rx(write_data->data_to_write, sesion_key, buffer_flash, data_length);
	                    	
							flash_write(file_buff, data_offset, buffer_flash, write_data->length[0]);
					    	
							resp = (DESFIRE_SW1<<8)|OPERATION_OK;
                            iso14443sendresp(resp);
                            
						}
						
						

					}
					else
					{
                        resp = (DESFIRE_SW1<<8)|LENGTH_ERROR;
                        iso14443sendresp(resp);
                        
					}
					
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
	else if(cmd_status.write_data > 0)
	{
		if((data_cntr + papdu_command->pheader->LEN) < data_length)
		{
			if(cmd_status.write_data == 2)
			{
				
				tdes16_encrypt_rx(tdes_init, papdu_command->papdudata, sesion_key, buffer_flash, papdu_command->pheader->LEN);
		        
				flash_write(file_buff, data_offset, buffer_flash, papdu_command->pheader->LEN);
				
				j = papdu_command->pheader->LEN - 8;
                
				for(i=0; i<8; i++)
				  tdes_init[i] = papdu_command->papdudata[j+i];
				  
				

			}
			else
			  flash_write(file_buff, data_offset, papdu_command->papdudata, papdu_command->pheader->LEN);
			
			data_cntr += papdu_command->pheader->LEN;
			data_offset += papdu_command->pheader->LEN;
					
		    resp = (DESFIRE_SW1<<8)|ADDITIONAL_FRAME;
            iso14443sendresp(resp);
            
		}
		else if((data_cntr + papdu_command->pheader->LEN) == data_length)
		{

			if(cmd_status.write_data == 2)
			{
				
				tdes16_encrypt_rx(tdes_init, papdu_command->papdudata, sesion_key, buffer_flash, papdu_command->pheader->LEN);
		        
				i = (unsigned short)papdu_command->pheader->LEN; 
				j = data_length - real_data_length;
				i -= j;
                
				flash_write(file_buff, data_offset, buffer_flash, i);
            	
			}
			else
			  flash_write(file_buff, data_offset, papdu_command->papdudata, papdu_command->pheader->LEN);
					
			  resp = (DESFIRE_SW1<<8)|OPERATION_OK;
              iso14443sendresp(resp);
            

			  cmd_status.write_data = 0;

		}
		else
		{
			resp = (DESFIRE_SW1<<8)|LENGTH_ERROR;
            iso14443sendresp(resp);          
			
		}
	}
}

//void write_data(ISOAPDU * papdu_command) 
//{
//	
//		if((VDET & 0x40) != 0x40){ //mode contactless
//			cmd_status.prev_command = papdu_command->pheader->INS;			
//			iso14443waitreq();
//						
//		}else{
//			write_process(papdu_command);
//		}      
//	
//}
