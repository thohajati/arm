#include "vutype.h"
#include "vuiso14443.h"
#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_Hard_Api.h"
#include "cmd.h"
#include "instruction.h"

#define PROTO14443_3_CONT       	0x00000000    // 14443-3 protocol isn't finish yet
#define PROTO14443_3_FINISHED   	0x00000001    // 14443-3 protocol finished

#define PROTO14443_4_CONT       	0x02
#define PROTO14443_4_FINISHED   	0x03



// ISO 14443-3
#define REQA                        0x26
#define WUPA                        0x52
#define SELECT1                     0x93
#define SELECT2                     0x95
#define SELECT3                     0x97
#define FIRSTNVB                    0x20
#define LASTNVB                     0x70
#define CTAG                        0x88
/* SAK */
#define SAK_NOT_COMPLETE            0x04
#define SAK_COMPLETE_ISO14443_4     0x20
#define SAK_COMPLETE_PROPRIETARY    0x00
#define SAK_COMPLETE_ISO18092       0x40

// ATQA
#define ATQA_BYTE1                  0x44//0x44 default	   
#define ATQA_BYTE2                  0x03//0x03 default	   //0x00 thomi

// ISO 14443-4
#define RATS                        0xE0
#define HLTA                        0x50
#define IBLOCK                      0x02
#define RBLOCK_NAK                  0xB2
#define RBLOCK_ACK                  0xA2
#define DESELECT                    0xC2
#define DESELECT_CID                0xCA
#define PPS                         0xD0
#define SBLOCK_WTS                  0xF2

/* FSDI to FSD Conversion. The FSD defines the maximum
size of a frame the PCD is able to receive */
#define FSD16                       0x00
#define FSD24                       0x01
#define FSD32                       0x02
#define FSD40                       0x03
#define FSD48                       0x04
#define FSD64                       0x05
#define FSD96                       0x06
#define FSD128                      0x07
#define FSD256                      0x08


//RATS CODE
#define TA1_ENABLE                  0x10
#define TB1_ENABLE                  0x20
#define TC1_ENABLE                  0x40

#define DIR							0x00
#define	DIVISOR_SEND				0x00  //0x70  //original	   0x30//desfire
#define	DIVISOR_RECEIVE				0x00  //0x07  //original	   0x03//desfire
//#define	DIVISOR_SEND				0x30
//#define	DIVISOR_RECEIVE				0x30

#define FWI							0x80  ///*0xE0*/0x80	  //default 0x40		 0x80//desfire
//#define FWI							0x92
#define	SFGI						0x01  ///*0x0E*/0x01	  //default 0x00		 0x01//desfire

#define NAD_NOT_SUPPORTED			0x00
#define CID_NOT_SUPPORTED			0x00  //0x00 //original					 0x02//desfire


// RATS CONFIGURATION
#define TLENGTH                     0x06
#define	T0							TA1_ENABLE | TB1_ENABLE | TC1_ENABLE | FSD64 //FSD64 ori
#define TA1						 	DIR | DIVISOR_SEND | DIVISOR_RECEIVE
#define	TB1							FWI | SFGI
#define	TC1							CID_NOT_SUPPORTED | NAD_NOT_SUPPORTED
#define	HISTORICAL_BYTE				0x80

/* ISO14443-3 State machine */
#define ISO_STATE_IDLE      0
#define ISO_STATE_READY     1
#define ISO_STATE_ACTIVE    2
#define ISO_STATE_HALT      3

//#define NFC_BUFFER ( ( unsigned char*)0x20002780U )
//const uint8_t UID[7] =  {0x04, 0x2d, 0x4f, 0x21, 0xbd, 0x25, 0x80};//{0x04, /*0x31*/0x33, 0x91, 0xd2, 0x52, 0x4d, 0x80};
//uint8_t code UID[7] =  {0x04, 0x53, 0x92, 0x61, 0x80, 0x1d, 0x80}; //thomi
//04135021BD2580 red card
//047D4D7AAE2780 mifare bertanda
static uint8_t isostate = ISO_STATE_IDLE;
static uint8_t flag=0;
static uint8_t  tempbuff[10];
static uint16_t  datalen;

uint8_t  backupbuff[SECTOR_SIZE];
command_status  cmd_status;
uint8_t read_all_flag;
unsigned char f_coll = 0;

