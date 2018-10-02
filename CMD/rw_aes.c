#include "vuiso14443.h"
#include "vuaes.h"
#include "cmd.h"
#include "xsim_hard_api.h"
#include "flash.h"
#include "auth_aes.h"
#include "reg.h"


/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
      c = (unsigned long) n;
      for (k = 0; k < 8; k++) {
        if (c & 1)
          c = 0xEDB88320L ^ (c >> 1);
        else
          c = c >> 1;
      }
      crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

unsigned long update_crc(unsigned long crcinit, unsigned char *buf, unsigned char* crc32,
                            int len)
{

	int n;

			//Enable CRC Clock
     CLKCON |= 0x20;
			
			CRC_MODE = 0x3;
      CRC_INIT = 0x1;
	    CRC_DATA = crcinit;
	    CRC_INIT = 0x0;
			
			for(n=0; n<len; n++)
	    {
	      CRC_DATA = *(buf+n);
		  }
			
				crc32[3] = (unsigned char) (CRC_DATA>>24);
        crc32[2] = (unsigned char) (CRC_DATA>>16);
	      crc32[1] = (unsigned char) (CRC_DATA>>8);
	      crc32[0] = (unsigned char) CRC_DATA;

			//disable CRC Clock
    CLKCON &= 0xDF;			

		return 0;

//unsigned long c = crcinit;
//	int n;

//    if (!crc_table_computed)
//      make_crc_table();
//    for (n = 0; n < len; n++) {
//      c = crc_table[(c ^ buf[n]) & 0xFF] ^ (c >> 8);
//    }

//    crc32[3] = (unsigned char) (c>>24);
//    crc32[2] = (unsigned char) (c>>16);
//	crc32[1] = (unsigned char) (c>>8);
//	crc32[0] = (unsigned char) c;
//    
//    return c;        


}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long veriu_crc32 (unsigned char *buf,  unsigned char* crc32, unsigned int len)
{
     return update_crc(0xFFFFFFFFL, buf, crc32, len);// ^ 0xFFFFFFFFL;
}

unsigned char verify_veriu_crc32(unsigned char* buff, unsigned int len)
{
    unsigned char crc32[4];
    unsigned char i;
    
    veriu_crc32(buff,crc32,len);
    
    for (i=0; i<4; i++)
	{
		if (crc32[i] != 0)
			return 0;		
	}
	return 1;
    
}

void 
iso_14443_aes_write_data(ISOAPDU * papdu_command)
{
    unsigned int cipher[20],plain[20],initvector[4];
    uint8_t buffer[256];
		uint8_t plain_byte[64];
    aes_t  aes;
    uint16_t resp,i;
		desfire_write_data * write_data;
		unsigned int aes_seskey[4];		
	uint16_t real_data_length; 
	uint16_t data_offset;
	uint8_t gap;
	
	write_data = (desfire_write_data *)papdu_command->papdudata;
	real_data_length = (write_data->length[1]<<8) | write_data->length[0];
	data_offset = (write_data->offset[1]<<8) | write_data->offset[0];				
  
		
		memcpy8to32(cipher,write_data->data_to_write,papdu_command->pheader->LEN - 7);
		memcpy8to32(aes_seskey,sesion_key,16);
	
	/* aes decrypt */
	aes.piv = init_vector;
  aes.pkey = 	aes_seskey;
	aes.presult = plain;/*&buffer[8]*/
	aes.pbuffer = cipher;
	aes.bufferlen = (papdu_command->pheader->LEN - 7)/4;
	aes.mode = AES_MODE_DECRYPT;					  	

	vuaes_process(&aes);
	
	buffer[0] = papdu_command->pheader->INS;
	for(i=1; i<=7; i++ )
		buffer[i] = papdu_command->papdudata[i-1];
	
	memcpy32to8(plain_byte,plain,(papdu_command->pheader->LEN - 7)/4);

	for(i=8; i<real_data_length +4 +8; i++) //8byte header, realdatalen data, 4byte crc, 
		buffer[i] = plain_byte[i-8];
	
	/* verify crc32 */																		
	if(!verify_veriu_crc32(buffer, real_data_length +4 +8)){ //8 byte header + real data + 4 byte crc		
		
		resp = (DESFIRE_SW1<<8)|INTEGRITY_ERROR;

		iso14443sendresp(resp);

		
	}else{
								
		data_offset += app[app_idx].app_file[file_idx].file_addr;												
	
		flash_write(file_buff, data_offset, /*&buffer[8]*/plain_byte, write_data->length[0]);         	
		
    gap = 8+ real_data_length +4 +8  ;
		if(gap%4 != 0)
				gap += 4 - (gap%4);
		
		/* build R-APDU */
		buffer[0] = 0x00;
		memcpy8to32(plain,buffer,gap);
		
		memcpy8to32(initvector,&papdu_command->papdudata[papdu_command->pheader->LEN -16],16);
		
	  aes.presult = cipher;
		aes.pkey = aes_seskey;
		aes.pbuffer = plain;
		aes.bufferlen = 1;
		aes.piv = initvector; // rest 16 bytes write process		
		
		vuaes_cmac(&aes);
		
		memcpy32to8(buffer,cipher,8);
        
      resp = (DESFIRE_SW1<<8) | OPERATION_OK;
        
			iso14443send(buffer,8,resp);
		    
		
		//update IV
		memcpy8to32(init_vector,buffer,16);
	
	}
}
 
void 
iso_14443_aes_read_data(ISOAPDU * papdu_command, uint8_t* preaddata, uint16_t data_length)
{
	  unsigned int cipher[20],plain[20];
    uint8_t  buffer[256];
    uint16_t i;
    uint8_t  crc32_buff[4];
    aes_t  aes;
	uint16_t resp;
	unsigned int aes_seskey[4];
    
    /* update IV */
	buffer[0] = papdu_command->pheader->INS;
	for (i=1; i<=7; i++)
		buffer[i] = papdu_command->papdudata[i-1];
	
	memcpy8to32(cipher,buffer,8);
	memcpy8to32(aes_seskey,sesion_key,16);
	
	aes.pbuffer = cipher;
	aes.pkey = aes_seskey;
	aes.bufferlen = 2;
	aes.piv = init_vector;													
	aes.presult = init_vector;						
	
	vuaes_cmac(&aes);
	  
	/* calculate crc32
	  input = data + 0x00 */													
	*(preaddata+data_length) = 0x00;													
	veriu_crc32(preaddata, crc32_buff, data_length + 1);	
								
	i = data_length + 4;
	while (data_length < i){
		*(preaddata+data_length) = crc32_buff[data_length - (i-4)];
		data_length++; 
	}
	
	// zero padding													
	if (data_length % 16 != 0){
		for(i=data_length; i<data_length + (16 - (data_length % 16)); i++)
			*(preaddata+i) = 0x00;

		data_length += 16 - (data_length % 16);
	}else{
		data_length++;
	}													

	memcpy8to32(plain,preaddata,data_length);
	/* aes encrypt */
	aes.piv = init_vector;
	aes.pkey = aes_seskey;
	aes.presult = cipher;																				
	aes.pbuffer = plain;
	aes.bufferlen = data_length/4;
	aes.mode = AES_MODE_ENCRYPT;							

	vuaes_process(&aes);
	
	memcpy32to8(&buffer[1],cipher,data_length/4);
	
    resp = (DESFIRE_SW1<<8) | OPERATION_OK;
		iso14443send(&buffer[1],data_length, resp);    

	

	/* update IV
	   rest 16 bytes response data
	*/
	memcpy8to32(init_vector, &buffer[data_length + 1 - 16],16);
														
}
