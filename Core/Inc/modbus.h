#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdint.h>

void modbus_init(void);
void modbus_recv(uint8_t *protocol, int size);
uint8_t modbus_address_get(uint8_t idx);
uint8_t modbus_address_temp_get(uint8_t idx);
void modbus_address_ts(uint8_t idx, uint8_t address);
void modbus_address_apply(void);

#endif
