#include "vuiso14443.h"
#include "flash.h"
#include "firmware.h"
#include "instruction.h"
#include "vutdes.h"		
#include "system_statusword.h" 	
#include "cmd.h"
#include "reg.h"

application  app[MAX_APP_NUM]		__attribute__((at(0x6000)));//_at_ 0x6000;
//application  app[MAX_APP_NUM]		__attribute__((at(0x12000)));
//uint8_t code UID[7] =  {0x67, 0x2d, 0x4f, 0x21, 0xbd, 0x25, 0x80};


uint8_t  randB[8];
uint8_t key_idx;
uint8_t app_idx;
uint8_t file_idx;	
extern uint8_t read_all_flag;
uint8_t  sesion_key[16];
uint16_t real_data_length;


void master_key_format(void)
{
	uint16_t i;
	
	for(i=0; i<16; i++)
    { 		 
		 //FLASHX |= 0x2;		
     NVM_CON |= 0x2;				
		 master_key[i] = 0;		
		 //FLASHX &= 0xFD;	
     NVM_CON &= 0xFD;	
    }	
		 
	//FLASHX |= 0x2;	
     NVM_CON |= 0x2;		
	master_key_setting[0] = 0x0F;	
	//FLASHX &= 0xFD;
     NVM_CON &= 0xFD;	
}

void firmware_init(ISOAPDU * apdu)
{
 	if (apdu->pheader->INS == ADDITIONAL_FRAME)
    {
	  	apdu->pheader->INS = cmd_status.prev_command;                   
   	}
	else
	{
		cmd_status.get_version = 0;
		cmd_status.read_data = 0;
		cmd_status.write_data = 0;
		
		if(cmd_status.authentication == 1)
		  cmd_status.authentication = 0;  
	}

	cmd_status.prev_command = apdu->pheader->INS;
}
                          
void 
firmware_response(ISOAPDU * papdu_command)
{
	firmware_init(papdu_command);

    switch(papdu_command->pheader->INS)
    {
        case CREATE_APPLICATION:
            create_app(papdu_command);
            break;
        case SELECT_APPLICATION:
            sel_app(papdu_command);
            break;
        case AUTHENTICATE_DES:
            auth_des(papdu_command);
            break;
				case AUTHENTICATE_AES:
            authenticate_aes(papdu_command);
            break;
        case FORMAT_PICC:
            format_picc(papdu_command);
            break;
        case GET_APPLICATION_ID:
            get_app();
            break;
        case DELETE_APPLICATION:
            delete_app(papdu_command);
            break;
        case GET_VERSION:
            get_ver();
            break;
		case CREATE_STD_DATA_FILE :   	   
		case CREATE_BACKUP_DATA_FILE : 
		case CREATE_VALUE_FILE :       
		case CREATE_LINEAR_RECORD_FILE :	
		case CREATE_CYCLIC_RECORD_FILE :	
            create_file(papdu_command);
            break;
        case GET_FILE_ID:
            get_file_id();
            break;
        case GET_FILE_SETTING:
            get_file_set(papdu_command);
            break;
        case WRITE_DATA:
            write_data(papdu_command);
            break;
        case READ_DATA:
            read_data(papdu_command);
            break;
        case CHANGE_FILE_SETTING:
            change_file_set(papdu_command);
            break;
        case CHANGE_KEY:
            change_key(papdu_command);
            break;
		case GET_KEY_SETTING:
			get_key_set();
			break;	
		case GET_KEY_VERSION:
			get_key_ver(papdu_command);
			break;
    }
}

void 
firmware_iso7816_response(ISOAPDU * papdu_command)
{
	switch(papdu_command->pheader->INS)
    {
		case ISO_SELECT:
			iso_select(papdu_command);
			break;
		case ISO_READ:
			iso_read(papdu_command);
			break;
		case ISO_WRITE:	  
			iso_write(papdu_command);
			break;	
		default:
			iso14443sendresp(SW_ERR_INS);;
			break;   
	}
}

void 
firmware_xirka_response(ISOAPDU * papdu_command)
{
	unsigned int addr;

	
	if(papdu_command->pheader->INS == 0x02)
			{	
				//Chip Erase
				 NVM_CON = 0x05;
//	   			FLASHX = 0x05;	  //Enable erase mode
	   			*(unsigned char*)0x5555 = 0x10;
//	   			FLASHX = 0x00;
				 NVM_CON = 0x00;
			}
			if(papdu_command->pheader->INS == 0x03)
			{
				if(((UID[7] == 0xFF) || (UID[7] == 0x00)) && (papdu_command->pheader->LEN<=8))
				{
					flash_write(UID, 0, papdu_command->papdudata, papdu_command->pheader->LEN);
				}				
				iso14443sendresp(SW_OK);
			}
			else if(papdu_command->pheader->INS == 0x04)
			{
				addr = (papdu_command->pheader->P1<<8)  + papdu_command->pheader->P2;				
				iso14443send((uint8_t *) addr, papdu_command->pheader->LEN,(SW_OK&0xFF));		
			} 
			else
			{			
				iso14443sendresp(SW_ERR_INS);
			}
		
}

void delay (void)
{
	int z;
		for(z=0; z<1000; z++)
			{
				z=z+0;
			}
}

void delay_2 (void)
{
	int z;
		for(z=0; z<100; z++)
			{
				z=z+0;
			}
}

void delay_3 (void)
{
	int z;
		for(z=0; z<6; z++)
			{
				z=z+0;
			}
}

