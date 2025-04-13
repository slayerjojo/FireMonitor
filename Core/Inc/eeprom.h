#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

void eeprom_write(uint16_t addr, const uint8_t *data, uint16_t size);
void eeprom_read(uint16_t addr, uint8_t *data, uint16_t size);

#endif
