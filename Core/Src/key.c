#include "key.h"
#include "digit.h"
#include "main.h"
#include "timer.h"
#include "usart.h"
#include "function_set.h"

struct {
    uint8_t cnt;
    GPIO_TypeDef *gpio;
    uint16_t pin;
}_keys[MAX_KEY] = {
    {
        .cnt = 0,
        .gpio = KEY_UP_GPIO_Port,
        .pin = KEY_UP_Pin,
    },
    {
        .cnt = 0,
        .gpio = KEY_DOWN_GPIO_Port,
        .pin = KEY_DOWN_Pin,
    },
    {
        .cnt = 0,
        .gpio = KEY_LEFT_GPIO_Port,
        .pin = KEY_LEFT_Pin,
    },
    {
        .cnt = 0,
        .gpio = KEY_RIGHT_GPIO_Port,
        .pin = KEY_RIGHT_Pin,
    },
};

static uint32_t _timer_key = 1;
static KEY_HANDLER _handler = 0;

void key_init(void)
{
    SEGGER_RTT_printf(0, "key initialized\n");
    _handler = 0;
}

void key_handler_set(KEY_HANDLER handler)
{
    _handler = handler;
}

void key_update(void)
{
    if (timer_diff(_timer_key) > 100)
    {
        _timer_key = timer_start();

        for (uint8_t key = 0; key < MAX_KEY; key++)
        {
            if (!HAL_GPIO_ReadPin(_keys[key].gpio, _keys[key].pin))
            {
                if (_keys[key].cnt == 3)
                {
                    _keys[key].cnt++;
                    if (_handler)
                        _handler(key, KEY_EVENT_PRESS);
                }
                else if (_keys[key].cnt < 100)
                {
                    _keys[key].cnt++;
                }
                else if (100 == _keys[key].cnt)
                {
                    _keys[key].cnt++;
                    SEGGER_RTT_printf(0, "key %u press\n", key);
                    if (_handler)
                        _handler(key, KEY_EVENT_PRESS_LONG);
                }
            }
            else if (_keys[key].cnt && _keys[key].cnt < 100)
            {
                _keys[key].cnt = 0;
                if (_handler)
                {
                    SEGGER_RTT_printf(0, "key %u click\n", key);
                    _handler(key, KEY_EVENT_CLICK);
                }
            }
            else if (_keys[key].cnt >= 100)
            {
                _keys[key].cnt = 0;
                SEGGER_RTT_printf(0, "key %u release\n", key);
                if (_handler)
                    _handler(key, KEY_EVENT_RELEASE);
            }
        }
    }
}

