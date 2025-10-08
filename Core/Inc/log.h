#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>

void log_init(void);
void log_update(void);
void log_append(uint8_t opcode, uint16_t param);
uint8_t log_opcode_read(uint32_t offset);
uint32_t log_timestamp_read(uint32_t offset);
uint16_t log_param_read(uint32_t offset);

#endif
