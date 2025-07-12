#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>

#include "function_set.h"

void sensor_init(void);
void sensor_update(void);

uint8_t sensor_type_get(void);
uint16_t sensor_intensity_raw_get(uint8_t sensor);
uint16_t sensor_intensity_get(uint8_t sensor, uint16_t smoothing);
uint16_t sensor_amplitude_get(uint8_t sensor, uint16_t smoothing);
uint16_t sensor_flicker_frequency_get(uint8_t sensor, uint16_t smoothing, uint16_t sensitivity, uint16_t max);
uint16_t sensor_quality_get(uint8_t sensor, struct function_set *f);
void sensor_sample_pause(uint8_t action);

#endif
