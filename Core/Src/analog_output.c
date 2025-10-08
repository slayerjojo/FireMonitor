#include "analog_output.h"
#include "main.h"
#include "timer.h"
#include "eeprom.h"
#include "sensor.h"
#include "tempareture.h"

extern SPI_HandleTypeDef hspi2;

static uint8_t _mode = 1;
static uint8_t _source = 0;
static uint16_t _output = 0;

void analog_output_init(void)
{
    eeprom_read(EEPROM_SETTINGS_AO_MODE, &_mode, 1);
    if (0xff == _mode)
    {
        _mode = AO_MODE_UV;
    }
    eeprom_read(EEPROM_SETTINGS_AO_SOURCE, &_source, 1);
    if (0xff == _source)
    {
        _source = AO_SOURCE_QUALITY;
    }
    _mode = AO_MODE_UV;
    _source = AO_SOURCE_INTENSITY;
}

void analog_output_update(void)
{
    if (AO_SOURCE_QUALITY == _source)
    {
        if (_mode == AO_MODE_UV)
        {
            analog_output_set(sensor_quality_get(0, function_set_get(FUNCTION_SET_AB)));
        }
        else if (_mode == AO_MODE_IR)
        {
            analog_output_set(sensor_quality_get(1, function_set_get(FUNCTION_SET_CD)));
        }
    }
    else if (AO_SOURCE_INTENSITY == _source)
    {
        if (_mode == AO_MODE_UV)
        {
            analog_output_set(sensor_intensity_get(0, function_set_get(FUNCTION_SET_AB)->intensity.filter));
        }
        else if (_mode == AO_MODE_IR)
        {
            analog_output_set(sensor_intensity_get(1, function_set_get(FUNCTION_SET_CD)->intensity.filter));
        }
    }
    else if (AO_SOURCE_FREQ == _source)
    {
        if (_mode == AO_MODE_UV)
        {
            struct function_set *f = function_set_get(FUNCTION_SET_AB);
            analog_output_set(sensor_flicker_frequency_get(0, f->frequency.filter, f->frequency.sensitivity, f->frequency.max));
        }
        else if (_mode == AO_MODE_IR)
        {
            struct function_set *f = function_set_get(FUNCTION_SET_CD);
            analog_output_set(sensor_flicker_frequency_get(1, f->frequency.filter, f->frequency.sensitivity, f->frequency.max));
        }
    }
    else if (AO_SOURCE_AMP == _source)
    {
        if (_mode == AO_MODE_UV)
        {
            analog_output_set(sensor_amplitude_get(0, function_set_get(FUNCTION_SET_AB)->amplitude.filter));
        }
        else if (_mode == AO_MODE_IR)
        {
            analog_output_set(sensor_amplitude_get(1, function_set_get(FUNCTION_SET_CD)->amplitude.filter));
        }
    }
    else if (AO_SOURCE_CORE_TEMPERATURE == _source)
    {
        analog_output_set(core_tempareture_get());
    }
    else
    {
        analog_output_set(0);
    }
}

void analog_output_set(uint8_t v)
{
    //x * 5 / 18.7 * 100 = (20 - 4) * v / 100 + 4
    //x = 187 * ((20 - 4) * v / 100 + 4) / 4120
    //x = 187 * ((20 - 4) * v + 400) / 412000
    
    uint32_t value = 0;
    if (v)
    {
        value = 4095 * 187 * (v * (20 - 4) + 400);
        value /= 500000;
    }
    _output = value;

    //SEGGER_RTT_printf(0, "ao:%u\n", _output);

    uint8_t data[2] = {0};
    data[0] = _output >> 8;
    data[1] = _output;
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, data, 2, 1000);
    timer_delay(10);
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}

uint8_t analog_output_mode_get(void)
{
    return _mode;
}

void analog_output_mode_set(uint8_t mode)
{
    _mode = mode;
    eeprom_write(EEPROM_SETTINGS_AO_MODE, &_mode, 1);
}

uint8_t analog_output_source_get(void)
{
    return _source;
}

void analog_output_source_set(uint8_t source)
{
    _source = source;
    eeprom_write(EEPROM_SETTINGS_AO_SOURCE, &_source, 1);
}
