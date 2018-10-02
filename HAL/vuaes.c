#include "vuaes.h"
//#include "vumacro.h"
#include "reg.h"
#include "auth_aes.h"

//#define AES_MODE_START      0x04 // start data encrypt/decrypt
#define AES_MODE_KEYEXPAND  0x10 // start key expanding
#define AES_MODE_KEY_INS    0x02 // key inserted
#define AES_MODE_DATA_INS   0x00 // data inserted

#define AES_SIZE_128            0x00
#define AES_SIZE_192            0x08
#define AES_SIZE_256            0x10


#define AESKEY_LEN  4

static unsigned int  s_aestemp[4];

/* const RB dan const Zero digunakan saat proses penghitungan AES CMAC */
static uint8_t const_rb[16] = { 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87 
};     
        


void xor_16byte_array(unsigned int* result, unsigned int* a, unsigned int* b)
{    
    result[0]   = a[0]  ^ b[0];     
    result[1]   = a[1]  ^ b[1];     
    result[2]   = a[2]  ^ b[2];     
    result[3]   = a[3]  ^ b[3];     
    
}

static void
XOR_16BYTEARRAY(unsigned char* result,unsigned char* a, unsigned char* b){
	unsigned char i;
	for(i=0;i<16;i++){
		result[i] = a[i]^b[i];
	} 

    
}
       
        
void AES_CRYPT(unsigned int* out,unsigned int* in)
{
	
	//Input array
	AES_DATA_0 = in[3];
	AES_DATA_1 = in[2];
	AES_DATA_2 = in[1];
	AES_DATA_3 = in[0];

  while ((AES_CON & 0x01)== 0x00); 
	
  //Output array
	out[3] = AES_DATA_0;
	out[2] = AES_DATA_1;
	out[1] = AES_DATA_2;
	out[0] = AES_DATA_3;
	
}


void
vuaes_insertkey16byte(unsigned int * pkey){
	
	
    // Insert key 
    AES_KEY_0 = pkey[3];
	  AES_KEY_1 = pkey[2];
	  AES_KEY_2 = pkey[1];
	  AES_KEY_3 = pkey[0];
		
    /* Wait key ready */
    while ((AES_CON & 0x02)== 0x00);

}

static 
void memcpy_16_byte(unsigned int* dest, unsigned int* src)    
{
    dest[0x00] = src[0x00]; 
    dest[0x01] = src[0x01]; 
    dest[0x02] = src[0x02]; 
    dest[0x03] = src[0x03];          
    
}

void MEMCPY_16BYTE(uint8_t* dest,uint8_t* src){
            dest[0x00] = src[0x00];  
            dest[0x01] = src[0x01]; 
            dest[0x02] = src[0x02]; 
            dest[0x03] = src[0x03]; 
            dest[0x04] = src[0x04]; 
            dest[0x05] = src[0x05]; 
            dest[0x06] = src[0x06]; 
            dest[0x07] = src[0x07]; 
            dest[0x08] = src[0x08]; 
            dest[0x09] = src[0x09]; 
            dest[0x0A] = src[0x0A]; 
            dest[0x0B] = src[0x0B]; 
            dest[0x0C] = src[0x0C]; 
            dest[0x0D] = src[0x0D]; 
            dest[0x0E] = src[0x0E]; 
            dest[0x0F] = src[0x0F];
}           




void 
vuaes_process(PINPUTAES pinp){
    uint16_t block;
    unsigned int* pivbuff;
    block = pinp->bufferlen/4;
    pivbuff = pinp->piv;
	
		//Enable AES Clock
		CLKCON |= 0x10;
	
		if(pinp->mode == AES_MODE_DECRYPT){
				AES_CON = AES_MODE_DECRYPT | AES_SIZE_128;
		}else{
				AES_CON = AES_MODE_ENCRYPT | AES_SIZE_128;
		}
		
		vuaes_insertkey16byte(pinp->pkey);
		
    while(block >0){
        if(pinp->mode == AES_MODE_DECRYPT){
            AES_CRYPT(s_aestemp,pinp->pbuffer);
            xor_16byte_array(pinp->presult,pivbuff,s_aestemp);
            pivbuff = pinp->pbuffer;

        }else{
            xor_16byte_array(s_aestemp,pivbuff,pinp->pbuffer);
            AES_CRYPT(pinp->presult,s_aestemp);
            pivbuff = pinp->presult;
        }
        pinp->presult+=4;
        pinp->pbuffer +=4;            
        block--;
    }
		AES_CON = 0x00;
		CLKCON &= 0xEF;  //Disable AES Clock

}



static void 
cmac_leftshift1bit(uint8_t * input, uint8_t * output){
    uint8_t idx;
    uint8_t overflow = 0;
    idx=16;
    while(idx){
        idx-=1;
        output[idx] = input[idx] << 1;
        output[idx] |= overflow;
        overflow = ((input[idx] & 0x80) !=0) ? 1:0;        
    }
    

}

