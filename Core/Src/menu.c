#include "menu.h"
#include "key.h"
#include "digit.h"

typedef struct {
    enum {
        MENU_IDLE_FWVER = 0,
        MENU_IDLE,
        MENU_COMM_PROTOCOL,
        MENU_COMM_PROTOCOL_ADDR,
        MENU_COMM_PROTOCOL_ADDR_LOOP,
        MENU_COMM_PROTOCOL_ADDR_TS,
        MENU_COMM_PROTOCOL_BAUD,
        MENU_COMM_PROTOCOL_BAUD_LOOP,
        MENU_COMM_PROTOCOL_BAUD_TS,
        MENU_COMM_RECORD,
        MENU_COMM_STORAGE,
        MENU_FUNCTION_SET,
        MENU_FUNCTION_SET_ATTRIBUTE,
        MENU_FUNCTION_SET_ATTRIBUTE_VALUE,
        MENU_FUNCTION_SET_DEFAULT,
        MENU_FUNCTION_SET_STORAGE,
    }type;
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
            uint8_t max_attribute;
            uint8_t functor;
            uint8_t value;
            uint16_t max_value;
            uint16_t min_value;
        }function_set;
        struct {
            uint8_t attribute;
            uint32_t timer;
        }idle;
    };
}Menu;

static Menu _menu = {
    .type = MENU_IDLE_FWVER,
};

