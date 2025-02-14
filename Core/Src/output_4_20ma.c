#include "output_4_20ma.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

static uint8_t _type = 0;

void output_4_20ma_init(void)
{
}

void output_4_20ma_update(void)
{
}

static void output(uint16_t v)
{
    uint8_t data[2] = {0};
    data[0] = v >> 8;
    data[1] = v;
    HAL_SPI_Transmit(&hspi2, data, 2, 1000);
}

void output_4_20ma_set_intensity(uint16_t v)
{
    if (0 == _type)
        output(v);
}

void output_4_20ma_set_freq(uint16_t v)
{
    if (1 == _type)
        output(v);
}

void output_4_20ma_set_amplitude(uint16_t v)
{
    if (2 == _type)
        output(v);
}

void output_4_20ma_set_quality(uint16_t v)
{
    if (3 == _type)
        output(v);
}

void output_4_20ma_set(uint8_t type)
{
    _type = type;
}
