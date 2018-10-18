#include "vuiso14443.h"
#include "vuaes.h"
#include "cmd.h"
#include "xsim_hard_api.h"
#include "reg.h"

unsigned int init_vector[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

void memcpy8to32(unsigned int* dest, unsigned char* source, unsigned char srclen)
{
	unsigned char i,j;
	j=0;
				for(i=0;i<srclen/4;i++){
					dest[i] = source[j++]<<24;
					dest[i] |= source[j++]<<16; 
					dest[i] |= source[j++]<<8; 
					dest[i] |= source[j++];
				}
}

void memcpy32to8(unsigned char* dest, unsigned int* source, unsigned char srclen){
	unsigned char i,j;
	
				j=0;
				for(i=0;i<srclen;i++){
					dest[j++] = (source[i]>>24)&0xff;
					dest[j++] = (source[i]>>16)&0xff;
					dest[j++] = (source[i]>>8)&0xff;
					dest[j++] = (source[i])&0xff;
				}
}

void 
authenticate_aes(ISOAPDU* papdu_command)
{
    aes_t aes;
    unsigned int  buffer[64];
    uint16_t i,j,resp,key_offset;
    unsigned int  randA_AES[4];
    static unsigned int  key[4]  = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    static unsigned int  randB_AES[4];// = {0xA6, 0xD6, 0x0C, 0xDC, 0x7C, 0xF1, 0x46, 0x00,												  
							                 //    0x1E, 0x43, 0xD0, 0x2D, 0x39, 0xB2, 0x3D, 0x04};
		unsigned int apdudata[64];
		unsigned char temp[32];
		static unsigned char randB_byte[16];
		unsigned char randA_byte[16];
		unsigned char key_byte[16];		

  
	if((cmd_status.authentication == 0) || (cmd_status.authentication == 2)) 
	{
	
		key_idx = papdu_command->papdudata[0];

		if(((cmd_status.sel_app == PICC_LEVEL) && (key_idx == 0x00)) || 
		   ((cmd_status.sel_app == APPLICATION_LEVEL) && (key_idx < (app[app_idx].key_setting[1] & 0xF))))
		{
			if(cmd_status.sel_app == PICC_LEVEL)
			{
				for(i=0; i<16; i++)
				  key_byte[i] = master_key[i];

	
			}
			else if(cmd_status.sel_app == APPLICATION_LEVEL)
			{
				key_offset = 0;
				key_offset += (app_idx<<8); 
				key_offset += (key_idx<<4);
	
				for(i=0; i<16; i++)
				  key_byte[i] = key_buff[key_offset+i];
	
			}
	
			if((cmd_status.sel_app == PICC_LEVEL) || (cmd_status.sel_app == APPLICATION_LEVEL))
			{
				/* reset IV */
				for (i=0; i<4; i++)
					init_vector[i] = 0x00000000;				
				
				get_rand(16,randB_byte);
//				for(i=0;i<16;i++)
//					randB_byte[i] = 0x76;
				
				memcpy8to32(randB_AES,randB_byte,16);
				memcpy8to32(key,key_byte,16);
		
				aes.presult = buffer;							
				aes.pbuffer = randB_AES;
				aes.bufferlen = 4;
				aes.piv = init_vector;
				aes.pkey = key;
				aes.mode = AES_MODE_ENCRYPT;						
				
				vuaes_process(&aes);

				for (i=0; i<4; i++)
					init_vector[i] = buffer[i]; 
				
        resp = (DESFIRE_SW1<<8) | ADDITIONAL_FRAME; 

				
				memcpy32to8(temp,buffer,4);

				iso14443send(temp,16,resp);
                
				cmd_status.authentication = 1;
			}
			else
			{
          resp = (DESFIRE_SW1<<8)|PERMISSION_DENIED;
					iso14443sendresp(resp);				
			}
		}
		else
		{
            resp = (DESFIRE_SW1<<8)|NO_SUCH_KEY;
					iso14443sendresp(resp);
			
		}
	}
	else if (cmd_status.authentication == 1)
	{
		
		memcpy8to32(apdudata,papdu_command->papdudata ,papdu_command->pheader->LEN);
		memcpy8to32(key,key_byte,16);
		
		aes.presult = buffer;
		aes.pkey = key;
		aes.pbuffer = apdudata;
		aes.bufferlen = papdu_command->pheader->LEN / 4;
		aes.mode = AES_MODE_DECRYPT;
		aes.piv = init_vector;

		vuaes_process(&aes);										   	
		
		memcpy32to8(temp,buffer,papdu_command->pheader->LEN / 4);
		
	   j=0;
	   for(i=0; i<15; i++)
	   {
		  if(temp[16+i] == randB_byte[i+1])
			j++;
	
	   } 				

	   if ((j==15) && (temp[31] == randB_byte[0]))
	   {
	   	
			/* get session key */
			for(i=0; i<4; i++)
			  sesion_key[i] = temp[i];
	
			for(i=0; i<4; i++)
			  sesion_key[i+4] = randB_byte[i];

			for(i=0; i<4; i++)
			  sesion_key[i+8] = temp[i+12];
	
			for(i=0; i<4; i++)
			  sesion_key[i+12] = randB_byte[i+12];
			 

			
			/* generate random A' 
			 * rotate left random A 1 byte
			 */
			for(i=0; i<15; i++)
			  randA_byte[i] = temp[i+1];
	
			randA_byte[15] = temp[0];
			
			memcpy8to32(randA_AES,randA_byte,16);			
         
			memcpy8to32(apdudata,&papdu_command->papdudata[papdu_command->pheader->LEN-16],16);
                
			aes.presult = buffer;
			aes.pkey = key;
			aes.pbuffer = randA_AES;
			aes.bufferlen = 4;
			aes.mode = AES_MODE_ENCRYPT;
			aes.piv = apdudata;
	
			vuaes_process(&aes);			
			
			memcpy32to8(temp,buffer,4);
			
						/* reset IV */
			for (i=0; i<4; i++)
					init_vector[i] = 0x00000000;	
			
      resp = (DESFIRE_SW1<<8) | OPERATION_OK;            
			iso14443send(temp,16,resp);
            
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
