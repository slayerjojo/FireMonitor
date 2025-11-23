#include "analog_output.h"
#include "main.h"
#include "timer.h"
#include "eeprom.h"
#include "sensor.h"
#include "tempareture.h"
#include "function_set.h"

static const uint8_t _log_level = 1;

extern SPI_HandleTypeDef hspi2;

static uint8_t _mode = AO_MODE_UV;
static uint8_t _source = 0;
static uint8_t _source_temp = 0;
static uint8_t _output = 0;

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
    _source_temp = _source;
}

void analog_output_update(void)
{
    uint8_t sensor = 1;
    uint8_t fs = FUNCTION_SET_CD;
    if (_mode == AO_MODE_UV)
    {
        sensor = 0;
        fs = FUNCTION_SET_AB;
    }
    if (AO_SOURCE_QUALITY == _source)
    {
        analog_output_set(sensor_quality_get(sensor, fs));
    }
    else if (AO_SOURCE_INTENSITY == _source)
    {
        analog_output_set(sensor_intensity_get(sensor, fs));
    }
    else if (AO_SOURCE_FREQ == _source)
    {
        analog_output_set(sensor_flicker_frequency_get(sensor, fs));
    }
    else if (AO_SOURCE_AMP == _source)
    {
        analog_output_set(sensor_amplitude_get(sensor, fs));
    }
    else if (AO_SOURCE_CORE_TEMPERATURE == _source)
    {
        int16_t t = core_tempareture_get();
        if (t < 0)
        {
            t = 0;
        }
        if (t > 850)
        {
            t = 850;
        }

        analog_output_set(100 * t / 850);
    }
    else
    {
        analog_output_set(0);
    }
}

uint8_t analog_output_get(void)
{
    return _output;
}

void analog_output_set(uint8_t v)
{
    //x * 5 / 18.7 * 100 = (20 - 4) * v / 100 + 4
    //x = 187 * ((20 - 4) * v / 100 + 4) / 5000
    //x = 187 * ((20 - 4) * v + 400) / 500000
  
    _output = v;
    uint32_t value = 0;
    if (_output)
    {
        value = 4095 * 187 * (_output * (20 - 4) + 400);
        value /= 500000;
    }

    LOG_DBG("ao:%u %u", _output, value);

    uint8_t data[2] = {0};
    data[0] = value >> 8;
    data[1] = value;
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, data, 2, 1000);
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

uint8_t analog_output_source_temp_get(void)
{
    return _source_temp;
}

void analog_output_source_temp_set(uint8_t source)
{
    _source_temp = source;
}

void analog_output_default_load(void)
{
    _mode = AO_MODE_UV;
    _source_temp = _source = AO_SOURCE_QUALITY;
    eeprom_write(EEPROM_SETTINGS_AO_MODE, &_mode, 1);
    eeprom_write(EEPROM_SETTINGS_AO_SOURCE, &_source, 1);
}

void analog_output_configure_apply(void)
{
    _source = _source_temp;
    eeprom_write(EEPROM_SETTINGS_AO_SOURCE, &_source, 1);
}
