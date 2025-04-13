#include "tempareture.h"
#include "main.h"
#include "timer.h"
#include "error.h"

extern I2C_HandleTypeDef hi2c1;

static uint32_t _timer = 1;
static int16_t _max = 70;
static int16_t _min = -60;
static int16_t _temp = 0;

void tempareture_init(void)
{
    SEGGER_RTT_printf(0, "tempareture initialized\n");
}

void tempareture_update(void)
{
    if (timer_diff(_timer) > 125)
    {
        _timer = timer_start();

        uint8_t data[2] = {0};
        int ret = HAL_I2C_Mem_Read(&hi2c1, 0x90, 0, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
        if (HAL_OK != ret)
        {
			SEGGER_RTT_printf(0, "tempareture error:%d\n", ret);
            return;
        }

        int16_t temp = ((uint16_t)data[0] << 8) + data[1];
        temp = temp >> 7;
        if (temp >= 0x100)
        {
            temp = (~temp + 1) & 0xff;
            temp = -temp;
        }
        _temp = temp;
        //SEGGER_RTT_printf(0, "tempareture:%d\n", temp);

        if (temp >= _max * 2)
            error_set(ERROR_TEMPERATURE_OVERFLOW);
        if (temp <= _min * 2)
            error_set(ERROR_TEMPERATURE_UNDERFLOW);
    }
}

void tempareture_minmax_set(int16_t max, int16_t min)
{
    _max = max;
    _min = min;
}

int16_t tempareture_get(void)
{
    return _temp / 2;
}
