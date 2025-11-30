#include "menu.h"
#include "log.h"
#include "key.h"
#include "digit.h"
#include "sensor.h"
#include "timer.h"
#include "relay.h"
#include "operating_mode.h"
#include "analog_output.h"
#include "sensor.h"
#include "usart.h"
#include "modbus.h"
#include "tempareture.h"
#include <stdio.h>

static const uint8_t _log_level = 1;

typedef struct {
    enum {
        MENU_IDLE = 0,
        MENU_IDLE_LOOP,
        MENU_IDLE_FWVER,
        MENU_AC_AMPLITUDE,
        MENU_AC_AMPLITUDE_VALUE,
        MENU_AC_AMPLITUDE_VALUE_TS,
        MENU_ANALOG_OUTPUT_DISPLAY,
        MENU_ANALOG_OUTPUT_DISPLAY_SET,
        MENU_ANALOG_OUTPUT_DISPLAY_TS,
        MENU_COMM_PROTOCOL,
        MENU_COMM_PROTOCOL_ADDR,
        MENU_COMM_PROTOCOL_ADDR_TS,
        MENU_COMM_PROTOCOL_ADDR_VALUE,
        MENU_COMM_PROTOCOL_BAUD,
        MENU_COMM_PROTOCOL_BAUD_TS,
        MENU_COMM_PROTOCOL_BAUD_VALUE,
        MENU_COMM_RECORD,
        MENU_COMM_STORAGE,
        MENU_CONFIGURE_DEFAULT,
        MENU_CONFIGURE_DEFAULT_SET,
        MENU_CONFIGURE_RECORD,
        MENU_CONFIGURE_RECORD_STORAGE,
        MENU_CONFIGURE_STORAGE,
        MENU_FUNCTION_SET,
        MENU_FUNCTION_SET_ACTIVE,
        MENU_FUNCTION_SET_ATTRIBUTE,
        MENU_FUNCTION_SET_ATTRIBUTE_TS,
        MENU_FUNCTION_SET_ATTRIBUTE_VALUE,
        MENU_FUNCTION_SET_SWITCH,
        MENU_FUNCTION_SET_SWITCH_TS,
        MENU_FUNCTION_SET_SWITCH_VALUE,
        MENU_HIGH_FREQ_LIMIT,
        MENU_HIGH_FREQ_LIMIT_TS,
        MENU_HIGH_FREQ_LIMIT_VALUE,
        MENU_OPERATING_MODE,
        MENU_OPERATING_MODE_TS,
        MENU_OPERATING_MODE_VALUE,
        MENU_SAFE_RELAY_USAGE,
        MENU_SAFE_RELAY_USAGE_TS,
        MENU_SAFE_RELAY_USAGE_VALUE,
    }type;
    uint32_t kb_idle;
    union {
        struct {
            uint8_t protocol:1;
            uint8_t addr:1;
            uint8_t save:2;
            uint8_t baud:2;
            uint8_t temp;
			uint8_t addrs[2];
            uint32_t timer;
        }comm;
        struct {
            uint8_t attribute;
            uint8_t functor;
            uint8_t value;
            uint16_t max_value;
            uint16_t min_value;
            uint32_t timer;
        }function_set;
        struct {
            uint8_t value;
            uint32_t timer;
        }operating_mode;
        struct {
            uint8_t value;
            uint32_t timer;
        }safe_relay_usage;
        struct {
            uint8_t value;
            uint32_t timer;
        }ao;
        struct {
            uint8_t value;
            uint32_t timer;
        }configure;
        struct {
            uint8_t attribute;
            uint8_t phase;
            uint32_t timer;
        }idle;
    };
}Menu;

static Menu _menu = {
    .type = MENU_IDLE_FWVER,
};

static const uint32_t _bauds[] = {9600, 19200, 38400, 115200};

static const uint16_t _attributes[] = {0, 1, 3, 5, 6, 7, 9, 11, 12, 13, 15, 17, 21, 20, 19};

static uint8_t _ao_display_mode_temp = 0;

