#include "key.h"
#include "main.h"
#include "timer.h"

enum {
    KEY_UP = 0,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    MAX_KEY,
};

enum {
    KEY_EVENT_CLICK = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
};

enum {
    MENU_IDLE = 0,
    MENU_COMM,
    MENU_MAIN_COMM,
}_menu = MENU_IDLE;

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

static void key_event(uint8_t key, uint8_t action)
{
    if (MENU_IDLE == _menu)
    {
        if (KEY_LEFT == key && action == KEY_EVENT_PRESS)
        {
            _menu = MENU_COMM;
        }
    }
    else if (MENU_COMM == _menu)
    {
        if (KEY_RIGHT == key)
        {
            _menu = MENU_MAIN_COMM;
        }
    }
}

void key_init(void)
{
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
                if (_keys[key].cnt < 100)
                {
                    _keys[key].cnt++;
                }
                else if (100 == _keys[key].cnt)
                {
                    _keys[key].cnt++;
                    key_event(key, KEY_EVENT_PRESS);
                }
            }
            else if (_keys[key].cnt < 100)
            {
                _keys[key].cnt = 0;
                key_event(key, KEY_EVENT_CLICK);
            }
            else if (_keys[key].cnt >= 100)
            {
                _keys[key].cnt = 0;
                key_event(key, KEY_EVENT_RELEASE);
            }
        }
    }
}

