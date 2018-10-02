#include "XSIM_Hard_Api.h"
#include "HAL_Init.h"
#include "reg.h"

void HAL_Init(void)
{
		 // Enable clock Interface
	 CLKCON |= 0x01;
	
   //IC Initialization
   IoInit(FD_9600);                   
}