static void key_event(uint8_t key, uint8_t action)
{
    log_append(0x02, key << 8 | action);

    LOG_DBG("key:%u action:%u", key, action);
    _menu.kb_idle = timer_start();

    if (MENU_IDLE_LOOP == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_PRESS_LONG == action)
        {
            _menu.comm.timer = 0;
            _menu.comm.protocol = COMM_PROTOCOL_MODBUS;
            _menu.type = MENU_COMM_PROTOCOL;
        }
        if (KEY_RIGHT == key && KEY_EVENT_PRESS_LONG == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ACTIVE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.idle.attribute > 0)
                _menu.idle.attribute--;
            _menu.idle.phase = 0;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.idle.attribute < 6)
                _menu.idle.attribute++;
            _menu.idle.phase = 0;
        }
    }
    else if (MENU_FUNCTION_SET_ACTIVE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.function_set.functor = 0;
            _menu.function_set.attribute = 0;
            _menu.function_set.value = 0;
            _menu.function_set.min_value = 0;
            _menu.function_set.max_value = 0;
            _menu.type = MENU_FUNCTION_SET;
        }
    }
    else if (MENU_FUNCTION_SET == _menu.type)
    {
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.functor > 0)
            {
                _menu.function_set.functor--;
                _menu.function_set.timer = 0;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.functor < 3)
            {
                _menu.function_set.functor++;
                _menu.function_set.timer = 0;
            }
            else
            {
                _menu.operating_mode.timer = 0;
                _menu.type = MENU_OPERATING_MODE;
            }
        }
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.attribute = 0;
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE;
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.value = function_set_temp_get(_menu.function_set.functor, _attributes[_menu.function_set.attribute], &_menu.function_set.min_value, &_menu.function_set.max_value);
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.attribute > 0)
            {
                _menu.function_set.attribute--;
                _menu.function_set.timer = 0;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.attribute < 14)
            {
                _menu.function_set.attribute++;
                _menu.function_set.timer = 0;
            }
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            function_set_attribute_ts(_menu.function_set.functor, _attributes[_menu.function_set.attribute], _menu.function_set.value);
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value < _menu.function_set.max_value)
            {
                _menu.function_set.value++;
                _menu.function_set.timer = 0;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value > _menu.function_set.min_value)
            {
                _menu.function_set.value--;
                _menu.function_set.timer = 0;
            }
        }
    }
    else if (MENU_OPERATING_MODE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.operating_mode.value = operating_mode_temp_get();
            _menu.operating_mode.timer = 0;
            _menu.type = MENU_OPERATING_MODE_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_SWITCH;
        }
    }
    else if (MENU_OPERATING_MODE_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.operating_mode.timer = 0;
            _menu.type = MENU_OPERATING_MODE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.operating_mode.timer = 0;
            operating_mode_ts(_menu.operating_mode.value);
            _menu.type = MENU_OPERATING_MODE_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.operating_mode.value > 0)
            {
                _menu.operating_mode.timer = 0;
                _menu.operating_mode.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.operating_mode.value < 3)
            {
                _menu.operating_mode.timer = 0;
                _menu.operating_mode.value++;
            }
        }
    }
    else if (MENU_FUNCTION_SET_SWITCH == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.value = function_set_switch_temp_get();
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_SWITCH_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_HIGH_FREQ_LIMIT;
        }
    }
    else if (MENU_FUNCTION_SET_SWITCH_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_SWITCH;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            function_set_switch_ts(_menu.function_set.value);
            _menu.type = MENU_FUNCTION_SET_SWITCH_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value > 0)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value < 2)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value++;
            }
        }
    }
    else if (MENU_HIGH_FREQ_LIMIT == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.value = function_set_enable_high_limit_temp_get();
            _menu.function_set.timer = 0;
            _menu.type = MENU_HIGH_FREQ_LIMIT_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_SWITCH;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_AC_AMPLITUDE;
        }
    }
    else if (MENU_HIGH_FREQ_LIMIT_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_HIGH_FREQ_LIMIT;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            function_set_enable_high_limit_ts(_menu.function_set.value);
            _menu.type = MENU_HIGH_FREQ_LIMIT_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value > 0)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value < 1)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value++;
            }
        }
    }
    else if (MENU_AC_AMPLITUDE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.value = function_set_enable_high_limit_temp_get();
            _menu.function_set.timer = 0;
            _menu.type = MENU_AC_AMPLITUDE_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_HIGH_FREQ_LIMIT;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.safe_relay_usage.timer = 0;
            _menu.type = MENU_SAFE_RELAY_USAGE;
        }
    }
    else if (MENU_AC_AMPLITUDE_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_AC_AMPLITUDE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            function_set_enable_ac_amplitude_ts(_menu.function_set.value);
            _menu.type = MENU_AC_AMPLITUDE_VALUE_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value > 0)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.function_set.value < 1)
            {
                _menu.function_set.timer = 0;
                _menu.function_set.value++;
            }
        }
    }
    else if (MENU_SAFE_RELAY_USAGE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.safe_relay_usage.value = safe_relay_usage_temp_get();
            _menu.safe_relay_usage.timer = 0;
            _menu.type = MENU_SAFE_RELAY_USAGE_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_AC_AMPLITUDE;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.ao.timer = 0;
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY;
        }
    }
    else if (MENU_SAFE_RELAY_USAGE_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.safe_relay_usage.timer = 0;
            _menu.type = MENU_SAFE_RELAY_USAGE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.safe_relay_usage.timer = 0;
            safe_relay_usage_ts(_menu.safe_relay_usage.value);
            _menu.type = MENU_SAFE_RELAY_USAGE_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.safe_relay_usage.value > 0)
            {
                _menu.safe_relay_usage.timer = 0;
                _menu.safe_relay_usage.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.safe_relay_usage.value < 3)
            {
                _menu.safe_relay_usage.timer = 0;
                _menu.safe_relay_usage.value++;
            }
        }
    }
    else if (MENU_ANALOG_OUTPUT_DISPLAY == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.ao.value = analog_output_source_temp_get();
            _menu.ao.timer = 0;
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY_SET;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.safe_relay_usage.timer = 0;
            _menu.type = MENU_SAFE_RELAY_USAGE;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_DEFAULT;
        }
    }
    else if (MENU_ANALOG_OUTPUT_DISPLAY_SET == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.ao.timer = 0;
            analog_output_source_temp_set(_menu.ao.value);
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.ao.value > 0)
            {
                _menu.ao.timer = 0;
                _menu.ao.value--;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.ao.value < 5)
            {
                _menu.ao.timer = 0;
                _menu.ao.value++;
            }
        }
    }
    else if (MENU_CONFIGURE_DEFAULT == _menu.type)
    {
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.ao.timer = 0;
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_RECORD;
        }
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
           _menu.configure.timer = 0; 
           _menu.configure.value = 0;
           _menu.type = MENU_CONFIGURE_DEFAULT_SET;
        }
    }
    else if (MENU_CONFIGURE_DEFAULT_SET == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_DEFAULT;
        }
        if (KEY_UP == key)
        {
            if (KEY_EVENT_PRESS == action)
            {
                _menu.configure.value |= 1;
            }
            else if (KEY_EVENT_RELEASE == action)
            {
                _menu.configure.value &= ~1;
            }
        }
        if (KEY_DOWN == key)
        {
            if (KEY_EVENT_PRESS == action)
            {
                _menu.configure.value |= 2;
            }
            else if (KEY_EVENT_RELEASE == action)
            {
                _menu.configure.value &= ~2;
            }
        }
        if ((_menu.configure.value & 0x03) == 0x03)
        {
            _menu.configure.value = 0;
            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_STORAGE;
        }
    }
    else if (MENU_CONFIGURE_RECORD == _menu.type)
    {
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_DEFAULT;
        }
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
           _menu.configure.timer = 0;
           _menu.type = MENU_CONFIGURE_RECORD_STORAGE;
        }
    }
    else if (MENU_COMM_PROTOCOL == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.addr = 0;
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (COMM_PROTOCOL_MODBUS == _menu.comm.protocol)
            {
                _menu.comm.timer = 0;
                _menu.type = MENU_COMM_RECORD;
            }
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.temp = modbus_address_temp_get(_menu.comm.addr);
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (1 == _menu.comm.addr)
            {
                _menu.comm.addr = 0;
                _menu.comm.timer = 0;
            }
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (0 == _menu.comm.addr)
            {
                _menu.comm.addr = 1;
                _menu.comm.timer = 0;
            }
            else if (1 == _menu.comm.addr)
            {
                _menu.comm.timer = 0;
                _menu.type = MENU_COMM_PROTOCOL_BAUD;
            }
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.save |= 1 << 0;
            if (COMM_PROTOCOL_MODBUS == _menu.comm.protocol)
            {
                modbus_address_ts(_menu.comm.addr, _menu.comm.temp);
            }
            else if (COMM_PROTOCOL_PROFIBUS == _menu.comm.protocol)
            {
                LOG_ERR("profibus not support");
            }
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.comm.temp < 254)
            {
                _menu.comm.temp++;
            }
            _menu.comm.timer = 0;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.comm.temp > 1)
            {
                _menu.comm.temp--;
            }
            _menu.comm.timer = 0;
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            uint32_t baud = usart_comm_baud_temp();
            _menu.comm.baud = 0;
            for (int i = 0; i < sizeof(_bauds) / sizeof(_bauds[0]); i++)
            {
                if (_bauds[i] == usart_comm_baud_temp())
                {
                    _menu.comm.baud = i;
                    break;
                }
            }
            _menu.type = MENU_COMM_PROTOCOL_BAUD_VALUE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.addr = 1;
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD_VALUE == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_BAUD;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.save |= 1 << 1;
            usart_comm_baud_ts(_bauds[_menu.comm.baud]);
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_BAUD_TS;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.baud++;
            _menu.comm.timer = 0;
        }
        if (KEY_DOWN == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.baud--;
            _menu.comm.timer = 0;
        }
    }
    else if (MENU_COMM_RECORD == _menu.type)
    {
        if (KEY_LEFT == key && KEY_EVENT_CLICK == action)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key && KEY_EVENT_CLICK == action)
        {
            if (_menu.comm.save & (1 << 0))
            {
                if (COMM_PROTOCOL_MODBUS == usart_comm_protocol())
                {
                    modbus_address_apply();
                }
            }
            if (_menu.comm.save & (1 << 1))
            {
                usart_comm_baud_apply();
            }
            _menu.comm.save = 0;
            
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_STORAGE;
        }
        if (KEY_UP == key && KEY_EVENT_CLICK == action)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL;
        }
    }
}

