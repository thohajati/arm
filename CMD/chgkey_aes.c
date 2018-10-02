#include "vuiso14443.h"
#include "vuaes.h"
#include "cmd.h"
#include "xsim_hard_api.h"
#include "flash.h"
#include "auth_aes.h"

void iso_14443_change_key_aes_random(ISOAPDU * papdu_command)
{
	unsigned int cipher[16];
	unsigned int plain[16];
	aes_t  aes;
	unsigned int aes_seskey[4];
    uint8_t  buffer[64];
    uint8_t  buffer_flash[32];
    uint16_t i,resp;
    uint16_t key_offset;
	
	memcpy8to32(cipher,&papdu_command->papdudata[1],papdu_command->pheader->LEN - 1);		
	memcpy8to32(aes_seskey,sesion_key,16);
    
    /* AES decrypt */
	aes.presult = plain;
	aes.pbuffer = cipher;
	aes.bufferlen = 8;
	aes.piv = init_vector;
	aes.mode = AES_MODE_DECRYPT;
	aes.pkey = aes_seskey;

	vuaes_process(&aes);
	
	memcpy32to8(&buffer[2],plain,(papdu_command->pheader->LEN-1)/4);
	
	/* verify crc32 */
	buffer[0] = papdu_command->pheader->INS;
	buffer[1] = papdu_command->papdudata[0];
	if(!verify_veriu_crc32(buffer, 34))
	{
		resp = (DESFIRE_SW1<<8)|INTEGRITY_ERROR;

					iso14443sendresp(resp);

        
	}else
	{
		key_offset = 0;
		key_offset += app_idx<<8;
		key_offset += (papdu_command->papdudata[0])<<4;
    	
		for(i=0; i<16; i++)
		  buffer_flash[i] = key_buff[key_offset+i] ^ buffer[i+2];
    	
		buffer_flash[16] = buffer[18]; 
		buffer_flash[17] = buffer[19];
	
		flash_write(key_buff, key_offset, buffer_flash, 16);				
		 
		resp = (DESFIRE_SW1<<8)|OPERATION_OK;
  				iso14443sendresp(resp);
	
        
    	
	}
}
