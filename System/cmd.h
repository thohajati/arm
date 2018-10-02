#ifndef _CMD_H_
#define _CMD_H_

#include "vutype.h"
#include "firmware.h"

typedef struct { 
  uint8_t  file_type;
  uint8_t  file_ID;
  uint8_t  ISO_file_ID[2];
  uint8_t  com_setting;
  uint8_t  access_right[2];
  uint8_t  file_lenght[3]; 
  uint16_t file_addr;
  //uint8_t  temp[16]; 

}file;

typedef struct {

  uint8_t AID[3];
  uint8_t key_setting[2];
  uint8_t file_ID_eq[2];
  uint8_t tag_ID[7];
  file app_file[MAX_FILE_NUM];

}application;

typedef struct { 
  unsigned char file_type;
  unsigned char file_ID;
  unsigned char com_setting;
  unsigned char access_right[2];
  unsigned char file_lenght[3];
  unsigned short file_addr;	  
  unsigned short file_mirror_addr;

}backup_file;

typedef struct { 
  unsigned char file_type;
  unsigned char file_ID;
  unsigned char com_setting;
  unsigned char access_right[2];
  unsigned char record_size[3];	
  unsigned char max_record_num[3];
  unsigned short file_addr;	
  unsigned short file_mirror_addr;
  unsigned char current_record[3];	  
  unsigned char mirror_record;

}record_file;

typedef struct { 
  unsigned char file_type;
  unsigned char file_ID;
  unsigned char com_setting;
  unsigned char access_right[2];
  unsigned char lower_limit[4];	
  unsigned char upper_limit[4];	
  unsigned char value[4];
  unsigned char limited_credit_enable;
  unsigned short file_addr;		
  unsigned char mirror_value[4]; 	
  unsigned char limited_credit_value[4];

}value_file;

typedef struct {

  unsigned char file_ID;
  unsigned char offset[3];
  unsigned char length[3];

}desfire_read_data;

typedef struct {

  unsigned char file_ID;
  unsigned char offset[3];
  unsigned char length[3];
  unsigned char data_to_write[60];

}desfire_write_data;

//file type
#define STD_DATA_FILE              0x00
#define BACKUP_DATA_FILE           0x01
#define VALUE_FILE                 0x02
#define LINEAR_RECORD_FILE         0x03
#define CYCLIC_RECORD_FILE         0x04

extern application app[MAX_APP_NUM];        //at(0x6000)
#define app_buff                  ((uint8_t*)0x00006000) //0x00012000)mode contact hang when reading address 0x6000 in cmd creat app (flash_sector_backup()))
#define master_key                ((uint8_t*)0x00008000)
#define UID                       ((uint8_t*)0x00008e00)
//uint8_t UID[7] =  {0x04, 0x2d, 0x4f, 0x21, 0xbd, 0x25, 0x80};
#define master_key_setting        ((uint8_t*)0x00008F20)
#define key_buff                  ((uint8_t*)0x00009000)
#define file_buff                 ((uint8_t*)0x0000A000)

#define SIZE_APP_BUFF             0x2000
#define SIZE_KEY_BUFF             0x1000
#define SIZE_FILE_BUFF            0x8000
#define SIZE_MASTER_KEY_BUFF      0x11


extern command_status cmd_status;
extern uint8_t randB[8];
extern uint8_t key_idx;
extern uint8_t app_idx;
extern uint8_t file_idx;
extern uint8_t sesion_key[16];
extern uint16_t real_data_length; 
extern uint8_t read_all_flag; 
//extern uint8_t code UID[7];
extern unsigned int init_vector[4];


void format_picc(ISOAPDU * papdu_command); 
void get_app(void);		  
void get_ver(void);
void get_file_id(void);	  
void get_key_set(void);
void auth_des(ISOAPDU* papdu_command);
void sel_app(ISOAPDU* papdu_command);
void create_app(ISOAPDU* papdu_command);
void delete_app(ISOAPDU* papdu_command); 
void create_file(ISOAPDU* papdu_command);
void get_file_set(ISOAPDU* papdu_command);
void write_data(ISOAPDU* papdu_command); 
void read_data(ISOAPDU* papdu_command);
void change_file_set(ISOAPDU* papdu_command);
void change_key(ISOAPDU* papdu_command);
void get_key_ver(ISOAPDU* papdu_command);
void iso_select(ISOAPDU* papdu_command);
void iso_write(ISOAPDU* papdu_command);
void iso_read(ISOAPDU* papdu_command);
void authenticate_aes(ISOAPDU* papdu_command);
void iso_14443_aes_write_data(ISOAPDU * papdu_command);
void iso_14443_aes_read_data(ISOAPDU * papdu_command, uint8_t* preaddata, uint16_t data_length);
void iso_14443_change_key_aes_random(ISOAPDU * papdu_command);
void write_process(ISOAPDU * papdu_command);
void read_process(ISOAPDU * papdu_command);

#endif