static void key_event(uint8_t key, uint8_t action)
{
    if (MENU_IDLE == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            if (KEY_EVENT_PRESS == action)
            {
                _menu.type = MENU_COMM_PROTOCOL;
                _menu.comm.protocol = COMM_PROTOCOL_MODBUS;
                if (COMM_PROTOCOL_PROFIBUS == usart_comm_protocol())
                    _menu.comm.protocol = COMM_PROTOCOL_PROFIBUS;
            }
        }
        if (KEY_RIGHT == key)
        {
            if (KEY_EVENT_CLICK == action)
            {
                _menu.function_set.functor = 0;
                _menu.type = MENU_FUNCTION_SET;
            }
        }
        if (KEY_DOWN == key)
        {
            if (_menu.idle.attribute < 6)
                _menu.idle.attribute++;
            _menu.type = MENU_IDLE_LABEL;
        }
        if (KEY_UP == key)
        {
            if (_menu.idle.attribute > 0)
                _menu.idle.attribute--;
            _menu.type = MENU_IDLE_LABEL;
        }
    }
    else if (MENU_FUNCTION_SET == _menu.type)
    {
        if (KEY_UP == key)
        {
            if (_menu.function_set.functor > 0)
                _menu.function_set.functor--;
        }
        if (KEY_DOWN == key)
        {
            if (_menu.function_set.functor < 10)
            {
                _menu.function_set.functor++;
            }
            else
            {
                _menu.type = MENU_FUNCTION_SET_DEFAULT;
            }
        }
        if (_menu.function_set.functor <= 3)
        {
            _menu.function_set.max_attribute = 15;
        }
        else if (4 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 4;
        }
        else if (5 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 3;
        }
        else if (6 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 2;
        }
        else if (7 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 2;
        }
        else if (8 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 4;
        }
        else if (9 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 1;
        }
        else if (10 == _menu.function_set.functor)
        {
            _menu.function_set.max_attribute = 6;
        }
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key)
        {
            if (KEY_EVENT_CLICK == action)
            {
                _menu.function_set.attribute = 0;
                _menu.type = MENU_FUNCTION_SET_ATTRIBUTE;
            }
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_RIGHT == key)
        {
            _menu.function_set.value = function_set_temp_get(_menu.function_set.functor, _menu.function_set.attribute, &_menu.function_set.min_value, &_menu.function_set.max_value);
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE_VALUE;
        }
        if (KEY_UP == key)
        {
            if (_menu.function_set.attribute > 0)
            {
                _menu.function_set.attribute--;
            }
        }
        if (KEY_DOWN == key)
        {
            if (_menu.function_set.attribute < (_menu.function_set.max_attribute - 1))
            {
                _menu.function_set.attribute++;
            }
        }
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE_VALUE == _menu.type)
    {
        if (KEY_UP == key)
        {
            if (_menu.function_set.value < _menu.function_set.max_value)
                _menu.function_set.value++;
        }
        if (KEY_DOWN == key)
        {
            if (_menu.function_set.value > _menu.function_set.min_value)
                _menu.function_set.value--;
        }
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_FUNCTION_SET_ATTRIBUTE;
        }
        if (KEY_RIGHT == key)
        {
            function_set_temp_set(_menu.function_set.functor, _menu.function_set.attribute, _menu.function_set.value);
        }
    }
    else if (MENU_FUNCTION_SET_DEFAULT == _menu.type)
    {
        if (KEY_UP == key)
        {
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_DOWN == key)
        {
            _menu.type = MENU_FUNCTION_SET_STORAGE;
        }
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key)
        {
            function_set_temp_default();
        }
    }
    else if (MENU_FUNCTION_SET_STORAGE == _menu.type)
    {
        if (KEY_UP == key)
        {
            _menu.type = MENU_FUNCTION_SET_DEFAULT;
        }
        if (KEY_DOWN == key)
        {
            _menu.type = MENU_FUNCTION_SET;
        }
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key)
        {
            function_set_apply();
        }
    }
    else if (MENU_COMM_PROTOCOL == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key)
        {
            _menu.comm.addr = 0;
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
        if (KEY_DOWN == key)
        {
            if (COMM_PROTOCOL_MODBUS == _menu.comm.protocol)
                _menu.comm.protocol = COMM_PROTOCOL_PROFIBUS;
            else if (COMM_PROTOCOL_PROFIBUS == _menu.comm.protocol)
                _menu.type = MENU_COMM_RECORD;
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_COMM_PROTOCOL;
        }
        if (KEY_RIGHT == key)
        {
            _menu.comm.temp = usart_comm_device_address(_menu.comm.addr);
            if (_menu.comm.save & (1 << _menu.comm.addr))
                _menu.comm.temp = _menu.comm.addrs[_menu.comm.addr];
            _menu.type = MENU_COMM_PROTOCOL_ADDR_LOOP;
        }
        if (KEY_UP == key)
        {
            if (1 == _menu.comm.addr)
                _menu.comm.addr = 0;
        }
        if (KEY_DOWN == key)
        {
            if (0 == _menu.comm.addr)
            {
                _menu.comm.addr = 1;
            }
            else if (1 == _menu.comm.addr)
            {
                if (!(_menu.comm.save & (1 << 2)))
                    _menu.comm.baud = usart_comm_baud();
                _menu.type = MENU_COMM_PROTOCOL_BAUD;
            }
        }
    }
    else if (MENU_COMM_PROTOCOL_ADDR_LOOP == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
        if (KEY_RIGHT == key)
        {
            _menu.comm.save |= 1 << _menu.comm.addr;
            usart_comm_device_address_set(_menu.comm.protocol, _menu.comm.addr, _menu.comm.temp);
            _menu.comm.timer = timer_start();
            _menu.type = MENU_COMM_PROTOCOL_ADDR_TS;
        }
        if (KEY_UP == key)
        {
            _menu.comm.temp++;
        }
        if (KEY_DOWN == key)
        {
            _menu.comm.temp--;
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_COMM_PROTOCOL;
        }
        if (KEY_RIGHT == key)
        {
            _menu.type = MENU_COMM_PROTOCOL_BAUD_LOOP;
        }
        if (KEY_UP == key)
        {
            _menu.comm.addr = 1;
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
        }
    }
    else if (MENU_COMM_PROTOCOL_BAUD_LOOP == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_COMM_PROTOCOL_BAUD;
        }
        if (KEY_RIGHT == key)
        {
            _menu.comm.save |= 1 << 2;
            usart_comm_baud_set(_menu.comm.protocol, _menu.comm.baud);
            _menu.comm.timer = timer_start();
            _menu.type = MENU_COMM_PROTOCOL_BAUD_TS;
        }
        if (KEY_UP == key)
        {
            _menu.comm.baud++;
        }
        if (KEY_DOWN == key)
        {
            _menu.comm.baud--;
        }
    }
    else if (MENU_COMM_RECORD == _menu.type)
    {
        if (KEY_LEFT == key)
        {
            _menu.type = MENU_IDLE;
        }
        if (KEY_RIGHT == key)
        {
            if (_menu.comm.save & (1 << 0))
            {
                usart_comm_device_address_apply(_menu.comm.protocol, 0);
            }
            if (_menu.comm.save & (1 << 1))
            {
                usart_comm_device_address_apply(_menu.comm.protocol, 1);
            }
            if (_menu.comm.save & (1 << 2))
            {
                usart_comm_baud_apply(_menu.comm.protocol);
            }
            _menu.comm.save = 0;
            
            _menu.comm.timer = timer_start();
            _menu.type = MENU_COMM_STORAGE;
        }
        if (KEY_UP == key)
        {
            _menu.type = MENU_COMM_PROTOCOL;
        }
    }
}

void menu_init(void)
{
    key_handler_set(key_event);
}

