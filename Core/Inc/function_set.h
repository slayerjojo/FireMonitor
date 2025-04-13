#ifndef __FUNCTION_SET_H__
#define __FUNCTION_SET_H__

#include <stdint.h>

void function_set_init(void);
void function_set_update(void);

uint8_t function_set_get(void);

uint8_t function_set_temp_get(uint8_t functor, uint8_t attribute, uint16_t *min, uint16_t *max);
void function_set_temp_set(uint8_t functor, uint8_t attribute, uint8_t value);
void function_set_temp_default(void);
void function_set_apply(void);

#endif
