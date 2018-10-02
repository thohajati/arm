#ifndef _FLASH_H_
#define _FLASH_H_

#define SECTOR_SIZE 512
#define ADDR_MIN 0x00006000

void flash_sector_erase(unsigned char* addr);
void flash_sector_backup(unsigned char* addr);
void flash_write(unsigned char* sector_addr, unsigned short addr_offset, unsigned char* data_buffer, unsigned short length);
//void flash_clear(unsigned char xdata *sector_addr, unsigned short addr_offset, int length);
void flash_write_byte(unsigned char *addr, unsigned char value);
void flash_format(unsigned char *addr, unsigned short len);
void flash_byte_set(unsigned char* addr, unsigned char value);

#endif