extern ISOAPDU  isoapdu;
extern uint8_t iblock_cnt;

BLOCK block;

void 
vuiso14443_init(void)
{
	
    // initiate status
   	cmd_status.sel_app = PICC_LEVEL;
	cmd_status.get_version = 0;
	cmd_status.authentication = 0;
	cmd_status.read_data = 0;
	cmd_status.write_data = 0;

    read_all_flag = 0;

}



void vutransmit(uint8_t length) //byte&bit transmit
{
//	CLKCFG &= 0x1F; 
	CL_SIZE = length;
	
	CL_CON = 0x3;
  //while ((CL_CON & 0x4) == 0x00);
	__WFI();
	//CL_CON = 0x0;
}

static void vureceive(uint16_t * outputlen)//byte&bit receive
{
	 IFC |= 0x00000001; // Set modulator 1
	
	 CL_CON = 0x1;
   //while ((CL_CON & 0x4) == 0x00);
	 __WFI();	
	 //CL_CON = 0x0;
	 
	 IFC &= 0xFFFFFFFE; // Set modulator 0
	
   *outputlen = CL_SIZE;	 
}


static void 
anticoll_lev1(uint8_t * pbuff, uint8_t idxbytecoll, uint8_t idxbitcoll){
	uint8_t idxbcc,idxuidcoll,i;//,bcc;

	switch(idxbytecoll){
		case 1: idxbcc = 4; break;
		case 2: idxbcc = 3; break;
		case 3: idxbcc = 2; break;
		case 4: idxbcc = 1; break;
	}
	//pbuff[idxbcc-1] = 0x00; //init bcc
    pbuff[idxbcc-1] = CTAG;
	
    if(idxbytecoll < 4){
        idxuidcoll = idxbytecoll - 1;
        pbuff[0] = UID[idxuidcoll] >> idxbitcoll;
    }

    
    if(idxbcc > 1){
        for(i=1;i<idxbcc-1;i++)
            pbuff[i] = UID[idxuidcoll+i];
    }
    
   	for(i=0;i<3;i++)
		pbuff[idxbcc-1] ^= UID[i];
    
	//bcc=pbuff[idxbcc-1];
    vutransmit(idxbcc);    

} 
 
static void 
anticoll_lev2(uint8_t * pbuff, uint8_t idxbytecoll, uint8_t idxbitcoll){
	uint8_t idxbcc,idxuidcoll,i;//,bcc;

	switch (idxbytecoll){
		case 4: idxbcc = 5; break;
		case 5: idxbcc = 4; break;
		case 6: idxbcc = 3; break;
		case 7: idxbcc = 2; break;
		case 8: idxbcc = 1; break;
	}
	//pbuff[idxbcc-1] = 0x00; //init bcc
	
    if(idxbytecoll < 8){
        idxuidcoll = idxbytecoll - 1;
        pbuff[0] = UID[idxuidcoll] >> idxbitcoll;
    }
    
    if(idxbcc > 1){
        for(i=1;i<idxbcc-1;i++)
            pbuff[i] = UID[idxuidcoll+i];
    }
	
    pbuff[idxbcc-1] = 0x00; //init bcc
   	for(i=0;i<4;i++)
		pbuff[idxbcc-1] ^= UID[i+3];
    
	//bcc=pbuff[idxbcc-1];
    vutransmit(idxbcc); 
    

}