void menu_update(void)
{
    if (MENU_IDLE_FWVER == _menu.type)
    {
        digit_set(FW_VERSION);
        _menu.type = MENU_IDLE_LABEL;
        _menu.idle.attribute = 0;
        _menu.idle.timer = timer_start();
    }
    else if (MENU_IDLE_LABEL == _menu.type)
    {
        if (timer_diff(_menu.idle.timer) < 500)
            return;
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
        _menu.type = MENU_IDLE;
        _menu.idle.timer = timer_start();
    }
    else if (MENU_IDLE == _menu.type)
    {
        if (_menu.idle.timer && timer_diff(_menu.idle.timer) < 500)
            return;
        char codes[4] = {0};
        if (0 == _menu.idle.attribute)
        {
            sprintf(codes, "%u", output_4_20ma_get_percent());
        }
        else if (1 == _menu.idle.attribute)
        {
            sprintf(codes, "%d", tempareture_get())
        }
        else if (2 == _menu.idle.attribute)
        {
            sprintf(codes, "UV");
        }
        else if (3 == _menu.idle.attribute)
        {
            sprintf(codes, "%u", sensor_intensity_get_percent());
        }
        else if (4 == _menu.idle.attribute)
        {
            sprintf(codes, "%u", sensor_frequency_get_percent());
        }
        else if (5 == _menu.idle.attribute)
        {
            sprintf(codes, "%u", sensor_amplitude_get_percent());
        }
        else if (6 == _menu.idle.attribute)
        {
            sprintf(codes, "%u", sensor_quality_get_percent());
        }
        digit_set(codes);
        _menu.idle.timer = 0;
    }
    else if (MENU_COMM_PROTOCOL == _menu.type)
    {
        if (_menu.comm.protocol == COMM_PROTOCOL_MODBUS)
            digit_set("Mb.");
        else if (_menu.comm.protocol == COMM_PROTOCOL_PROFIBUS)
            digit_set("Pb.");
        else
            digit_set("Unk");
    }
    else if (MENU_COMM_PROTOCOL_ADDR == _menu.type)
    {
        if (0 == _menu.comm.addr)
            digit_set("Ad1");
        else if (1 == _menu.comm.addr)
            digit_set("Ad2");
        else
            digit_set("Unk");
    }
    else if (MENU_COMM_PROTOCOL_ADDR_LOOP == _menu.type)
    {
        char codes[7] = {0};
        sprintf(codes, "%u", _menu.comm.temp);
        digit_set(codes);
    }
    else if (MENU_COMM_PROTOCOL_ADDR_TS == _menu.type)
    {
        digit_set("tS");
        if (timer_diff(_menu.comm.timer) > 1000)
            _menu.type = MENU_COMM_PROTOCOL_ADDR;
    }
    else if (MENU_COMM_PROTOCOL_BAUD == _menu.type)
    {
        digit_set("Bdr");
    }
    else if (MENU_COMM_PROTOCOL_BAUD_LOOP == _menu.type)
    {
        char codes[7] = {0};
        sprintf(codes, "%u", _menu.comm.baud);
        digit_set(codes);
    }
    else if (MENU_COMM_PROTOCOL_BAUD_TS == _menu.type)
    {
        digit_set("tS");
        if (timer_diff(_menu.comm.timer) > 1000)
            _menu.type = MENU_COMM_PROTOCOL_BAUD;
    }
    else if (MENU_COMM_RECORD == _menu.type)
    {
        digit_set("rEC");
    }
    else if (MENU_COMM_STORAGE == _menu.type)
    {
        digit_set("Sto");
        if (timer_diff(_menu.comm.timer) > 1000)
            _menu.type = MENU_COMM_RECORD;
    }
    else if (MENU_FUNCTION_SET == _menu.type)
    {
        if (0 == _menu.function_set.functor)
            digit_set("FSA");
        else if (1 == _menu.function_set.functor)
            digit_set("FSB");
        else if (2 == _menu.function_set.functor)
            digit_set("FSC");
        else if (3 == _menu.function_set.functor)
            digit_set("FSD");
        else if (4 == _menu.function_set.functor)
            digit_set("oPM");
        else if (5 == _menu.function_set.functor)
            digit_set("FSS");
        else if (6 == _menu.function_set.functor)
            digit_set("HIL");
        else if (7 == _menu.function_set.functor)
            digit_set("ACA");
        else if (8 == _menu.function_set.functor)
            digit_set("SrU");
        else if (9 == _menu.function_set.functor)
            digit_set("AoM");
        else if (10 == _menu.function_set.functor)
            digit_set("Aoo");
        else
            digit_set("unk");
    }
    else if (MENU_FUNCTION_SET_ATTRIBUTE == _menu.type)
    {
        if (_menu.function_set.functor <= 3)
        {
            const char *names[] = {"IPI", "IDO", "INO", "ISN", "FPI", "FDO", "FNO", "FSN", "API", "ADO", "ANO", "ASN", "FSY", "DPI", "DDO"};
            digit_set(names[_menu.function_set.attribute]);
        }
        else if (4 == _menu.function_set.functor)
        {
            const char *names[] = {"Cnr", "WLI", "LGH", "TBN"};
            digit_set(names[_menu.function_set.attribute]);
        }
        else if (5 == _menu.function_set.functor)
        {
            const char *names[] = {"OFF", "dln", "Srl"};
            digit_set(names[_menu.function_set.attribute]);
        }
        else if (5 == _menu.function_set.functor)
        {
            const char *names[] = {"OFF", "dln", "Srl"};
            digit_set(names[_menu.function_set.attribute]);
        }
    }
}
