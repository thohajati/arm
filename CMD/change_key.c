#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"
#include "vuaes.h"

void iso_14443_change_key_des_unselect_app(ISOAPDU * papdu_command)
{
    uint8_t  buffer[64];
	uint16_t resp;

	xstsm212_tdes16_encrypt_rx(&papdu_command->papdudata[1], sesion_key, buffer, 24);
											
	flash_write(master_key, 0, buffer, 16);					
		 
	resp = (DESFIRE_SW1<<8)|OPERATION_OK;
    iso14443sendresp(resp);        
	
	//status.authentication = 0;
}

void iso_14443_change_key_des_auth(ISOAPDU * papdu_command)
{
	uint16_t key_offset, resp;
    uint8_t  buffer[64];
    uint8_t  buffer_flash[32];	

	xstsm212_tdes16_encrypt_rx(&papdu_command->papdudata[1], sesion_key, buffer, 24);
				                            	
	if(xstsm212_verify_crc(buffer, 18) == 0)
	{
	
		key_offset = 0;
		key_offset += app_idx<<8;
		key_offset += (papdu_command->papdudata[0])<<4;
		
		flash_write(key_buff, key_offset, buffer, 16);
		
		key_offset = (app_idx+1)<<8;
		key_offset -= 16;
		key_offset += papdu_command->papdudata[0];	  
		
		buffer_flash[0] = key_buff[key_offset] + 1;  
		flash_write(key_buff, key_offset, buffer_flash, 1);   					
		 
		resp = (DESFIRE_SW1<<8)|OPERATION_OK;
        iso14443sendresp(resp);        
	
		//status.authentication = 0;
    
	}
	else
	{
        resp = (DESFIRE_SW1<<8)|INTEGRITY_ERROR;
        iso14443sendresp(resp);
		    
		cmd_status.authentication = 0;
	}	
}

void iso_14443_change_key_des_random(ISOAPDU * papdu_command)
{
	uint16_t key_offset,i,resp;
    uint8_t  buffer_flash[32];
    uint8_t  buffer[64];
	uint8_t key_ver;
    
    xstsm212_tdes16_encrypt_rx(&papdu_command->papdudata[1], sesion_key, buffer, 24);
                                            	
	key_offset = 0;
	key_offset += app_idx<<8;
	key_offset += (papdu_command->papdudata[0])<<4;
    
	for(i=0; i<16; i++)
	  buffer_flash[i] = key_buff[key_offset+i] ^ buffer[i];
    
    if((master_key_setting[0] & 0xf0) == 0)
	{
		if(papdu_command->papdudata[0] == 0) // change key 0 (key authentikasi)
		{
			buffer_flash[16] = buffer[16]; 
			buffer_flash[17] = buffer[17];
		}
		else 
		{
			buffer_flash[16] = buffer[18]; 
			buffer_flash[17] = buffer[19];
		}
	}
    
	if((xstsm212_verify_crc(buffer, 18) == 0) &&
	   (xstsm212_verify_crc(buffer_flash, 18) == 0))
	{											  										
		flash_write(key_buff, key_offset, buffer_flash, 16);
		
		key_offset = (app_idx+1)<<8;
		key_offset -= 16;
		key_offset += papdu_command->papdudata[0];	  
		
		key_ver = key_buff[key_offset] + 1;  
		flash_write(key_buff, key_offset, &key_ver, 1);   			
		 
		resp = (DESFIRE_SW1<<8)|OPERATION_OK;
        iso14443sendresp(resp); 
	
		//status.authentication = 0;
    
	}
	else
	{
        resp = (DESFIRE_SW1<<8)|INTEGRITY_ERROR;
        iso14443sendresp(resp);
		    
		cmd_status.authentication = 0;
	}
}

