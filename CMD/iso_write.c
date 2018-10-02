#include "flash.h"
#include "firmware.h"
#include "reg.h"
#include "XSIM_hard_Api.h"
#include "system_statusword.h"	
#include "vutdes.h"	
#include "cmd.h"

void iso_write(ISOAPDU * papdu_command)
{
	uint16_t file_addr, data_offset;

	file_addr = app[app_idx].app_file[file_idx].file_addr;
	data_offset = papdu_command->pheader->P1 <<8 | papdu_command->pheader->P2;	
	data_offset += file_addr;

	flash_write(file_buff, data_offset, papdu_command->papdudata, papdu_command->pheader->LEN);

	iso14443sendresp(SW_OK);	
}
