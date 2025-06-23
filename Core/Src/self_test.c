#include "self_test.h"
#include "main.h"
#include "eeprom.h"
#include "flash.h"
#include "led.h"
#include "key.h"
#include "digit.h"
#include "timer.h"
#include "relay.h"
#include "sensor.h"
#include "analog_output.h"
#include "tempareture.h"
#include "function_set.h"
#include <stdio.h>

static enum 
{
    STATE_SELFTEST_IDLE = 0,
    STATE_SELFTEST_START,
    STATE_SELFTEST_EEPROM,
    STATE_SELFTEST_FLASH,
    STATE_SELFTEST_AO,
    STATE_SELFTEST_SENSOR,
    STATE_SELFTEST_TEMPERATURE,
}_state = STATE_SELFTEST_IDLE;

uint32_t _timer = 0;

static void key_handler(uint8_t key, uint8_t action)
{
    if (0 == key)
    {
        led_set(LED_1_RED, 0xaa, 0xff);
    }
    else if (1 == key)
    {
        led_set(LED_1_RED, 0, 0xff);
    }
    else if (2 == key)
    {
        led_set(LED_1_GREEN, 0xaa, 0xff);
    }
    else if (3 == key)
    {
        led_set(LED_1_GREEN, 0, 0xff);
    }
}

void self_test_init(void)
{
    _state = STATE_SELFTEST_IDLE;
}

void self_test_update(void)
{
    if (STATE_SELFTEST_IDLE == _state)
    {
    }
    else
    {
        if (function_set_dinput(0))
        {
            flame_relay_set(1);
        }
        else
        {
            flame_relay_set(0);
        }
        if (function_set_dinput(1))
        {
            safe_relay_set(1);
        }
        else
        {
            safe_relay_set(0);
        }
        if (flame_relay_feedback())
        {
            led_set(LED_2_RED, 0xaa, 0xff);
        }
        else
        {
            led_set(LED_2_RED, 0, 0xff);
        }
        if (safe_relay_feedback())
        {
            led_set(LED_2_GREEN, 0xaa, 0xff);
        }
        else
        {
            led_set(LED_2_GREEN, 0, 0xff);
        }
        if (STATE_SELFTEST_START == _state)
        {
            _state = STATE_SELFTEST_EEPROM;
            _timer = 0;
            key_handler_set(key_handler);
        }
        else if (STATE_SELFTEST_EEPROM == _state)
        {
            if (!_timer || timer_diff(_timer) > 3000)
            {
                digit_set("ST.E");
                _timer = timer_start();
                uint8_t data = _timer;
                eeprom_write(0, &data, 1);
                uint8_t check = data + 1;
                eeprom_read(0, &check, 1);
                if (check == data)
                {
                    _state = STATE_SELFTEST_FLASH;
                    _timer = 0;
                }
            }
        }
        else if (STATE_SELFTEST_FLASH == _state)
        {
            if (!_timer || timer_diff(_timer) > 3000)
            {
                digit_set("ST.F");
                _timer = timer_start();
                flash_sector_erase(0);
                uint8_t data = _timer;
                flash_write(0, &data, 1);
                uint8_t check = data + 1;
                flash_read(0, &check, 1);
                if (check == data)
                {
                    _state = STATE_SELFTEST_AO;
                    _timer = 0;
                }
            }
        }
        else if (STATE_SELFTEST_AO == _state)
        {
            if (!_timer)
            {
                _timer = timer_start();
                digit_set("ST.A");
                analog_output_set(50);
            }
            if (timer_diff(_timer) > 10000)
            {
                _state = STATE_SELFTEST_SENSOR;
                _timer = 0;
                return;
            }
        }
        else if (STATE_SELFTEST_SENSOR == _state)
        {
            if (!_timer)
            {
                _timer = timer_start();

                uint16_t v = sensor_intensity_get(0, 0);
                char buffer[8] = {0};
                sprintf(buffer, "%3u.", v);
                digit_set(buffer);
            }
            if (timer_diff(_timer) > 2000)
            {
                _state = STATE_SELFTEST_TEMPERATURE;
                _timer = 0;
                return;
            }
        }
        else if (STATE_SELFTEST_TEMPERATURE == _state)
        {
            if (!_timer)
            {
                _timer = timer_start();

                uint16_t v = core_tempareture_get();
                char buffer[8] = {0};
                sprintf(buffer, "%3u", v);
                digit_set(buffer);
            }
            if (timer_diff(_timer) > 2000)
            {
                _state = STATE_SELFTEST_SENSOR;
                _timer = 0;
                return;
            }
        }
    }
}

void self_test_start(void)
{
    _state = STATE_SELFTEST_START;
}
