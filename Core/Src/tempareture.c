#include "tempareture.h"
#include "main.h"
#include "timer.h"
#include "error.h"

extern I2C_HandleTypeDef hi2c1;

static uint32_t _timer = 1;
static int16_t _max = 70;
static int16_t _min = -60;

void tempareture_init(void)
{
}

void tempareture_update(void)
{
    if (timer_diff(_timer) > 125)
    {
        _timer = timer_start();

        uint8_t data[2] = {0};
        if (HAL_OK != HAL_I2C_Mem_Read(&hi2c1, 0, 0, I2C_MEMADD_SIZE_8BIT, data, 2, 1000))
        {
            return;
        }

        int16_t temp = ((uint16_t)data[0] << 8) + data[1];

        if (temp >= _max * 2)
            error_set(ERROR_TEMPERATURE_OVERFLOW);
        if (temp <= _min * 2)
            error_set(ERROR_TEMPERATURE_UNDERFLOW);
    }
}

void tempareture_set(int16_t max, int16_t min)
{
    _max = max;
    _min = min;
}
