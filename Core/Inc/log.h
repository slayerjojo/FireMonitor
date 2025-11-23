#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdarg.h>

#define LOG_ERR(fmt, ...) log_print(3, _log_level, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) log_print(2, _log_level, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INF(fmt, ...) log_print(1, _log_level, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...) log_print(0, _log_level, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERR_RAW(fmt, ...) log_raw_print(3, _log_level, fmt, ##__VA_ARGS__)
#define LOG_WRN_RAW(fmt, ...) log_raw_print(2, _log_level, fmt, ##__VA_ARGS__)
#define LOG_INF_RAW(fmt, ...) log_raw_print(1, _log_level, fmt, ##__VA_ARGS__)
#define LOG_DBG_RAW(fmt, ...) log_raw_print(0, _log_level, fmt, ##__VA_ARGS__)
#define LOG_ERR_HEX(data, size, fmt, ...) log_hex_print(3, _log_level, __FUNCTION__, __LINE__, data, size, fmt, ##__VA_ARGS__)
#define LOG_WRN_HEX(data, size, fmt, ...) log_hex_print(2, _log_level, __FUNCTION__, __LINE__, data, size, fmt, ##__VA_ARGS__)
#define LOG_INF_HEX(data, size, fmt, ...) log_hex_print(1, _log_level, __FUNCTION__, __LINE__, data, size, fmt, ##__VA_ARGS__)
#define LOG_DBG_HEX(data, size, fmt, ...) log_hex_print(0, _log_level, __FUNCTION__, __LINE__, data, size, fmt, ##__VA_ARGS__)

void log_init(void);
void log_update(void);
void log_append(uint8_t opcode, uint16_t param);
uint8_t log_opcode_read(uint32_t offset);
uint32_t log_timestamp_read(uint32_t offset);
uint16_t log_param_read(uint32_t offset);
void log_print(uint8_t action, uint8_t level, const char *func, uint16_t line, const char *fmt, ...);
void log_raw_print(uint8_t action, uint8_t level, const char *fmt, ...);
void log_hex_print(uint8_t action, uint8_t level, const char *func, uint16_t line, uint8_t *data, uint16_t size, const char *fmt, ...);

#endif
