#include "error.h"
#include "main.h"

static uint32_t _error = 0;

void error_set(uint8_t error)
{
    _error |= 1ul << error;
}

void error_clear(uint8_t error)
{
    _error &= ~(1ul << error);
}

uint8_t is_error(uint8_t type)
{
    if (FAULT_TYPE_SCANNER == type)
    {
        return !!(_error & (1 << ERROR_FLAME_RELAY_FAILURE | 1 << ERROR_SAFE_RELAY_FAILURE | 1 << ERROR_OVER_TEMPERATURE));
    }
    return 0;
}
