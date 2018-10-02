#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"
 
void get_ver(void)
{
    uint8_t  buffer[16];
    uint8_t i;
    
	if(cmd_status.get_version == 0)
	{	
		buffer[0] = 0x04;
		buffer[1] = 0x01;
		buffer[2] = 0x01;
		buffer[3] = 0x01;
		buffer[4] = 0x00;
		buffer[5] = 0x1A;
		buffer[6] = 0x05;

    iso14443send(buffer,7,ADDITIONAL_FRAME);
		
		cmd_status.get_version = 1;
	}
	else if(cmd_status.get_version == 1)
	{	
		buffer[0] = 0x04;
		buffer[1] = 0x01;
		buffer[2] = 0x01;
		buffer[3] = 0x01;
		buffer[4] = 0x03;
		buffer[5] = 0x1A;
		buffer[6] = 0x05;

        iso14443send(buffer,7,ADDITIONAL_FRAME);
		
		cmd_status.get_version = 2; 
	}
	else if (cmd_status.get_version == 2)
	{
	    for(i=0; i<7; i++)
		  buffer[i+1] = UID[i];

		buffer[8] = 0x00; 
		buffer[9] = 0x00;
		buffer[10] = 0x00; 
		buffer[11] = 0x00;
		buffer[12] = 0x00;
		buffer[13] = 0x39;
		buffer[14] = 0x08;
        
        iso14443send(&buffer[1],14,OPERATION_OK);
		
		cmd_status.get_version = 0;
	}

}
