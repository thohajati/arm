#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define GETFIRMWAREVERSION  0x01

//Security Related Commands
#define AUTHENTICATE_DES    0x0A
#define CHANGE_KEY_SETTING  0x54
#define GET_KEY_SETTING     0x45
#define CHANGE_KEY          0xC4
#define GET_KEY_VERSION     0x64
#define AUTHENTICATE_AES 	0xAA

//PICC Level Commands
#define CREATE_APPLICATION  0xCA
#define DELETE_APPLICATION  0xDA
#define GET_APPLICATION_ID  0x6A
#define SELECT_APPLICATION  0x5A
#define FORMAT_PICC         0xFC
#define GET_VERSION         0x60

//Application Level Commands
#define GET_FILE_ID                0x6F
#define GET_FILE_SETTING           0xF5
#define CHANGE_FILE_SETTING        0x5F
#define CREATE_STD_DATA_FILE       0xCD
#define CREATE_BACKUP_DATA_FILE    0xCB
#define CREATE_VALUE_FILE          0xCC
#define CREATE_LINEAR_RECORD_FILE  0xC1
#define CREATE_CYCLIC_RECORD_FILE  0xC0
#define DELETE_FILE                0xDF

//Data Manipulation Commands
#define READ_DATA                  0xBD
#define WRITE_DATA                 0x3D
#define GET_VALUE                  0x6C
#define CREDIT                     0x0C
#define DEBIT                      0xDC
#define LIMITED_CREDIT             0x1C
#define WRITE_RECORD               0x3B
#define READ_RECORD                0xBB
#define CLEAR_RECORD_FILE          0xEB  
#define COMMIT_TRANSACTION         0xC7
#define ABORT_TRANSACTION          0xA7

// ISO 7816 command
#define ISO_SELECT   0xA4
#define ISO_READ     0xB0
#define ISO_WRITE    0xD6

#endif
