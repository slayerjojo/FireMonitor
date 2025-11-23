#include "tempareture.h"
#include "main.h"
#include "timer.h"
#include "error.h"

static const uint8_t _log_level = 1;

extern I2C_HandleTypeDef hi2c1;

static uint32_t _timer = 1;
static int16_t _max = 85;
static int16_t _min = -60;
static int16_t _temp = 0;

void tempareture_init(void)
{
    LOG_INF("tempareture initialized");
}

void tempareture_update(void)
{
    if (timer_diff(_timer) < 125)
    {
        return;
    }
    _timer = timer_start();

    uint8_t data[2] = {0};
    int ret = HAL_I2C_Mem_Read(&hi2c1, 0x90, 0, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
    if (HAL_OK != ret)
    {
        LOG_ERR("tempareture error:%d", ret);
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
    LOG_DBG("tempareture:%d", _temp * 10 / 2);

    if (temp >= _max * 2)
        error_set(ERROR_OVER_TEMPERATURE);
    if (temp <= _min * 2)
        error_set(ERROR_OVER_TEMPERATURE);
}

int16_t core_tempareture_get(void)
{
    return _temp * 10 / 2;
}
