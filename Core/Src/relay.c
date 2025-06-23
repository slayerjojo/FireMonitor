#include "relay.h"
#include "timer.h"
#include "error.h"
#include "sensor.h"
#include "eeprom.h"
#include "led.h"
#include "tempareture.h"
#include "function_set.h"

static uint8_t _flame_relay_overridden = 0;
static uint8_t _safe_relay_usage = 0;
static uint8_t _safe_relay_usage_temp = 0;
static uint16_t _safe_relay_temperature_threshold = 800;
static struct flame_status 
{
    uint8_t status;
    uint32_t timer;
}_flame_status[2] = {0};
static struct relay_status
{
    uint8_t status;
    uint16_t count;
    uint32_t timer;
}_relay_status[2] = {0};

void relay_init(void)
{
    SEGGER_RTT_printf(0, "relay initialized\n");

    eeprom_read(EEPROM_SETTINGS_RELAY_OVERRIDDEN, &_flame_relay_overridden, 1);
    eeprom_read(EEPROM_SETTINGS_RELAY_SAFE_USAGE, &_safe_relay_usage, 1);
    eeprom_read(EEPROM_SETTINGS_RELAY_SAFE_TEMPERATURE_THRESHOLD, &_safe_relay_temperature_threshold, 2);
    eeprom_read(EEPROM_SETTINGS_RELAY_COUNT, &_relay_status[0].count, 2);
    eeprom_read(EEPROM_SETTINGS_RELAY_COUNT + 2, &_relay_status[1].count, 2);
    if (0xff == _flame_relay_overridden)
    {
        _flame_relay_overridden = 0;
        _safe_relay_usage = SAFE_RELAY_USAGE_SAFE;
        _safe_relay_temperature_threshold = 800;
        _relay_status[0].count = _relay_status[1].count = 0;
    }
    _safe_relay_usage_temp = _safe_relay_usage;

    if (SAFE_RELAY_USAGE_SAFE == _safe_relay_usage)
    {
        safe_relay_set(is_error(0xff));
    }
}

void relay_update(void)
{
    if (SAFE_RELAY_USAGE_SAFE == _safe_relay_usage)
    {
        safe_relay_set(is_error(0xff));
    }
    if (SAFE_RELAY_USAGE_SECOND_FLAME == _safe_relay_usage)
    {
        struct function_set *f = function_set_get(FUNCTION_SET_CD);
        struct flame_status *fs = &_flame_status[1];
        if (fs->status)
        {
            if (!function_set_flame_status_get(0, f, fs->status))
            {
                if (!fs->timer)
                {
                    fs->timer = timer_start();
                }
                else if (timer_diff(fs->timer) > f->delay.drop_out)
                {
                    fs->timer = 0;
                    fs->status = 0;
                }
            }
            else
            {
                fs->timer = 0;
            }
        }
        else
        {
            if (function_set_flame_status_get(0, f, fs->status))
            {
                if (!fs->timer)
                {
                    fs->timer = timer_start();
                }
                else if (timer_diff(fs->timer) > f->delay.pull_in)
                {
                    fs->timer = 0;
                    fs->status = 1;
                }
            }
            else
            {
                fs->timer = 0;
            }
        }
        safe_relay_set(_flame_relay_overridden ? _flame_relay_overridden : fs->status);
    }
    if (SAFE_RELAY_USAGE_QUALITY == _safe_relay_usage)
    {
        struct function_set *f = function_set_get(FUNCTION_SET_CD);
        safe_relay_set(sensor_quality_get(0, f) >= f->quality_threshold);
    }
    if (SAFE_RELAY_USAGE_CORE_TEMPERATURE == _safe_relay_usage)
    {
        safe_relay_set(core_tempareture_get() > CORE_TEMPERATURE_THRESHOLD);
    }
    {
        struct function_set *f = function_set_get(FUNCTION_SET_AB);
        struct flame_status *fs = &_flame_status[0];
        if (fs->status)
        {
            if (!function_set_flame_status_get(0, f, fs->status))
            {
                if (!fs->timer)
                {
                    fs->timer = timer_start();
                }
                else if (timer_diff(fs->timer) > f->delay.drop_out)
                {
                    fs->timer = 0;
                    fs->status = 0;
                }
            }
            else
            {
                fs->timer = 0;
            }
        }
        else
        {
            if (function_set_flame_status_get(0, f, fs->status))
            {
                if (!fs->timer)
                {
                    fs->timer = timer_start();
                }
                else if (timer_diff(fs->timer) > f->delay.pull_in)
                {
                    fs->timer = 0;
                    fs->status = 1;
                }
            }
            else
            {
                fs->timer = 0;
            }
        }
        flame_relay_set(_flame_relay_overridden ? _flame_relay_overridden : fs->status);
    }
    if (_relay_status[0].timer && timer_diff(_relay_status[0].timer) > 125)
    {
        _relay_status[0].timer = timer_start();

        if (flame_relay_feedback() != _relay_status[0].status)
        {
            error_set(ERROR_FLAME_RELAY_FAILURE);
        }
        else
        {
            error_clear(ERROR_FLAME_RELAY_FAILURE);
        }
    }
    if (_relay_status[1].timer && timer_diff(_relay_status[1].timer) > 125)
    {
        _relay_status[1].timer = timer_start();

        if (flame_relay_feedback() != _relay_status[1].status)
        {
            error_set(ERROR_SAFE_RELAY_FAILURE);
        }
        else
        {
            error_clear(ERROR_SAFE_RELAY_FAILURE);
        }
    }
}

