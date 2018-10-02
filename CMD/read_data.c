#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"
#include "vuiso14443.h"

const unsigned char max_read_len = 0xFF;

void read_data(ISOAPDU * papdu_command) 
{
    desfire_read_data *read_data;	
    uint16_t resp,i,j,data_offset,data_length,file_addr;
    uint16_t data_cntr = 0;
    uint8_t  buffer_flash[256];//[60];
    uint8_t  buffer[256];
    uint8_t  tdes_init[8];
	
		//if((VDET & 0x40) != 0x40) //mode contactless
//			papdu_command->pheader->INS = cmd_status.prev_command;
    
	if(cmd_status.read_data == 0)
	{					 
		if(cmd_status.sel_app == APPLICATION_LEVEL)
		{  						 
			read_data = (desfire_read_data *) papdu_command->papdudata; 
										
			i=0;
			while(i<MAX_FILE_NUM)
			{
				if((app[app_idx].app_file[i].file_ID == read_data->file_ID) &&
				   ((app[app_idx].app_file[i].file_type == STD_DATA_FILE) || 
					(app[app_idx].app_file[i].file_type == BACKUP_DATA_FILE)))
				{
					file_idx = i;
					i = MAX_FILE_NUM+1;
				}
				else
				  i++;
		
			}

			if(i == MAX_FILE_NUM+1) 
			{
				if (((app[app_idx].app_file[file_idx].access_right[1] & 0xF0) == 0xE0) ||
					((app[app_idx].app_file[file_idx].access_right[0] & 0xF0) == 0xE0))
				{ 
					data_length = read_data->length[0] | (read_data->length[1]<<8);

					if (data_length == 0)
					{
						read_all_flag = 1;
						data_length = (app[app_idx].app_file[file_idx].file_lenght[0]) |
									  (app[app_idx].app_file[file_idx].file_lenght[1]<<8);
					}

					file_addr = app[app_idx].app_file[file_idx].file_addr;
					data_offset = (read_data->offset[1]<<8) | read_data->offset[0];
					data_offset += file_addr;

					if(data_length>max_read_len)
					{
                      
                        iso14443send(&file_buff[data_offset],max_read_len,ADDITIONAL_FRAME);						

						cmd_status.read_data = 1;

						data_cntr = max_read_len;

					}
					else
					{										
                      
                        iso14443send(&file_buff[data_offset],data_length,OPERATION_OK);					 
						
					}
				}
				else if (((app[app_idx].app_file[file_idx].access_right[1] & 0xF0) == 0xF0) &&
						 ((app[app_idx].app_file[file_idx].access_right[0] & 0xF0) == 0xF0))
				{ 
					 
					resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
                    iso14443sendresp(resp);
                    
				}
				else if((cmd_status.authentication == 2) &&
						(((key_idx<<4) == (app[app_idx].app_file[file_idx].access_right[1] & 0xF0)) ||
						 ((key_idx<<4) == (app[app_idx].app_file[file_idx].access_right[0] & 0xF0))))
				{

					data_length = read_data->length[0] | (read_data->length[1]<<8);

					if (data_length == 0)
					{
						data_length = (app[app_idx].app_file[file_idx].file_lenght[0]) |
									  (app[app_idx].app_file[file_idx].file_lenght[1]<<8);
					}

					file_addr = app[app_idx].app_file[file_idx].file_addr;
					data_offset = (read_data->offset[1]<<8) | read_data->offset[0];
					data_offset += file_addr;
					
					if (data_length>max_read_len)
					{
						
						xstsm212_tdes16_encrypt_tx(&file_buff[data_offset], sesion_key, &buffer[1], 48);
                        
                        iso14443send(&buffer[1],48,ADDITIONAL_FRAME);	                    
						
                        
						for(i=0; i<8; i++)
							tdes_init[i] = buffer[41+i];											    
						
						cmd_status.read_data = 2;                                                	
						data_cntr = 48;												

						
					}
					else
					{
						/* get data from file */
						for(i=0; i<data_length; i++){
						  buffer_flash[i] = file_buff[data_offset + i];                         
                        }
																	  
						if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA)) // AES
						{													
							iso_14443_aes_read_data(papdu_command,buffer_flash,data_length);			
						
											
						}else{ // DES
						
							vucalc_crc(buffer_flash, &buffer_flash[i+1], &buffer_flash[i], i);
	
							data_length += 2;
	                    	
							if((data_length & 0x7) != 0)
							{
							  i	= data_length;
							  data_length = (i & 0xFFF8) + 8;
							}
						
							for(; i<data_length; i++)
							  buffer_flash[i] = 0x00;
	
							xstsm212_tdes16_encrypt_tx(buffer_flash, sesion_key, &buffer[1], i);
                            
                            iso14443send(&buffer[1],i,OPERATION_OK);	                    	
							
						}
					}

				}
				else
				{
                    resp = (DESFIRE_SW1<<8)|AUTHENTICATION_ERROR;
                    iso14443sendresp(resp);
				}
			}

		}
		else
		{
            resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
            iso14443sendresp(resp);
                    
		}

	}


	else if(cmd_status.read_data == 1)
	{
		data_offset += 50;

		if((data_cntr+50)<data_length)
		{
          
			iso14443send(&file_buff[data_offset],50,ADDITIONAL_FRAME); 
			
			data_cntr += 50;

			cmd_status.read_data = 1;

		}
		else
		{
			data_length -= data_cntr;
			 
			iso14443send(&file_buff[data_offset],data_length,OPERATION_OK);
            			
			data_cntr = 0;

			cmd_status.read_data = 0;

		}
	}
	else if(cmd_status.read_data == 2)
	{
		
		j = data_offset + data_cntr;

		if((data_cntr+54)<data_length)
		{
			 
			tdes16_encrypt_tx(tdes_init, &file_buff[j], sesion_key, &buffer[1], 48);
            
            iso14443send(&buffer[1],48,ADDITIONAL_FRAME);
            
			          
			for(i=0; i<8; i++)
			  tdes_init[i] = buffer[41+i];		
		  

			cmd_status.read_data = 2;

			data_cntr += 48;
		}
		else
		{
			data_length -= data_cntr;

			for(i=0; i<data_length; i++)
			  buffer_flash[i] = file_buff[j + i];

			vucalc_crc(&file_buff[data_offset], &buffer_flash[i+1], &buffer_flash[i], data_length+data_cntr);
					
			data_length += 2;

			if((data_length & 0x7) != 0)
			{
				i = data_length;
				data_length = (i & 0xFFF8) + 8;

				if(read_all_flag)
				  buffer_flash[i++] = 0x80;

				for(; i<data_length; i++)
				  buffer_flash[i] = 0x00;

			}								
			

			tdes16_encrypt_tx(tdes_init, buffer_flash, sesion_key, &buffer[1], i);
            
            iso14443send(&buffer[1],i,OPERATION_OK);
            
			cmd_status.read_data = 0;

			data_cntr = 0;
		}
	}

}

//void read_data(ISOAPDU * papdu_command) 
//{
//	
//		if((VDET & 0x40) != 0x40){ //mode contactless
//			cmd_status.prev_command = papdu_command->pheader->INS;			
//			iso14443waitreq();
//						
//		}else{
//			read_process(papdu_command);
//		}      
//	
//}
