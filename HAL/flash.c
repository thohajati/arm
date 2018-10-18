#include "flash.h"
#include "XSIM_hard_api.h"
#include "reg.h"
unsigned char zerobuff[SECTOR_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //16
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //32
																		   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //64
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																		   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //128
																		   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																		   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																		   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
																			 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //256

extern unsigned char  backupbuff[SECTOR_SIZE];
//static unsigned char dbsel = 0;

void flash_sector_erase(unsigned char *addr)
{
	if(((unsigned int)addr) >= ADDR_MIN)
	{		
		addr = (unsigned char*)((unsigned int)addr>> 9); // 1 sector = 512 byte
//		FLASHX |= 0x01;
		NVM_CON |= 0x01;
		*addr = 0x30;
//		FLASHX &= 0xFE;	
		NVM_CON &= 0xFE;
	} 
}

static void flash_write_block(unsigned char * dest, unsigned char * source, unsigned short len) 
{

	NVM_SIZE = len;
	SRC_ADDR = (unsigned int)source;
	DST_ADDR = (unsigned int)dest;
	
	// Disable Sleep SLEEPDEEP
	*(unsigned int*) 0xE000ED10 = 0x00000000;
	
	NVM_CON |= 0x04; // Enable Write Flash
  __WFI();
  NVM_CON &= 0xFB; // Disable Write Flash
	
	// Enable Sleep SLEEPDEEP
	*(unsigned int*) 0xE000ED10 = 0x00000004;
}

void flash_write_byte(unsigned char * addr, unsigned char value)
{ 
	if(((unsigned int)addr) >= ADDR_MIN)
	{
//	    FLASHX |= 0x2;			 
		*(addr) = value;			 			
//		FLASHX &= 0xFD;
	}
}

void flash_sector_backup(unsigned char  *addr)
{
	int i;

	for(i=0; i<SECTOR_SIZE; i++)
	{	
	   backupbuff[i] = *(addr+i);	
	}

}

void flash_byte_set(unsigned char * addr, unsigned char value)
{
//	iso_rx_disable();
	flash_write_byte(addr, value);
//	iso_rx_enable();
}



//void flash_write(unsigned char xdata *sector_addr, 
//                 unsigned short addr_offset, 
//				 unsigned char xdata *data_buffer, 
//				 unsigned short length)
//{
//	int i;
//
//	iso_rx_disable();
//  	timer_on();
//	
//	for(i=0; i<length; i++)
//	{
//	  FLASHX |= 0x4;
//	  *(sector_addr + addr_offset + i) = data_buffer[i];			
//	  FLASHX &=  0xFB;
//	}
//
//  	timer_off();
//  	iso_rx_enable();
//
//}

void flash_write(unsigned char *sector_addr, 
                 unsigned short addr_offset, 
				 unsigned char  *data_buffer, 
				 unsigned short length)
{
    //unsigned short i,j;

		unsigned int mod;
	
	mod = (unsigned int)sector_addr % SECTOR_SIZE;

	if(mod !=0 ){
		sector_addr -= mod;
	  addr_offset += mod;
	}

	while (addr_offset >= SECTOR_SIZE)
	{
	  sector_addr += SECTOR_SIZE;
	  addr_offset -= SECTOR_SIZE;
	}

	//j = 0;


	while((addr_offset + length) >=  SECTOR_SIZE)
	{
		flash_sector_backup(sector_addr);
		flash_sector_erase(sector_addr);

//		for(i=0; i<addr_offset; i++)
//			flash_write_byte(sector_addr+i, backupbuff[i]);
		flash_write_block(sector_addr,backupbuff,addr_offset);

//		for(; i<SECTOR_SIZE; i++)
//			flash_write_byte(sector_addr+i, data_buffer[j++]);
		flash_write_block(sector_addr+addr_offset,data_buffer,SECTOR_SIZE-addr_offset);
			
		
		//i = 0; 
		sector_addr += SECTOR_SIZE;
		length = length - (SECTOR_SIZE - addr_offset);
		addr_offset = 0x00;
			
	}

	flash_sector_backup(sector_addr);
	flash_sector_erase(sector_addr);
	
	if(addr_offset > 0){
		if(addr_offset <= 256){
	
//	  for(i=0; i<addr_offset; i++)
//			flash_write_byte(sector_addr+i, backupbuff[i]);
			flash_write_block(sector_addr,backupbuff,addr_offset);
		}else{
				flash_write_block(sector_addr,backupbuff,0x100);
				flash_write_block(sector_addr+0x100,backupbuff+0x100,addr_offset-0x100);
	
		}
	}

//	for(; i<addr_offset+length; i++)
//		flash_write_byte(sector_addr+i, data_buffer[j++]);
	flash_write_block(sector_addr+addr_offset,data_buffer,length);

//	for(; i<SECTOR_SIZE; i++) 
//		flash_write_byte(sector_addr+i, backupbuff[i]);
	
//	if(SECTOR_SIZE-(addr_offset+length)<256){
//		flash_write_block(sector_addr+addr_offset+length, backupbuff+addr_offset,SECTOR_SIZE-addr_offset+length);
//	}		
//	else{
//	
//		flash_write_block(sector_addr+addr_offset+length, backupbuff+addr_offset,0x100);
//	  flash_write_block(sector_addr+0x100,backupbuff+0x100,SECTOR_SIZE-(addr_offset+length)-0x100);
//	}
	
  if(addr_offset+length != 0x1FF){	
		if((addr_offset+length)>256){
			flash_write_block(sector_addr+addr_offset+length, backupbuff+addr_offset+length,SECTOR_SIZE-addr_offset-length);
			
		}		
		else{
		
			//flash_write_block(sector_addr+addr_offset+length, backupbuff+addr_offset,0x100-addr_offset-length);
			flash_write_block(sector_addr+addr_offset+length, backupbuff+addr_offset+length,0x100-addr_offset-length);
			
			//flash_write_block(sector_addr+0x100,backupbuff+addr_offset+0x100-addr_offset-length,SECTOR_SIZE-(addr_offset-length)-0x100);
			flash_write_block(sector_addr+0x100,backupbuff+addr_offset+length+0x100-addr_offset-length,0x100);
			
			//flash_write_block(sector_addr+0x100+SECTOR_SIZE-(addr_offset-length)-0x100,backupbuff+addr_offset+0x100+SECTOR_SIZE-(addr_offset-length)-0x100,0x100);
		}
	}
	
//  flash_write_block(sector_addr+0x100,backupbuff+addr_offset+0x100,0x100);
}



void flash_format(unsigned char  *addr, unsigned short len)
{
//	unsigned short i;

//	iso_rx_disable();
//  	timer_on();

	while(len >= SECTOR_SIZE)
	{
		flash_sector_erase(addr);

//		for(i=0; i<SECTOR_SIZE; i++)
//			flash_write_byte(addr+i, 0x00);
		flash_write_block(addr,zerobuff,SECTOR_SIZE);
		flash_write_block(addr+0x100,zerobuff,SECTOR_SIZE);

		addr += SECTOR_SIZE;
		len -= SECTOR_SIZE;
	}

//	if(len)
//	{
//		flash_sector_erase(addr);	

////		for(i=0; i<len; i++)
////			flash_write_byte(addr+i, 0x00);
//		flash_write_block(addr,zerobuff,len);
//	}	
//		 
////  	timer_off();
////  	iso_rx_enable();	
//}


	if(len)
	{
		flash_sector_erase(addr);	

//		for(i=0; i<len; i++)
//			flash_write_byte(addr+i, 0x00);
		if(len < 256){
			flash_write_block(addr,zerobuff,len);
		}else{
			flash_write_block(addr,zerobuff,len-0x100);
			flash_write_block(addr+0x100,zerobuff,0x100);
			
		}
	}	

}	

