#include "reg.h"
#include "vuiso14443.h"
#include "firmware.h"


void xstsm212_tdes_decrypt(unsigned char* datain, unsigned char* keyin, unsigned char* dataout)
{
   unsigned char i;	
   //unsigned char idata temp;

//Enable DES Clock
CLKCON |= 0x08;
      
  //decrypt mode, key in mode
  DES_CON = 0x06 ; //0000_0110
  
//  //Insert key
  for (i=0;i<8;i++){
     //temp = keyin[i];   
     DES_IN = keyin[i]; //temp;
  }
  
    
  //decrypt mode, data in mode
  DES_CON = 0x04 ; //0000_0100
  
  //Insert data input
  for (i=0;i<8;i++){
     //temp = datain[i]; 
     DES_IN = datain[i]; //temp;
  }
  
  //decrypt mode, data in mode, start decryption
  DES_CON = 0x05 ; //0000_0101
  
  //Wait output ready
  while ((DES_CON & 0x01)== 0x00){};


  //Fetch data output
  for (i=0;i<8;i++){
     //temp = DES_OUT;	
     dataout[i] = DES_OUT;//temp;
  }	
//Disable DES Clock
CLKCON &= 0xF7;
  
}

void xstsm212_tdes_encrypt(unsigned char* datain, unsigned char* keyin, unsigned char* dataout)
{  
   unsigned char i;	
   //unsigned char idata temp;
   
//Enable DES Clock
CLKCON |= 0x08;    																		

  //encrypt mode, key in mode
  DES_CON = 0x02 ; //0000_0010     

  //Insert key
  for (i=0;i<8;i++){
     //temp = keyin[i];
     DES_IN = keyin[i];//temp;
  }	

  //encrypt mode, data in mode
  DES_CON = 0x00 ; //0000_0000        

  //Insert data input
  for (i=0;i<8;i++){ 
     DES_IN =  datain[i];//temp;
  }

  //encrypt mode, data in mode, start encryption
  DES_CON = 0x01 ; //0000_0001    

  //Wait output ready
  while ((DES_CON & 0x01)== 0x00){}; 


  //Fetch data output
  for (i=0;i<8;i++){
     //temp = DES_OUT;	
     dataout[i] = DES_OUT;	//temp;
  }
//Disable DES Clock
CLKCON &= 0xF7;
   
}

void xstsm212_tdes16_encrypt(unsigned char* datain, unsigned char* keyin, unsigned char* dataout)
{
  unsigned char data_temp [8];

  xstsm212_tdes_encrypt(datain, keyin, data_temp);
  xstsm212_tdes_decrypt(data_temp, keyin+8, data_temp);
  xstsm212_tdes_encrypt(data_temp, keyin, dataout);
}



void tdes16_encrypt_rx(unsigned char *data_init, unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen)
{
    unsigned char data_temp [8];
    unsigned char i; 
    unsigned char j;

    xstsm212_tdes16_encrypt(datain, keyin, data_temp);

	for(i=0; i<8; i++)
	  dataout[i] = data_init[i] ^ data_temp[i];

	j=0;
	while((j+8) < dinlen)
	{
	  xstsm212_tdes16_encrypt(datain+8+j, keyin, data_temp);

	  for(i=0; i<8; i++)
	    dataout[i+j+8] = datain[i+j] ^ data_temp[i];

	  j+= 8;
	} 
}

void xstsm212_tdes16_encrypt_rx(unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen)
{
  unsigned char data_temp [8];
  unsigned char i; 
  unsigned char j;

//  if (dinlen == 8)
//  {
//
//    xstsm212_tdes16_encrypt(datain, keyin, dataout);
//
//  }
//  else if (dinlen == 16)
//  {
//
//    xstsm212_tdes16_encrypt(datain, keyin, dataout);
//
//	xstsm212_tdes16_encrypt(datain+8, keyin, data_temp);
//	
//	for(i=0; i<8; i++)
//	  dataout[i+8] = datain[i] ^ data_temp[i];
//  }
//  else
//  {
    xstsm212_tdes16_encrypt(datain, keyin, dataout);
	j=0;
	while((j+8) < dinlen)
	{
	  xstsm212_tdes16_encrypt(datain+8+j, keyin, data_temp);

	  for(i=0; i<8; i++)
	    dataout[i+j+8] = datain[i+j] ^ data_temp[i];

	  j+= 8;
	}  
//  }

}

void tdes16_encrypt_tx(unsigned char *data_init, unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen)
{
    unsigned char data_temp [8];
    unsigned char i;			 
    unsigned char j;

    for(i=0; i<8; i++)
	  data_temp[i] = data_init[i] ^ datain[i];

    xstsm212_tdes16_encrypt(data_temp, keyin, dataout);
	
	j = 0;

	while((j+8) < dinlen)
	{
	  for(i=0; i<8; i++)
	    data_temp[i] = dataout[i+j] ^ datain[8+i+j];

	  xstsm212_tdes16_encrypt(data_temp, keyin, dataout+8+j);
	  j+= 8;
	}

}

void xstsm212_tdes16_encrypt_tx(unsigned char* datain, unsigned char* keyin, unsigned char* dataout, unsigned char dinlen)
{
  unsigned char data_temp [8];
  unsigned char i;			 
  unsigned char j;

//  if (dinlen == 8)
//  {
//    xstsm212_tdes16_encrypt(datain, keyin, dataout);	 
//  }
//  else if (dinlen == 16)
//  {
//
//    xstsm212_tdes16_encrypt(datain, keyin, dataout);
//
//	for(i=0; i<8; i++)
//	  data_temp[i] = dataout[i] ^ datain[8+i];
//
//	xstsm212_tdes16_encrypt(data_temp, keyin, dataout+8);
//   
//  }
//  else
//  {
    xstsm212_tdes16_encrypt(datain, keyin, dataout);
	j=0;
	while((j+8) < dinlen)
	{
	  for(i=0; i<8; i++)
	    data_temp[i] = dataout[i+j] ^ datain[8+i+j];

	  xstsm212_tdes16_encrypt(data_temp, keyin, dataout+8+j);
	  j+= 8;
	}  
//  }	   

}