static uint8_t iso14443_3_handler(uint8_t * pbuff)
{
		uint8_t idxbytecoll,idxbitcoll,idxuidcoll,res,i,plus;
    uint8_t command = pbuff[0];
	
//		CLKCFG |= 0xE0; 

    switch(isostate)
		{
        case ISO_STATE_IDLE:
        {
            if(command == WUPA || command == REQA)
						{
                pbuff[0] = ATQA_BYTE1;
                pbuff[1] = ATQA_BYTE2;
                vutransmit(2);
                isostate=ISO_STATE_READY;
			    //CFG = 0x00;				//thomi
                return PROTO14443_3_CONT;	//comment thomi
            }
        }
        break;

        case ISO_STATE_READY:
        {

            if(command==SELECT1)
						{
                if(pbuff[1] == FIRSTNVB) 
								{ 
                /* Number of valid bits = 16 bits( 2 Byte) , We'll send our first 3 byte UID CLn 
                   using format: CT + UID[0] + UID[1] + UID[2] + BCC 
                 */
                   

                    pbuff[0] = CTAG;
                    pbuff[4] = 0x00;
                    pbuff[4] ^= pbuff[0];
                    for ( i=0; i<3; i++)
										{
											pbuff[i+1] = UID[i];
											pbuff[4] ^= UID[i];
                    }
										//bcc=pbuff[4];
                    vutransmit(5);
                    return PROTO14443_3_CONT;
                }

                else if(pbuff[1] == LASTNVB)
								{
									/* we have response from PCD, we should continue verified its UID CLn to ours
										then we informs the PCD that there will be another UID (we are double UID)
										NOTE: The 2 byte CRC data should be inverted ( Annex B in ISO 14443-3)
									*/
	
									/* handling ketika ada 2 picc ditempel tapi reader tidak mengirim 93 XX */
										res = 0;
										for(i=0;i<3;i++){
										if(pbuff[i+3] != UID[i])
											res = 1;
										}
										if(res == 0)
										{
												pbuff[0] = 0x24;   
												//pbuff[0] = SAK_NOT_COMPLETE;//thomi	When b3 is set to 1, all other bits of SAK should be set to 0 [refer to 14443-3]
												vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);					
												vutransmit(3);
												return PROTO14443_3_CONT;
											
										}else
										{
												isostate = ISO_STATE_HALT;
												return PROTO14443_3_CONT;
										}
												
								}
								else // collision occurs
								{                   
										/* contoh C-APDU kalau terjadi collision                                                                                
										* ex: C-APDU = 93 42 88 04 03; 
										*     0x42 -> 4-2+1 = posisi byte collision; 2 = posisi bit collision
										*     0x03 -> value 2 bit dari UID yg dipilih  
										*/
				
										idxbytecoll = (pbuff[1] >> 4) - 2;
										idxbitcoll = pbuff[1] % 8;
				
										if(idxbytecoll > 0){
											idxuidcoll = idxbytecoll - 1;
											if(idxbitcoll > 0){
													res = ((UID[idxuidcoll] << (8-idxbitcoll)) & 0xFF) >> (8-idxbitcoll);
												}else{
												res = UID[idxbytecoll];
											}
										}else{
											res = ((CTAG << (8-idxbitcoll)) & 0xFF) >> (8-idxbitcoll);;
										}
				
										if(idxbitcoll > 0){
											plus = pbuff[idxbytecoll+2];
										}else{
											plus = UID[idxbytecoll];
										}
										
										if(res == plus){  // this UID is selected
												f_coll = 1;
												//TX_ADDR_BIT = (8-idxbitcoll);
												CL_SIZE = (8-idxbitcoll)<<8; 
																		
												anticoll_lev1(pbuff, idxbytecoll, idxbitcoll); 
										
												//TX_ADDR_BIT=0x00;
												CL_SIZE = 0x00 << 8;
												flag=1;
												return PROTO14443_3_CONT;
				
										}else{
												isostate = ISO_STATE_HALT;
												return PROTO14443_3_CONT; 
										}
				
						
								}
                
						}            
            else if(command == SELECT2)
						{

								/* Select cascade no 2 begin here */
                if(pbuff[1] == FIRSTNVB)
								{
                
                
                /* Send last 4 byte UID */
                    pbuff[4] = 0x00;
                    for ( i=0; i<4; i++)
										{
                        pbuff[i] = UID[i+3];
                        pbuff[4] ^= UID[i+3];
                    }
										//bcc1=pbuff[4];
                    vutransmit(5);
                    return PROTO14443_3_CONT;
                }
                else if(pbuff[1] == LASTNVB)
								{
										/* handling ketika ada 2 picc ditempel tapi reader tidak mengirim 93 XX */
										res = 0;
										for(i=3;i<7;i++){
											if(pbuff[i-1] != UID[i])
											res = 1;
										}
									
										if(res == 0)
										{
													/* using 14443_4 protocol */
													pbuff[0] = SAK_COMPLETE_ISO14443_4;
													vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);
													vutransmit(3);
													isostate = ISO_STATE_ACTIVE;
													//return PROTO14443_3_CONT;	//un-comment thomi
										}
										else
										{
												isostate = ISO_STATE_HALT;
												return PROTO14443_3_CONT;
										}
								}
								else
								{
										/* contoh C-APDU kalau terjadi collision                                                                                
									* ex: C-APDU = 93 42 88 04 03; 
									*     0x42 -> 4-2+1 = 3 = posisi byte collision; 2 = posisi bit collision
									*     0x03 -> value 2 bit dari UID yg dipilih  
									*/

									idxbytecoll = (pbuff[1] >> 4) + 2;
									idxbitcoll = pbuff[1] % 8;
			
									if(idxbytecoll > 0){
										idxuidcoll = idxbytecoll - 1;
										if(idxbitcoll > 0){
												res = ((UID[idxuidcoll] << (8-idxbitcoll)) & 0xFF) >> (8-idxbitcoll);
											}else{
											res = UID[idxbytecoll];
										}
									}else{
										res = ((CTAG << (8-idxbitcoll)) & 0xFF) >> (8-idxbitcoll);;
									}

									if(idxbitcoll > 0){
										plus = pbuff[idxbytecoll-2];
									}else{
										plus = UID[idxbytecoll];
									}
									
									if(res == plus){  // this UID is selected
											//TX_ADDR_BIT = (8-idxbitcoll);
											f_coll = 1;
											CL_SIZE = (8-idxbitcoll)<<8;
																	
											anticoll_lev2(pbuff, idxbytecoll, idxbitcoll); 
									
											//TX_ADDR_BIT=0x00;
											CL_SIZE = 0<<8;
											flag=1;
											return PROTO14443_3_CONT;
			
									}else{
											isostate = ISO_STATE_HALT;
																return PROTO14443_3_CONT; 
									}
                    
								}	
						}
            else if(command == WUPA || command == REQA)	//thomi
						{									  //thomi
                pbuff[0] = ATQA_BYTE1;	 //thomi
                pbuff[1] = ATQA_BYTE2;	  //thomi
                vutransmit(2);			  //thomi
                //isostate=ISO_STATE_READY; //thomi	 commented cause already in this state, can even disturb idling (with WUPA) communication if enabled
								//CFG = 0x00;				//thomi
                return PROTO14443_3_CONT; //thomi
            }
						//	else if(command	==0x50)	{ //khusus reader hitam
								//    	vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1); //tambahan
						//		vutransmit(3);											   //tambahan
						//		return PROTO14443_3_CONT;								  //tambahan
						//	}															  //tambahan
            else
						{
                isostate = ISO_STATE_IDLE;
								//isostate=ISO_STATE_READY; //thomi
                return PROTO14443_3_CONT;
            }
				}
        break;
        case ISO_STATE_ACTIVE:        
        
            if(command == HLTA)
						{
                isostate = ISO_STATE_HALT;
                return PROTO14443_3_CONT;
            }
            else if(command == WUPA || command == REQA)	//thomi
						{									  //thomi
                pbuff[0] = ATQA_BYTE1;	 //thomi
                pbuff[1] = ATQA_BYTE2;	  //thomi
                vutransmit(2);			  //thomi
                isostate=ISO_STATE_READY; //thomi
								//CFG = 0x00;				//thomi
                return PROTO14443_3_CONT; //thomi
            }							  //thomi
            else
						{
                return PROTO14443_3_FINISHED;
            }
        
        case ISO_STATE_HALT:
            if(command == WUPA)
						{
                pbuff[0] = ATQA_BYTE1;
                pbuff[1] = ATQA_BYTE2;
                vutransmit(2);
                isostate = ISO_STATE_READY;
				//CFG = 0x00;				//thomi
                return PROTO14443_3_CONT;	 //comment thomi
            }

        break;        
    }
    return PROTO14443_3_CONT;
}


