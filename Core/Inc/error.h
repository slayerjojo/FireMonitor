#ifndef __ERROR_H__
#define __ERROR_H__

#include "main.h"

enum {
    ERROR_FLAME_RELAY_FAILURE,
    ERROR_SAFE_RELAY_FAILURE,
    ERROR_OVER_TEMPERATURE,
};

enum {
    FAULT_TYPE_COMBINED_UNIT = 0,
    FAULT_TYPE_SCANNER,
    FAULT_TYPE_SENSOR_0,
    FAULT_TYPE_SENSOR_1,
};

void error_set(uint8_t error);
void error_clear(uint8_t error);
uint8_t is_error(uint8_t type);

#endif