uint8_t relay_flame_status_get(uint8_t id)
{
    return _relay_status[id].status;
}

uint16_t safe_relay_temperature_threshold_get(void)
{
    return _safe_relay_temperature_threshold;
}

void safe_relay_temperature_threshold_set(uint16_t v)
{
    _safe_relay_temperature_threshold = v;
    eeprom_write(EEPROM_SETTINGS_RELAY_SAFE_TEMPERATURE_THRESHOLD, &_safe_relay_temperature_threshold, 2);
}

uint8_t flame_relay_feedback(void)
{
    return !!HAL_GPIO_ReadPin(RELAY_FLAME_FB_GPIO_Port, RELAY_FLAME_FB_Pin);
}

void flame_relay_set(uint8_t v)
{
    if (v)
    {
        HAL_GPIO_WritePin(RELAY_FLAME_GPIO_Port, RELAY_FLAME_Pin, GPIO_PIN_SET);
        led_set(LED_2_RED, 0, 1);
        if (_flame_relay_overridden)
        {
            led_set(LED_2_GREEN, 0xffff, 0xffff);
        }
        else
        {
            led_set(LED_2_GREEN, 0xaaaa, 0xffff);
        }
    }
    else
    {
        HAL_GPIO_WritePin(RELAY_FLAME_GPIO_Port, RELAY_FLAME_Pin, GPIO_PIN_RESET);
        led_set(LED_2_RED, 0xffff, 0xffff);
        led_set(LED_2_GREEN, 0, 1);
    }
    if (v != _relay_status[0].status)
    {
        _relay_status[0].timer = timer_start();
        _relay_status[0].count++;
    }
    _relay_status[0].status = v;
}

void flame_relay_overridden_set(uint8_t overridden)
{
    _flame_relay_overridden = overridden;
    eeprom_write(EEPROM_SETTINGS_RELAY_OVERRIDDEN, &_flame_relay_overridden, 1);
}

uint8_t flame_relay_overridden(void)
{
    return _flame_relay_overridden;
}

void safe_relay_set(uint8_t v)
{
    if (v)
    {
        HAL_GPIO_WritePin(RELAY_SAFE_GPIO_Port, RELAY_SAFE_Pin, GPIO_PIN_SET);
        led_set(LED_1_RED, 0xffff, 0xffff);
        led_set(LED_1_GREEN, 0, 1);
    }
    else
    {
        HAL_GPIO_WritePin(RELAY_SAFE_GPIO_Port, RELAY_SAFE_Pin, GPIO_PIN_RESET);
        led_set(LED_1_RED, 0, 1);
        led_set(LED_1_GREEN, 0xffff, 0xffff);
    }
    if (v != _relay_status[1].status)
    {
        _relay_status[1].timer = timer_start();
        _relay_status[1].count++;
    }
    _relay_status[1].status = v;
}

uint8_t safe_relay_feedback(void)
{
    return !!HAL_GPIO_ReadPin(RELAY_SAFE_FB_GPIO_Port, RELAY_SAFE_FB_Pin);
}

uint8_t safe_relay_usage(void)
{
    return _safe_relay_usage;
}

void safe_relay_usage_set(uint8_t usage)
{
    _safe_relay_usage = usage;
    eeprom_write(EEPROM_SETTINGS_RELAY_SAFE_USAGE, &_safe_relay_usage, 1);
}

uint8_t safe_relay_usage_temp_get(void)
{
    return _safe_relay_usage_temp;
}

void safe_relay_usage_ts(uint8_t usage)
{
    _safe_relay_usage_temp = usage;
}

uint16_t relay_action_count(uint8_t relay)
{
    return _relay_status[relay].count;
}

void relay_clear_counters(void)
{
    _relay_status[0].count = _relay_status[1].count = 0;
    eeprom_write(EEPROM_SETTINGS_RELAY_COUNT, &_relay_status[0].count, 2);
    eeprom_write(EEPROM_SETTINGS_RELAY_COUNT + 2, &_relay_status[1].count, 2);
}

void relay_default_load(void)
{
    _safe_relay_usage = SAFE_RELAY_USAGE_SAFE;
    _safe_relay_usage_temp = _safe_relay_usage;
    eeprom_write(EEPROM_SETTINGS_RELAY_SAFE_USAGE, &_safe_relay_usage, 1);
}

void relay_configure_apply(void)
{
    _safe_relay_usage = _safe_relay_usage_temp;
    eeprom_write(EEPROM_SETTINGS_RELAY_SAFE_USAGE, &_safe_relay_usage, 1);
}