static uint8_t iso14443_4_handler(uint8_t * pbuff)
{
    uint8_t  cmd;
    uint8_t  blocknum;
    uint16_t resp;
		const unsigned char masterkeyval[1] = {0x0F};

    cmd = pbuff[0] & 0xFE;
    blocknum = pbuff[0] & 0x01;
		if (flag==2)
			tempbuff[1]=0xAA;
		else if (flag==3)
			tempbuff[1]=0xAB;
		else
			tempbuff[1]=0xEE;
		
		
    switch(cmd)
    {
        case WUPA :						 //thomi
					pbuff[0] = ATQA_BYTE1;	 //thomi
					pbuff[1] = ATQA_BYTE2;	  //thomi
					vutransmit(2);			  //thomi
					isostate=ISO_STATE_READY; //thomi
			    //CFG = 0x00;				//thomi
                //return PROTO14443_3_CONT; //thomi
					break;							  //thomi
		
				case RATS:
            /* Hardcoded the ATS command */
            pbuff[0] = TLENGTH;
						pbuff[1] = T0;
            pbuff[2] = TA1;
            pbuff[3] = TB1;
            pbuff[4] = TC1;
            pbuff[5] = HISTORICAL_BYTE;
						vucalc_crc(NFC_BUFFER, &NFC_BUFFER[7], &NFC_BUFFER[6], 6);
						flag=2;
            vutransmit(8);
        break;

        case PPS: //d0 11 0f
            tempbuff[0]=pbuff[2];
						vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);
						flag=3;
						vutransmit(3);
						CL_CFG = tempbuff[0] & 0xf; //0xf still has effect. even though it should not, because RX and TX speed only uses CFG[0:3]
						//CFG = 0xe & 0xf; //thomi
				break;    

        case IBLOCK: //02 00
						if((NFC_BUFFER[1] == 0x00)||(NFC_BUFFER[1] == 0x51)||(NFC_BUFFER[1]==0x90)) //iso-7816
						{
							return PROTO14443_4_FINISHED;
						}    
						else
						{
							pbuff[1]=0x91;
							pbuff[2]=ILLEGAL_COMMAND_CODE;
							vucalc_crc(NFC_BUFFER, &NFC_BUFFER[4], &NFC_BUFFER[3], 3);
							vutransmit(5);
						}
							return PROTO14443_4_FINISHED;        
 		
				case RBLOCK_NAK:
            if (blocknum == 0x01)
                pbuff[0] = RBLOCK_ACK;
            else
                pbuff[0] = 0xA3;
            
            vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);
            vutransmit(3);
        break;
        
        case SBLOCK_WTS:			

							flash_format(app_buff, SIZE_APP_BUFF);
							flash_format(file_buff, SIZE_FILE_BUFF);
							flash_format(key_buff, SIZE_KEY_BUFF); 
							flash_format(master_key, SIZE_MASTER_KEY_BUFF);