void menu_init(void)
{
    LOG_INF("menu initialized");

    key_handler_set(key_event);
}

void menu_update(void)
{
    if (MENU_IDLE_FWVER == _menu.type)
    {
        digit_set(FW_VERSION);
        _menu.type = MENU_IDLE;
    }
    else if (MENU_IDLE == _menu.type)
    {
        _menu.idle.attribute = 0;
        _menu.idle.phase = 0;
        _menu.idle.timer = timer_start();
        _menu.type = MENU_IDLE_LOOP;
    }
    else if (MENU_IDLE_LOOP == _menu.type)
    {
        if (_menu.idle.timer && timer_diff(_menu.idle.timer) < 500)
        {
            return;
        }
        _menu.idle.timer = timer_start();

        if (!(_menu.idle.phase++ % 10))
        {
            const char *labels[] = {
                "Ao.",
                "CtE",
                "1-S",
                "1-I",
                "1-F",
                "1-A",
                "1-Q",
            };
            digit_set(labels[_menu.idle.attribute]);
        }
        else
        {
            char codes[4] = {0};
            if (0 == _menu.idle.attribute)
            {
                sprintf(codes, "%3u", analog_output_get());
            }
            else if (1 == _menu.idle.attribute)
            {
                sprintf(codes, "%3d", core_tempareture_get() / 10);
            }
            else if (2 == _menu.idle.attribute)
            {
                sprintf(codes, " UV");
            }
            else if (3 == _menu.idle.attribute)
            {
                sprintf(codes, "%3u", sensor_intensity_get(0, FUNCTION_SET_AB));
            }
            else if (4 == _menu.idle.attribute)
            {
                sprintf(codes, "%3u", sensor_flicker_frequency_get(0, FUNCTION_SET_AB));
            }
            else if (5 == _menu.idle.attribute)
            {
                sprintf(codes, "%3u", sensor_amplitude_get(0, FUNCTION_SET_AB));
            }
            else if (6 == _menu.idle.attribute)
            {
                sprintf(codes, "%3u", sensor_quality_get(0, FUNCTION_SET_AB));
            }
            digit_set(codes);
        }
    }
    else if (MENU_COMM_PROTOCOL == _menu.type)
    {
        if (_menu.kb_idle && timer_diff(_menu.kb_idle) > 60000)
        {
            _menu.type = MENU_IDLE;
            return;
        }
        if (0 == _menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            if (_menu.comm.protocol == COMM_PROTOCOL_MODBUS)
                digit_set("Mb.");
            else if (_menu.comm.protocol == COMM_PROTOCOL_PROFIBUS)
                digit_set("Pb.");
            else
                digit_set("Unk");
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR == _menu.type)
    {
        if (0 == _menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            if (0 == _menu.comm.addr)
                digit_set("Ad1.");
            else if (1 == _menu.comm.addr)
                digit_set("Ad2.");
            else
                digit_set("Unk");
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR_VALUE == _menu.type)
    {
        if (0 == _menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            char codes[7] = {0};
            sprintf(codes, "%3u", _menu.comm.temp);
            digit_set(codes);
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR_TS == _menu.type)
    {
        if (0 == _menu.comm.timer)
        {
            _menu.comm.timer = timer_start();
            digit_set("tS");
        }
        else if (timer_diff(_menu.comm.timer) > 500)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR_VALUE;
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD == _menu.type)
    {
        if (!_menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            digit_set("Bdr");
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD_VALUE == _menu.type)
    {
        if (!_menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            char codes[7] = {0};
            sprintf(codes, "%3u", _menu.comm.baud);
            digit_set(codes);
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD_TS == _menu.type)
    {
        if (!_menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.comm.timer) > 1000)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_PROTOCOL_BAUD_VALUE;
        }
    }
    else if (MENU_COMM_RECORD == _menu.type)
    {
        if (0 == _menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            digit_set("rEC");
        }
    }
    else if (MENU_COMM_STORAGE == _menu.type)
    {
        if (!_menu.comm.timer)
        {
            _menu.comm.timer = timer_start();

            digit_set("Sto");
        }
        else if (timer_diff(_menu.comm.timer) > 1000)
        {
            _menu.comm.timer = 0;
            _menu.type = MENU_COMM_RECORD;
        }
    }
    else if (MENU_FUNCTION_SET_ACTIVE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            char names[5] = {0};
            sprintf(names, " %c%c.", function_set_active_ab() ? 'B' : 'A', function_set_active_cd() ? 'D' : 'C');
            digit_set(names);
        }
    }
    else if (MENU_FUNCTION_SET == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            if (0 == _menu.function_set.functor)
                digit_set("FSA");
            else if (1 == _menu.function_set.functor)
                digit_set("FSB");
            else if (2 == _menu.function_set.functor)
                digit_set("FSC");
            else if (3 == _menu.function_set.functor)
                digit_set("FSD");
            else
                digit_set("unk");
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            const char *names[] = {"IPI", "IDO", "INO", "ISN", "FPI", "FDO", "FNO", "FSN", "API", "ADO", "ANO", "ASN", "FSY", "DPI", "DDO"};
            digit_set(names[_menu.function_set.attribute]);
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE_VALUE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            char names[5] = {0};
            if (19 == _attributes[_menu.function_set.attribute] || 20 == _attributes[_menu.function_set.attribute])
            {
                sprintf(names, "%2u.%u", _menu.function_set.value / 10, _menu.function_set.value % 10);
            }
            else
            {
                sprintf(names, "%3u", _menu.function_set.value);
            }
            digit_set(names);
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE_TS == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.function_set.timer) > 1000)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE_VALUE;
        }
    }
    else if (MENU_OPERATING_MODE == _menu.type)
    {
        if (!_menu.operating_mode.timer)
        {
            _menu.operating_mode.timer = timer_start();

            digit_set("oPM");
        }
    }
    else if (MENU_OPERATING_MODE_VALUE == _menu.type)
    {
        if (!_menu.operating_mode.timer)
        {
            _menu.operating_mode.timer = timer_start();

            const char *names[] = {"Cnr", "WLI", "LGH", "TBN"};
            digit_set(names[_menu.operating_mode.value]);
        }
    }
    else if (MENU_OPERATING_MODE_TS == _menu.type)
    {
        if (!_menu.operating_mode.timer)
        {
            _menu.operating_mode.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.operating_mode.timer) > 1000)
        {
            _menu.operating_mode.timer = 0;
            _menu.type = MENU_OPERATING_MODE_VALUE;
        }
    }
    else if (MENU_FUNCTION_SET_SWITCH == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("FSS");
        }
    }
    else if (MENU_FUNCTION_SET_SWITCH_VALUE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            const char *names[] = {"OFF", "dln", "Srl"};
            digit_set(names[_menu.function_set.value]);
        }
    }
    else if (MENU_FUNCTION_SET_SWITCH_TS == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.function_set.timer) > 1000)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_FUNCTION_SET_SWITCH_VALUE;
        }
    }
    else if (MENU_HIGH_FREQ_LIMIT == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("HIL");
        }
    }
    else if (MENU_HIGH_FREQ_LIMIT_VALUE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            const char *names[] = {"diH", "EnH"};
            digit_set(names[_menu.function_set.value]);
        }
    }
    else if (MENU_HIGH_FREQ_LIMIT_TS == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.function_set.timer) > 1000)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_HIGH_FREQ_LIMIT_VALUE;
        }
    }
    else if (MENU_AC_AMPLITUDE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("ACA");
        }
    }
    else if (MENU_AC_AMPLITUDE_VALUE == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            const char *names[] = {"diA", "EnA"};
            digit_set(names[_menu.function_set.value]);
        }
    }
    else if (MENU_AC_AMPLITUDE_VALUE_TS == _menu.type)
    {
        if (!_menu.function_set.timer)
        {
            _menu.function_set.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.function_set.timer) > 1000)
        {
            _menu.function_set.timer = 0;
            _menu.type = MENU_AC_AMPLITUDE_VALUE;
        }
    }
    else if (MENU_SAFE_RELAY_USAGE == _menu.type)
    {
        if (!_menu.safe_relay_usage.timer)
        {
            _menu.safe_relay_usage.timer = timer_start();

            digit_set("SrU");
        }
    }
    else if (MENU_SAFE_RELAY_USAGE_VALUE == _menu.type)
    {
        if (!_menu.safe_relay_usage.timer)
        {
            _menu.safe_relay_usage.timer = timer_start();

            const char *names[] = {"SFr", "SSF", "Srq", "Srt"};
            digit_set(names[_menu.safe_relay_usage.value]);
        }
    }
    else if (MENU_SAFE_RELAY_USAGE_TS == _menu.type)
    {
        if (!_menu.safe_relay_usage.timer)
        {
            _menu.safe_relay_usage.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.safe_relay_usage.timer) > 1000)
        {
            _menu.safe_relay_usage.timer = 0;
            _menu.type = MENU_SAFE_RELAY_USAGE_VALUE;
        }
    }
    else if (MENU_ANALOG_OUTPUT_DISPLAY == _menu.type)
    {
        if (!_menu.ao.timer)
        {
            _menu.ao.timer = timer_start();

            digit_set("Aoo");
        }
    }
    else if (MENU_ANALOG_OUTPUT_DISPLAY_SET == _menu.type)
    {
        if (!_menu.ao.timer)
        {
            _menu.ao.timer = timer_start();

            const char *names[] = {"QLt", "InT", "Frq", "AMP", "tMP", "qLC"};
            digit_set(names[_menu.ao.value]);
        }
    }
    else if (MENU_ANALOG_OUTPUT_DISPLAY_TS == _menu.type)
    {
        if (!_menu.ao.timer)
        {
            _menu.ao.timer = timer_start();

            digit_set("tS");
        }
        else if (timer_diff(_menu.ao.timer) > 1000)
        {
            _menu.ao.timer = 0;
            _menu.type = MENU_ANALOG_OUTPUT_DISPLAY;
        }
    }
    else if (MENU_CONFIGURE_DEFAULT == _menu.type)
    {
        if (!_menu.configure.timer)
        {
            _menu.configure.timer = timer_start();

            digit_set("dEF");
        }
    }
    else if (MENU_CONFIGURE_DEFAULT_SET == _menu.type)
    {
        if (!_menu.configure.timer)
        {
            _menu.configure.timer = timer_start();

            digit_set(" . . .");
        }
    }
    else if (MENU_CONFIGURE_STORAGE == _menu.type)
    {
        if (!_menu.configure.timer)
        {
            _menu.configure.timer = timer_start();

            digit_set("Sto");
        }
        else if (timer_diff(_menu.configure.timer) > 1000)
        {
            function_set_default_load();
            operating_mode_default_load();
            relay_default_load();
            analog_output_default_load();

            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_DEFAULT_SET;
        }
    }
    else if (MENU_CONFIGURE_RECORD == _menu.type)
    {
        if (!_menu.configure.timer)
        {
            _menu.configure.timer = timer_start();

            digit_set("rEC");
        }
    }
    else if (MENU_CONFIGURE_RECORD_STORAGE == _menu.type)
    {
        if (!_menu.configure.timer)
        {
            _menu.configure.timer = timer_start();

            digit_set("Sto");
        }
        else if (timer_diff(_menu.configure.timer) > 1000)
        {
            function_set_configure_apply();
            operating_mode_configure_apply();
            relay_configure_apply();
            analog_output_configure_apply();

            _menu.configure.timer = 0;
            _menu.type = MENU_CONFIGURE_RECORD;
        }
    }
}
