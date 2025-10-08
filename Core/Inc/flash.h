#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

void flash_init(void);
uint16_t flash_device_id(void);
void flash_uid(uint8_t *uid);
void flash_write(uint32_t addr, const void *buffer, uint16_t size);
void flash_sector_erase(uint32_t addr);
void flash_read(uint32_t addr, void *buffer, uint16_t size);

#endif