//								flash_byte_set(master_key_setting, 0x0F);
							flash_write(master_key_setting,0, (unsigned char*)masterkeyval, 1);
					
							NFC_BUFFER[0] = cmd_status.prev_block;
								
							resp = (DESFIRE_SW1<<8)|OPERATION_OK;
							iso14443sendresp(resp);
			  break;
        
        case DESELECT:
            vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);
            vutransmit(3);
            isostate = ISO_STATE_HALT;
						CL_CFG = 0x00;				//thomi
						//return PROTO14443_3_CONT; //thomi
						break;

 //       case DESELECT_CID:
 //           vucalc_crc(NFC_BUFFER, &NFC_BUFFER[2], &NFC_BUFFER[1], 1);
 //           vutransmit(3);
 //           isostate = ISO_STATE_HALT;
 //       break;
           
        case 0x00:	//comment thomi
            break;	//comment thomi

    }
    return PROTO14443_4_CONT;
}
uint16_t
iso14443receive(ISOAPDU  *pisoapdu)
{
    while(1)
	{
		vureceive(&datalen);
		if(isostate != ISO_STATE_ACTIVE){
			if(iso14443_3_handler(&NFC_BUFFER[0]) == PROTO14443_3_CONT)
			{
            	continue;
        	}
		}
        
        
        if(iso14443_4_handler(&NFC_BUFFER[0]) == PROTO14443_4_CONT)
		{
            continue;
        }
        
        break;
		

    }
    
    
    pisoapdu->pheader   = (APDUHEADER *)&NFC_BUFFER[1];
    pisoapdu->papdudata = (uint8_t *)&NFC_BUFFER[6];
    return 0;//datalen-1;
}

