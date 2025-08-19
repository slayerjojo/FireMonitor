#ifndef __OPERATING_MODE_H__
#define __OPERATING_MODE_H__

#include <stdint.h>

void operating_mode_init(void);
uint8_t operating_mode_temp_get(void);
void operating_mode_ts(uint8_t mode);
uint8_t operating_mode_get(void);
void operating_mode_set(uint8_t mode);
void operating_mode_default_load(void);
void operating_mode_configure_apply(void);

#endif
