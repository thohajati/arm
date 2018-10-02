#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "vuiso14443.h"

#define MAX_APP_NUM 4
#define MAX_FILE_NUM 16

//Status and Error Codes
#define OPERATION_OK               0x00
#define NO_CHANGES                 0x0C	
#define OUT_OF_EEPROM_ERROR        0x0E
#define ILLEGAL_COMMAND_CODE       0x1C
#define INTEGRITY_ERROR            0x1E
#define NO_SUCH_KEY                0x40
#define LENGTH_ERROR               0x7E
#define PERMISSION_DENIED          0x9D
#define PARAMETER_ERROR            0x9E
#define APPLICATION_NOT_FOUND      0xA0
#define APPL_INTEGRITY_ERROR       0xA1
#define AUTHENTICATION_ERROR       0xAE
#define ADDITIONAL_FRAME           0xAF
#define BOUNDARY_ERROR             0xBE
#define PICC_INTEGRITY_ERROR       0xC1
#define COMMAND_ABORTED            0xCA
#define PICC_DISABLED_ERROR        0xCD
#define COUNT_ERROR                0xCE	
#define DUPLICATE_ERROR            0xDE
#define EEPROM_ERROR               0xEE
#define FILE_NOT_FOUND             0xF0
#define FILE_INTEGRITY_ERROR       0xF1

#define DESFIRE_SW1                0x95 

void master_key_format(void);
void firmware_response(ISOAPDU * papdu_command);	
void firmware_iso7816_response(ISOAPDU * papdu_command);
void firmware_xirka_response(ISOAPDU * papdu_command);

void delay (void);
void delay_2 (void);
void delay_3 (void);

#endif
