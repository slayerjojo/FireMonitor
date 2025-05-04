#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>

void sensor_init(void);
void sensor_update(void);

extern volatile uint8_t _sensor_sampling;

#endif
