#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "cmd.h"

void get_app()
{
    uint16_t i,j,k;
    uint8_t  buffer[20];
    
    i = 0;
	j = 0;
	
   //search all active application
	while(i<MAX_APP_NUM)
	{	
		if ((app[i].AID[0] != 0) ||
			(app[i].AID[1] != 0) ||
			(app[i].AID[2] != 0))
		{
			j++; //number of active application
		
		}
		
		i++;						

	}

	if(j<20) // 0 up to 19 AIDs
	{
		i = 0;
		k = 0;

		while(i<j)
		{
			buffer[k++] = app[i].AID[0];
			buffer[k++] = app[i].AID[1];
			buffer[k++] = app[i].AID[2];  	

			i++;	
		}
        iso14443send(buffer,k,OPERATION_OK);
									
	}
}
