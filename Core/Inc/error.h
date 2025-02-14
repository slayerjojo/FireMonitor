#ifndef __ERROR_H__
#define __ERROR_H__

#include "main.h"

enum {
    ERROR_RELAY_FRAME_PICKUP = 0,
    ERROR_RELAY_FRAME_DROPOUT,
    ERROR_TEMPERATURE_OVERFLOW,
    ERROR_TEMPERATURE_UNDERFLOW,
};

void error_set(uint8_t error);

#endif