//void iso_14443_change_key_aes_random(ISOAPDU * papdu_command)
//{
//	aes_t  aes;
//    uint8_t  buffer[64];
//    uint8_t  buffer_flash[32];
//    uint16_t i,resp;
//    uint16_t key_offset;
//    
//    /* AES decrypt */
//	aes.presult = &buffer[2];
//	aes.pbuffer = &papdu_command->papdudata[1];
//	aes.bufferlen = 32;
//	aes.piv = init_vector;
//	aes.mode = AES_MODE_DECRYPT;
//	
//	vuaes_insertkey16byte(sesion_key);
//	vuaes_process(&aes);
//	
//	/* verify crc32 */
//	buffer[0] = papdu_command->pheader->INS;
//	buffer[1] = papdu_command->papdudata[0];
//	if(!verify_veriu_crc32(buffer, 34))
//	{
//		resp = (DESFIRE_SW1<<8)|INTEGRITY_ERROR;
//        if(cl_rst){
//					iso14443sendresp(resp);
//				}else{
//					iso7816sendresp(resp);
//				}
//        
//	}else
//	{
//		key_offset = 0;
//		key_offset += app_idx<<8;
//		key_offset += (papdu_command->papdudata[0])<<4;
//    	
//		for(i=0; i<16; i++)
//		  buffer_flash[i] = key_buff[key_offset+i] ^ buffer[i+2];
//    	
//		buffer_flash[16] = buffer[18]; 
//		buffer_flash[17] = buffer[19];
//	
//		flash_write(key_buff, key_offset, buffer_flash, 16);				
//		 
//		resp = (DESFIRE_SW1<<8)|OPERATION_OK;
//        if(cl_rst){
//					iso14443sendresp(resp);
//				}else{
//					iso7816sendresp(resp);
//				}
//        
//		//status.authentication = 0;
//    	
//	}
//}

void change_key(ISOAPDU * papdu_command) 
{
    uint16_t resp;
    
	if(((cmd_status.sel_app == PICC_LEVEL) && (papdu_command->papdudata[0] == 0x00)) || 
	   ((cmd_status.sel_app == APPLICATION_LEVEL) && (papdu_command->papdudata[0] < app[app_idx].key_setting[1])))
	{
		if(cmd_status.sel_app == PICC_LEVEL)
		{
			if((master_key_setting[0] & 0x01) == 0x01) 
			{
				if(cmd_status.authentication == 2) // reader tidak select app -> yg diubah adalah master key
				{
					if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA)) // AES
					{							
						//iso_14443_change_key_aes_unselect_app();											
						
					}else{ //DES								
						iso_14443_change_key_des_unselect_app(papdu_command);
					
					}
															
				}
				else
				{
                    resp = (DESFIRE_SW1<<8) | AUTHENTICATION_ERROR;
                    iso14443sendresp(resp);                    
					
				}	
			}
			else
			{
                resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
                iso14443sendresp(resp);	 				
			}	
		}
		else if(cmd_status.sel_app == APPLICATION_LEVEL)
		{
			if((((app[app_idx].key_setting[0] & 0xF0) != 0xF0) && (papdu_command->papdudata[0] != 0x00)) ||
			   (((app[app_idx].key_setting[0] & 0x01) == 0x01) && (papdu_command->papdudata[0] == 0x00)))
			{
				if((cmd_status.authentication == 2) && 
				   (key_idx == papdu_command->papdudata[0]) && 
				   ((app[app_idx].key_setting[0] & 0xF0) == 0xE0)) // key number yg diubah 	harus sama dengan key number auth
				{
					if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA)) // AES
					{								
						//iso_14443_change_key_aes_auth();											
						
					}else{ //DES							
						iso_14443_change_key_des_auth(papdu_command);												
					}
				}
				else if((cmd_status.authentication == 2) &&
						((app[app_idx].key_setting[0] & 0xF0) != 0xE0) && 
						(key_idx == (app[app_idx].key_setting[0] & 0xF0))) // bisa mengganti key number berapapun asalkan key authentikasinya harus sesuai dgn key setting 
			
				{
					if((((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  8) || (((app[app_idx].key_setting[1] >> 4) & 0x0F) ==  0xA)) // AES
					{
						iso_14443_change_key_aes_random(papdu_command);								
						
					}else{ //DES{
						iso_14443_change_key_des_random(papdu_command);
						
					}

				}
				else
				{
                    resp = (DESFIRE_SW1<<8) | AUTHENTICATION_ERROR;
                    iso14443sendresp(resp);
					
				} 					
				   
			}
			else
			{
                resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
                iso14443sendresp(resp);
                
			}

		}
		else
		{
            resp = (DESFIRE_SW1<<8) | PERMISSION_DENIED;
            iso14443sendresp(resp);			
		}
	}
	else
	{
		resp = (DESFIRE_SW1<<8) | NO_SUCH_KEY;
        iso14443sendresp(resp);
        
	}

}
