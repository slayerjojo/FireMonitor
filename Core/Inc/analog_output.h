#ifndef __ANALOG_OUTPUT_H__
#define __ANALOG_OUTPUT_H__

#include <stdint.h>

enum {
    AO_MODE_AUTO = 0,
    AO_MODE_UV,
    AO_MODE_IR,
};

enum {
    AO_SOURCE_QUALITY = 0,
    AO_SOURCE_INTENSITY,
    AO_SOURCE_FREQ,
    AO_SOURCE_AMP,
    AO_SOURCE_CORE_TEMPERATURE,
    AO_SOURCE_FLAME_TEMPERATURE,
};

void analog_output_init(void);
void analog_output_update(void);

void analog_output_set(uint8_t v);
uint8_t analog_output_mode_get(void);
void analog_output_mode_set(uint8_t mode);
uint8_t analog_output_source_get(void);
void analog_output_source_set(uint8_t source);

#endif