static void 
cmac_gen_subkey(uint8_t * key,uint8_t * k1,uint8_t * k2){
    static uint8_t  input[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    uint8_t  ciphered[16];
    uint8_t  temp[16];
		unsigned int keyint[4];
		unsigned int inputint[4];
    unsigned int cipheredint[4];				
		
		memcpy8to32(keyint,key,16);
	  memcpy8to32(inputint,input,16);
				
    //AES_INSERTKEY(keyint);
		vuaes_insertkey16byte(keyint);
    AES_CRYPT(cipheredint,inputint);
				
		memcpy32to8(ciphered,cipheredint,4);		
    if(( ciphered[0] & 0x80 ) == 0) { /* MSB(L) = 0 then K1 = L << 1 */
        cmac_leftshift1bit(ciphered,k1);
    }else{ /* K1 = (L<<1) ^ Rb */
        cmac_leftshift1bit(ciphered,temp);
        XOR_16BYTEARRAY(k1,temp,const_rb);
    }

    if((k1[0] & 0x80) == 0){
        cmac_leftshift1bit(k1,k2);
    }else{
        cmac_leftshift1bit(k1,temp);
        XOR_16BYTEARRAY(k2,temp,const_rb);
    }


}

/*
    example:
    input : C0 FF EE
    output: C0 FF EE 80 00 00 00 00 .... 00
*/
static void
cmac_padding(uint8_t * input,uint8_t * padding,uint8_t inputlen){
    uint8_t cnt;
    /* memcpy */
    for(cnt=0;cnt<inputlen;cnt++){
        padding[cnt] = input[cnt];
    }
    padding[inputlen] = 0x80;
    /* memset di memory sisanya */
    for(cnt = 0;cnt<(15-inputlen);++cnt){
        padding[cnt+inputlen+1] = 0x00;
    }
}

void 
vuaes_cmac(PINPUTAES pinput){
    uint8_t  subkey_k1[16];
    uint8_t  subkey_k2[16];
		uint8_t  key[16];
    uint8_t  m_last[16];
    uint8_t  X[16];
    uint8_t  Y[16];
	  unsigned int  X32[4];
    unsigned int  Y32[4];
    uint8_t  padded[16];
    uint8_t numround;
    uint8_t idx;
    uint8_t flagfragment = 0;
	  uint8_t buffer[64];
	  uint8_t bufferlen;
	  uint8_t initvector[16];
			
	//Enable AES Clock
		CLKCON |= 0x10;
	AES_CON = AES_MODE_ENCRYPT | AES_SIZE_128;
	
	/*only 2 options: 1 or 8 bytes (see veriu security doc)*/
	  bufferlen = (pinput->bufferlen == 1) ? 1 : pinput->bufferlen * 4;
	
		memcpy32to8(key,pinput->pkey,4);
    
    /* generate 2 subkey */
    cmac_gen_subkey(key,subkey_k1,subkey_k2);
    /* Tentukan jumlah putaran proses per blok data ( 16 byte ) */
    numround = (bufferlen + 15 ) / 16;
    if(numround == 0){
        numround = 1;
        flagfragment = 1;
    }else{ /* tentukan apakah butuh padding (16 byte) atau tidak */
        flagfragment = ((bufferlen % 16) != 0 ) ? 1 : 0;
    }
    /* jika panjang data input genap kelipatan 16 byte (tidak terfragment)
       maka blok terakhir pada data input di-XOR dengan subkey k1, 
       hasil dari xor ini akan digunakan pada akhir putaran AES CMAC
     */
		memcpy32to8(buffer,pinput->pbuffer,4);
    if(!flagfragment){
        XOR_16BYTEARRAY(m_last,(&buffer[16*(numround-1)]),subkey_k1);
    }else{				
        cmac_padding(&buffer[16*(numround-1)],padded,bufferlen%16);
        XOR_16BYTEARRAY(m_last,padded,subkey_k2);
    }

    /* Inisialisasi X dengan nilai IV */
		memcpy32to8(initvector,pinput->piv,4);
    MEMCPY_16BYTE(X,initvector);
    /* lakukan loop sebanyak putaran 'numround' */
    for(idx=0;idx<numround-1;idx++){
        XOR_16BYTEARRAY(Y,((uint8_t *)&pinput->pbuffer[16*idx]),X);
        vuaes_insertkey16byte(pinput->pkey);
			  memcpy8to32(X32,X,16);
			  memcpy8to32(Y32,Y,16);
        AES_CRYPT(X32,Y32); /* X = AES128(Y,key) */
    }

    /* final process dengan blok terakhir */
		XOR_16BYTEARRAY(Y,X,m_last);
		memcpy8to32(X32,X,16);
		memcpy8to32(Y32,Y,16);
    AES_CRYPT(X32,Y32);
    /* now put it all to the output */
    memcpy_16_byte(pinput->presult,X32);
		
		AES_CON = 0x00;
		CLKCON &= 0xEF;  //Disable AES Clock

}




