#include "sensor.h"

void sensor_init(void)
{
    SEGGER_RTT_printf(0, "sensor initialized\n");
}

void sensor_update(void)
{
}

uint16_t sensor_intensity_get_percent(void)
{
    return 0;
}

uint16_t sensor_frequency_get_percent(void)
{
    return 0;
}

uint16_t sensor_amplitude_get_percent(void)
{
    return 0;
}

uint16_t sensor_quality_get_percent(void)
{
    return 0;
}