void
iso7816send(uint8_t * payload, uint16_t len, uint8_t status)
{
  uint16_t cnt;
	uint8_t sw1,sw2;
//	uint8_t LRC;

//	
//	if((VDET & 0x40)==0x40){ //mode contact
//	
//		LRC= block.NAD^block.PCB;
//		
//		iso_tx(block.NAD);
//		iso_tx(block.PCB);
//	
//		LRC ^= len+2;
//	
//		iso_tx(len+2);
//		for(cnt=0;cnt<len;cnt++)
//		{
//			iso_tx(payload[cnt]);
//			LRC ^= payload[cnt]; 
//		}  
//	
//		iso_tx(0x90);
//		iso_tx(status);
//	
//		LRC ^= (0x90 ^ status);
//	
//		iso_tx(LRC);
//		
//	}else{

    for(cnt=0;cnt<len;cnt++)
		{
        NFC_BUFFER[cnt+1] = payload[cnt];
    }
		sw1 = 0x90;
		sw2 = status;

    NFC_BUFFER[len+1]=sw1;
    NFC_BUFFER[len+2]=sw2;
    vucalc_crc(NFC_BUFFER, &NFC_BUFFER[len+4], &NFC_BUFFER[len+3], len+3);
    vutransmit(len+5);
//	}
}

void
iso14443send(uint8_t * payload, uint16_t len, uint8_t status)
{
  uint16_t cnt;
//	uint8_t LRC;
	uint8_t sw1,sw2;

//	
//	if((VDET & 0x40)==0x40){ // mode contact
//		
//		LRC= block.NAD^block.PCB;
//		
//		iso_tx(block.NAD);
//		iso_tx(block.PCB);
//	
//		LRC ^= len+2;
//	
//		iso_tx(len+2);
//		for(cnt=0;cnt<len;cnt++)
//		{
//					iso_tx(payload[cnt]);
//			LRC ^= payload[cnt]; 
//		}  
//	
//		iso_tx(DESFIRE_SW1);
//		iso_tx(status);
//	
//		LRC ^= (DESFIRE_SW1 ^ status);
//	
//		iso_tx(LRC);
//			
//	}else{
    
    for(cnt=0;cnt<len;cnt++)
		{
        NFC_BUFFER[cnt+1] = payload[cnt];
    }
		sw1 = 0x95;
		sw2 = status;

    NFC_BUFFER[len+1]=sw1;
    NFC_BUFFER[len+2]=sw2;
    vucalc_crc(NFC_BUFFER, &NFC_BUFFER[len+4], &NFC_BUFFER[len+3], len+3);
    vutransmit(len+5);
//	}
}

void
iso14443sendresp(uint16_t resp){
//	uint8_t LRC; 
//	
//	if((VDET & 0x40) == 0x40){ //mode contact

//		LRC= block.NAD^block.PCB;
//		
//		iso_tx(block.NAD);
//		iso_tx(block.PCB);
//	
//		LRC ^= 0x2; //len status
//		LRC ^= resp >> 8;
//		LRC ^= resp & 0xFF;
//	
//		iso_tx(0x2);
//		iso_tx(resp >> 8);
//		iso_tx(resp & 0xFF);
//		iso_tx(LRC);
//		
//	}else{
			
    NFC_BUFFER[1] = (resp >> 8);
    NFC_BUFFER[2] = resp & 0xFF;
    vucalc_crc(NFC_BUFFER, &NFC_BUFFER[4], &NFC_BUFFER[3], 3);
    vutransmit(5);    
//	}
}

void
iso14443waitreq(){
//	uint8_t LRC, i;
//	
//	if((VDET & 0x40) == 0x40){ //mode contact
// 
//		uint8_t  buff[8]; 
//									
//		buff[0]= block.NAD;
//		buff[1]= 0xC3;
//		buff[2]= 0x01;
//		buff[3]= 0x9F;
//	
//		LRC = 0;
//		for(i=0;i<4;i++) {
//			LRC ^= buff[i];
//		}
//		buff[4]= LRC;
//		for(i=0;i<5;i++) {
//			iso_tx(buff[i]);
//		}
//		
//	}else{
		
		cmd_status.prev_block = NFC_BUFFER[0];
	
		NFC_BUFFER[0] = 0xF2;
		NFC_BUFFER[1] = 0x3B;
	
		vucalc_crc(NFC_BUFFER, &NFC_BUFFER[3], &NFC_BUFFER[2], 2);
	
		vutransmit(4);
//	}
    
}


