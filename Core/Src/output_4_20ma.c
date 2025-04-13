#include "output_4_20ma.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

static uint8_t _type = 0;
static uint16_t _output = 0;

static void output(uint16_t v)
{
    _output = v;

    uint8_t data[2] = {0};
    data[0] = _output >> 8;
    data[1] = _output;
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, data, 2, 1000);
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
    
    SEGGER_RTT_printf(0, "4-20ma output:%u\n", v);
}

void output_4_20ma_init(void)
{
    output(2000);
}

void output_4_20ma_update(void)
{
}

void output_4_20ma_intensity_set(uint16_t v)
{
    if (0 == _type)
        output(v);
}

void output_4_20ma_freq_set(uint16_t v)
{
    if (1 == _type)
        output(v);
}

void output_4_20ma_amplitude_set(uint16_t v)
{
    if (2 == _type)
        output(v);
}

void output_4_20ma_quality_set(uint16_t v)
{
    if (3 == _type)
        output(v);
}

void output_4_20ma_type_set(uint8_t type)
{
    _type = type;
}

uint16_t output_4_20ma_get_percent(void)
{
    return 100 * _output / 4095;
}
