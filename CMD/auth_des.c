#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"
#include "vutdes.h"			
#include "cmd.h"


void auth_des(ISOAPDU * papdu_command)
{    
    uint16_t i,j;
    uint16_t key_offset;
    uint16_t resp;
    uint8_t  randA[8];
    static uint8_t  key[16]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

    uint8_t  buffer[256]; 
		unsigned char f_auth;

	if((cmd_status.authentication == 0) || (cmd_status.authentication == 2)) 
	{
		key_idx = papdu_command->papdudata[0];
    
		if(((cmd_status.sel_app == PICC_LEVEL) && (key_idx == 0x00)) || 
		   ((cmd_status.sel_app == APPLICATION_LEVEL) && (key_idx < (app[app_idx].key_setting[1] & 0xF))))
		{
			if(cmd_status.sel_app == PICC_LEVEL)
			{
				for(i=0; i<16; i++)
				  key[i] = master_key[i];
    
	
			}
			else if(cmd_status.sel_app == APPLICATION_LEVEL)
			{
				key_offset = 0;                
				key_offset += (app_idx<<8); 
				key_offset += (key_idx<<4);
	
				for(i=0; i<16; i++)
				  key[i] = key_buff[key_offset+i];
	
			}
	
			if((cmd_status.sel_app == PICC_LEVEL) || (cmd_status.sel_app == APPLICATION_LEVEL))
			{

				get_rand(8,randB);

			  xstsm212_tdes16_encrypt_tx(randB, key, buffer, 8);	
				cmd_status.authentication = 1; 				
        iso14443send(buffer, 8, ADDITIONAL_FRAME);
				f_auth = 1; // hanya utk delay supaya jalan di sof 26 sept
			}
			else
			{
                resp = (DESFIRE_SW1 << 8) | PERMISSION_DENIED;
                iso14443sendresp(resp);				
			}
		}
		else
		{
            resp = (DESFIRE_SW1 << 8) | NO_SUCH_KEY;
            iso14443sendresp(resp);		
		}
	}
	else if (cmd_status.authentication == 1)
	{	

			
//		if(f_auth){}
			
	   xstsm212_tdes16_encrypt_rx(papdu_command->papdudata, key, buffer, 16);
       					   
	   j=0;
	   for(i=0; i<7; i++)
	   {
		  if(buffer[8+i] == randB[i+1])
			j++;
	
	   }

				
	   if ((j==7) && (buffer[15] == randB[0]))
	   {
	
			for(i=0; i<4; i++)
			  sesion_key[i] = buffer[i];
	
			for(i=0; i<4; i++)
			  sesion_key[i+4] = randB[i];
	
			j=0;
			for(i=0; i<8; i++)
			{
			   if(key[i] == key[i+8])
				  j++;
	
			}   
	
			if(j == 8)
			{
				for(i=0; i<4; i++)
				  sesion_key[i+8] = buffer[i];
	
				for(i=0; i<4; i++)
				  sesion_key[i+12] = randB[i];
			}
			else
			{
				for(i=0; i<4; i++)
				  sesion_key[i+8] = buffer[i+4];
	
				for(i=0; i<4; i++)
				  sesion_key[i+12] = randB[i+4];
			}
	
	
			for(i=0; i<7; i++)
			  randA[i] = buffer[i+1];
	
			randA[7] = buffer[0];
	
			xstsm212_tdes16_encrypt_tx(randA, key, buffer, 8);

            iso14443send(buffer,8, OPERATION_OK);
          
			cmd_status.authentication = 2;
			
	
	   }
	   else
	   {
            resp = (DESFIRE_SW1<<8)|AUTHENTICATION_ERROR;
            iso14443sendresp(resp);

			cmd_status.authentication = 0;							 
	   }
	
	}
	else
	{
        resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
        iso14443sendresp(resp);
		
	}
	
}
