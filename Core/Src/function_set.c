#include "function_set.h"
#include "main.h"
#include "log.h"
#include "led.h"
#include "sensor.h"
#include "relay.h"
#include "eeprom.h"
#include "timer.h"
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static struct function_set _fsa = {0}, _fsb = {0}, _fsc = {0}, _fsd = {0}, _ta = {0}, _tb = {0}, _tc = {0}, _td = {0}, _default = {
    .intensity = {
        .trip = {
            .pull_in = 30,
            .drop_out = 30,
            .high = 100,
        },
        .normalization = {
            .value = 20,
            .high = 5,
        },
        .filter = 0,
    },
    .frequency = {
        .trip = {
            .pull_in = 10,
            .drop_out = 10,
            .high = 125,
        },
        .normalization = {
            .value = 15,
            .high = 1,
        },
        .filter = 2,
        .max = 125,
        .sensitivity = 10,
    },
    .amplitude = {
        .trip = {
            .pull_in = 10,
            .drop_out = 10,
            .high = 100,
        },
        .normalization = {
            .value = 20,
            .high = 1,
        },
        .filter = 2,
    },
    .delay = {
        .pull_in = 20,
        .drop_out = 20,
    },
    .quality_threshold = 0,
};

static uint8_t _active_ab = 0;
static uint8_t _active_cd = 0;
static uint8_t _switch = FUNCTION_SET_SWITCH_OFF;
static uint8_t _switch_temp = FUNCTION_SET_SWITCH_OFF;
static uint8_t _alternative_flame_logic = 0;//0 - or/1 - and
static uint8_t _high_limit_enable = 0;
static uint8_t _high_limit_enable_temp = 0;
static uint8_t _ac_amplitude_enable = 0;
static uint8_t _ac_amplitude_enable_temp = 0;

static uint8_t _di1 = 0;
static uint8_t _di2 = 0;

static uint32_t _save_timer = 0;

static void function_set_info(struct function_set *f)
{
    SEGGER_RTT_printf(0, "intensity:\n");
    SEGGER_RTT_printf(0, "\ttrip(pull:%u drop:%u high:%u) ", f->intensity.trip.pull_in, f->intensity.trip.drop_out, f->intensity.trip.high);
    SEGGER_RTT_printf(0, "norm(value:%u high:%u) ", f->intensity.normalization.value, f->intensity.normalization.high);
    SEGGER_RTT_printf(0, "filter:%u\n", f->intensity.filter);

    SEGGER_RTT_printf(0, "frequency:\n");
    SEGGER_RTT_printf(0, "\ttrip(pull:%u drop:%u high:%u) ", f->frequency.trip.pull_in, f->frequency.trip.drop_out, f->frequency.trip.high);
    SEGGER_RTT_printf(0, "norm(value:%u high:%u) ", f->frequency.normalization.value, f->frequency.normalization.high);
    SEGGER_RTT_printf(0, "filter:%u max:%u sensitivity:%u\n", f->frequency.filter, f->frequency.max, f->frequency.sensitivity);
    
    SEGGER_RTT_printf(0, "amplitude:\n");
    SEGGER_RTT_printf(0, "\ttrip(pull:%u drop:%u high:%u) ", f->amplitude.trip.pull_in, f->amplitude.trip.drop_out, f->amplitude.trip.high);
    SEGGER_RTT_printf(0, "norm(value:%u high:%u) ", f->amplitude.normalization.value, f->amplitude.normalization.high);
    SEGGER_RTT_printf(0, "filter:%u\n", f->amplitude.filter);
    
    SEGGER_RTT_printf(0, "delay(pull:%u drop:%u)\n", f->delay.pull_in, f->delay.drop_out);
    
    SEGGER_RTT_printf(0, "quality(threshold:%u)\n", f->quality_threshold);
}

void function_set_init(void)
{
    eeprom_read(EEPROM_SETTINGS_FSA, &_fsa, sizeof(_fsa));
    eeprom_read(EEPROM_SETTINGS_FSB, &_fsb, sizeof(_fsb));
    eeprom_read(EEPROM_SETTINGS_FSC, &_fsc, sizeof(_fsc));
    eeprom_read(EEPROM_SETTINGS_FSD, &_fsd, sizeof(_fsd));
    eeprom_read(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
    eeprom_read(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
    eeprom_read(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
    eeprom_read(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
    eeprom_read(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
    eeprom_read(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
    if (_fsa.quality_threshold == 0xffff)
    {
        SEGGER_RTT_printf(0, "load default function set\n");
        memcpy(&_fsa, &_default, sizeof(_default));
        memcpy(&_fsb, &_default, sizeof(_default));
        memcpy(&_fsc, &_default, sizeof(_default));
        memcpy(&_fsd, &_default, sizeof(_default));
        _switch = FUNCTION_SET_SWITCH_OFF;
        _active_ab = 0;
        _active_cd = 0;
        _alternative_flame_logic = 0;
        _high_limit_enable = 0;
        _ac_amplitude_enable = 0;

        eeprom_write(EEPROM_SETTINGS_FSA, &_fsa, sizeof(_fsa));
        eeprom_write(EEPROM_SETTINGS_FSB, &_fsb, sizeof(_fsb));
        eeprom_write(EEPROM_SETTINGS_FSC, &_fsc, sizeof(_fsc));
        eeprom_write(EEPROM_SETTINGS_FSD, &_fsd, sizeof(_fsd));
        eeprom_write(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
        eeprom_write(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
        eeprom_write(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
    }

    memcpy(&_ta, &_fsa, sizeof(_ta));
    memcpy(&_tb, &_fsb, sizeof(_tb));
    memcpy(&_tc, &_fsc, sizeof(_tc));
    memcpy(&_td, &_fsd, sizeof(_td));
    _switch_temp = _switch;
    _high_limit_enable_temp = _high_limit_enable;
    _ac_amplitude_enable_temp = _ac_amplitude_enable;

    SEGGER_RTT_printf(0, "active ab:%u/cd:%u\n", _active_ab, _active_cd);
    SEGGER_RTT_printf(0, "fsa ");
    function_set_info(&_fsa);
    SEGGER_RTT_printf(0, "fsb ");
    function_set_info(&_fsb);
    SEGGER_RTT_printf(0, "fsc ");
    function_set_info(&_fsc);
    SEGGER_RTT_printf(0, "fsd ");
    function_set_info(&_fsd);
    SEGGER_RTT_printf(0, "switch:%u\n", _switch);
    SEGGER_RTT_printf(0, "high limit:%u\n", _high_limit_enable);
    SEGGER_RTT_printf(0, "ac amplitude:%u\n", _ac_amplitude_enable);
}

void function_set_update(void)
{
    _di1 = !!HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin);
    _di2 = !!HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin);

    if (FUNCTION_SET_SWITCH_OFF == _switch)
    {
        _active_ab = 0;
        _active_cd = 0;
    }
    else if (FUNCTION_SET_SWITCH_DIGITAL_INPUTS == _switch)
    {
        _active_ab = _di1;
        _active_cd = _di2;
    }
    else if (FUNCTION_SET_SWITCH_SERIAL_LINE == _switch)
    {
    }

    if (_save_timer && timer_diff(_save_timer) > 1000)
    {
        _save_timer = 0;
        eeprom_write(EEPROM_SETTINGS_FSA, &_fsa, sizeof(struct function_set));
        eeprom_write(EEPROM_SETTINGS_FSB, &_fsb, sizeof(struct function_set));
        eeprom_write(EEPROM_SETTINGS_FSC, &_fsc, sizeof(struct function_set));
        eeprom_write(EEPROM_SETTINGS_FSD, &_fsd, sizeof(struct function_set));
        eeprom_write(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
        eeprom_write(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
        eeprom_write(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
    }
}

uint8_t function_set_dinput(uint8_t i)
{
    if (i)
        return _di2;
    return _di1;
}

struct function_set *function_set_get(uint8_t fs)
{
    struct function_set *f = 0;
    if (!fs)
    {
        if (!_active_ab)
        {
            f = &_fsa;
        }
        else
        {
            f = &_fsb;
        }
    }
    else
    {
        if (!_active_cd)
        {
            f = &_fsc;
        }
        else
        {
            f = &_fsd;
        }
    }
    return f;
}

uint8_t function_set_flame_status_get(uint8_t sensor, struct function_set *f, uint8_t prev_status)
{
    uint16_t intensity = sensor_intensity_get(sensor, f->intensity.filter);
    uint16_t frequency = sensor_flicker_frequency_get(sensor, f->frequency.filter, f->frequency.sensitivity, f->frequency.max);
    uint16_t amplitude = sensor_amplitude_get(sensor, f->amplitude.filter);

    if (prev_status)
    {
        if ((intensity < f->intensity.trip.drop_out &&
            frequency < f->frequency.trip.drop_out &&
            (!_ac_amplitude_enable || amplitude < f->amplitude.trip.drop_out)) || 
            (_high_limit_enable && (
                intensity >= f->intensity.trip.high &&
                frequency >= f->frequency.trip.high &&
                (!_ac_amplitude_enable || amplitude >= f->amplitude.trip.high))))
        {
            return 0;
        }
    }
    else
    {
        if (intensity >= f->intensity.trip.pull_in &&
            frequency >= f->frequency.trip.pull_in &&
            (!_ac_amplitude_enable || amplitude >= f->amplitude.trip.pull_in))
        {
            return 1;
        }
    }
    return prev_status;
}

uint8_t function_set_active_ab(void)
{
    return _active_ab;
}

void function_set_active_ab_set(uint8_t v)
{
    _active_ab = v;

    if (FUNCTION_SET_SWITCH_SERIAL_LINE == _switch)
    {
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
    }
}

uint8_t function_set_active_cd(void)
{
    return _active_cd;
}

void function_set_active_cd_set(uint8_t v)
{
    _active_cd = v;
    
    eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
}

uint8_t function_set_alternative_flame_logic_get(void)
{
    return _alternative_flame_logic;
}

void function_set_alternative_flame_logic_set(uint8_t v)
{
    _alternative_flame_logic = v;
    
    eeprom_write(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
}

uint8_t function_set_switch_temp_get(void)
{
    return _switch_temp;
}

void function_set_switch_ts(uint8_t v)
{
    _switch_temp = v;
}

uint8_t function_set_switch_get(void)
{
    return _switch;
}

void function_set_switch(uint8_t v)
{
    _switch = v;
    
    eeprom_write(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
    eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
    eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
}

uint8_t is_function_set_enable_high_limit(void)
{
    return _high_limit_enable;
}

void function_set_enable_high_limit(uint8_t v)
{
    _high_limit_enable = v;
    eeprom_write(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
}

uint8_t function_set_enable_high_limit_temp_get(void)
{
    return _high_limit_enable_temp;
}

void function_set_enable_high_limit_ts(uint8_t v)
{
    _high_limit_enable_temp = v;
}

uint8_t is_function_set_enable_ac_amplitude(void)
{
    return _ac_amplitude_enable;
}

void function_set_enable_ac_amplitude(uint8_t v)
{
    _ac_amplitude_enable = v;
    eeprom_write(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
}

uint8_t function_set_enable_ac_amplitude_temp_get(void)
{
    return _ac_amplitude_enable_temp;
}

void function_set_enable_ac_amplitude_ts(uint8_t v)
{
    _ac_amplitude_enable_temp = v;
}

uint16_t function_set_attribute_get(uint8_t functor, uint8_t attribute)
{
    struct function_set *f = 0;
    if (0 == functor)
    {
        f = &_fsa;
    }
    else if (1 == functor)
    {
        f = &_fsb;
    }
    else if (2 == functor)
    {
        f = &_fsc;
    }
    else if (3 == functor)
    {
        f = &_fsd;
    }
    if (0 == attribute)
    {
        return f->intensity.trip.pull_in;
    }
    else if (1 == attribute)
    {
        return f->intensity.trip.drop_out;
    }
    else if (2 == attribute)
    {
        return f->intensity.trip.high;
    }
    else if (3 == attribute)
    {
        return f->intensity.normalization.value;
    }
    else if (4 == attribute)
    {
        return f->intensity.normalization.high;
    }
    else if (5 == attribute)
    {
        return f->intensity.filter;
    }
    else if (6 == attribute)
    {
        return f->frequency.trip.pull_in;
    }
    else if (7 == attribute)
    {
        return f->frequency.trip.drop_out;
    }
    else if (8 == attribute)
    {
        return f->frequency.trip.high;
    }
    else if (9 == attribute)
    {
        return f->frequency.normalization.value;
    }
    else if (10 == attribute)
    {
        return f->frequency.normalization.high;
    }
    else if (11 == attribute)
    {
        return f->frequency.filter;
    }
    else if (12 == attribute)
    {
        return f->amplitude.trip.pull_in;
    }
    else if (13 == attribute)
    {
        return f->amplitude.trip.drop_out;
    }
    else if (14 == attribute)
    {
        return f->amplitude.trip.high;
    }
    else if (15 == attribute)
    {
        return f->amplitude.normalization.value;
    }
    else if (16 == attribute)
    {
        return f->amplitude.normalization.high;
    }
    else if (17 == attribute)
    {
        return f->amplitude.filter;
    }
    else if (18 == attribute)
    {
        return f->frequency.max;
    }
    else if (19 == attribute)
    {
        return f->delay.drop_out;
    }
    else if (20 == attribute)
    {
        return f->delay.pull_in;
    }
    else if (21 == attribute)
    {
        return f->frequency.sensitivity;
    }
    else if (22 == attribute)
    {
        return f->quality_threshold;
    }
	return 0;
}

void function_set_attribute_set(uint8_t functor, uint8_t attribute, uint16_t v)
{
    log_append(0x03, functor << 8 | attribute);

    struct function_set *f = 0;
    struct function_set *t = 0;
    if (0 == functor)
    {
        f = &_fsa;
        t = &_ta;
    }
    else if (1 == functor)
    {
        f = &_fsb;
        t = &_tb;
    }
    else if (2 == functor)
    {
        f = &_fsc;
        t = &_tc;
    }
    else if (3 == functor)
    {
        f = &_fsd;
        t = &_td;
    }
    if (0 == attribute)
    {
        f->intensity.trip.pull_in = v;
        t->intensity.trip.pull_in = v;
    }
    else if (1 == attribute)
    {
        f->intensity.trip.drop_out = v;
        t->intensity.trip.drop_out = v;
    }
    else if (2 == attribute)
    {
        f->intensity.trip.high = v;
        t->intensity.trip.high = v;
    }
    else if (3 == attribute)
    {
        f->intensity.normalization.value = v;
        t->intensity.normalization.value = v;
    }
    else if (4 == attribute)
    {
        f->intensity.normalization.high = v;
        t->intensity.normalization.high = v;
    }
    else if (5 == attribute)
    {
        f->intensity.filter = v;
        t->intensity.filter = v;
    }
    else if (6 == attribute)
    {
        f->frequency.trip.pull_in = v;
        t->frequency.trip.pull_in = v;
    }
    else if (7 == attribute)
    {
        f->frequency.trip.drop_out = v;
        t->frequency.trip.drop_out = v;
    }
    else if (8 == attribute)
    {
        f->frequency.trip.high = v;
        t->frequency.trip.high = v;
    }
    else if (9 == attribute)
    {
        f->frequency.normalization.value = v;
        t->frequency.normalization.value = v;
    }
    else if (10 == attribute)
    {
        f->frequency.normalization.high = v;
        t->frequency.normalization.high = v;
    }
    else if (11 == attribute)
    {
        f->frequency.filter = v;
        t->frequency.filter = v;
    }
    else if (12 == attribute)
    {
        f->amplitude.trip.pull_in = v;
        t->amplitude.trip.pull_in = v;
    }
    else if (13 == attribute)
    {
        f->amplitude.trip.drop_out = v;
        t->amplitude.trip.drop_out = v;
    }
    else if (14 == attribute)
    {
        f->amplitude.trip.high = v;
        t->amplitude.trip.high = v;
    }
    else if (15 == attribute)
    {
        f->amplitude.normalization.value = v;
        t->amplitude.normalization.value = v;
    }
    else if (16 == attribute)
    {
        f->amplitude.normalization.high = v;
        t->amplitude.normalization.high = v;
    }
    else if (17 == attribute)
    {
        f->amplitude.filter = v;
        t->amplitude.filter = v;
    }
    else if (18 == attribute)
    {
        f->frequency.max = v;
        t->frequency.max = v;
    }
    else if (19 == attribute)
    {
        f->delay.drop_out = v;
        t->delay.drop_out = v;
    }
    else if (20 == attribute)
    {
        f->delay.pull_in = v;
        t->delay.pull_in = v;
    }
    else if (21 == attribute)
    {
        f->frequency.sensitivity = v;
        t->frequency.sensitivity = v;
    }
    else if (22 == attribute)
    {
        f->quality_threshold = v;
        t->quality_threshold = v;
    }
    _save_timer = timer_start();
}

void function_set_attribute_ts(uint8_t functor, uint8_t attribute, uint16_t v)
{
    log_append(0x04, functor << 8 | attribute);

    struct function_set *f = 0;
    if (0 == functor)
    {
        f = &_ta;
    }
    else if (1 == functor)
    {
        f = &_tb;
    }
    else if (2 == functor)
    {
        f = &_tc;
    }
    else if (3 == functor)
    {
        f = &_td;
    }
    if (0 == attribute)
    {
        f->intensity.trip.pull_in = v;
    }
    else if (1 == attribute)
    {
        f->intensity.trip.drop_out = v;
    }
    else if (2 == attribute)
    {
        f->intensity.trip.high = v;
    }
    else if (3 == attribute)
    {
        f->intensity.normalization.value = v;
    }
    else if (4 == attribute)
    {
        f->intensity.normalization.high = v;
    }
    else if (5 == attribute)
    {
        f->intensity.filter = v;
    }
    else if (6 == attribute)
    {
        f->frequency.trip.pull_in = v;
    }
    else if (7 == attribute)
    {
        f->frequency.trip.drop_out = v;
    }
    else if (8 == attribute)
    {
        f->frequency.trip.high = v;
    }
    else if (9 == attribute)
    {
        f->frequency.normalization.value = v;
    }
    else if (10 == attribute)
    {
        f->frequency.normalization.high = v;
    }
    else if (11 == attribute)
    {
        f->frequency.filter = v;
    }
    else if (12 == attribute)
    {
        f->amplitude.trip.pull_in = v;
    }
    else if (13 == attribute)
    {
        f->amplitude.trip.drop_out = v;
    }
    else if (14 == attribute)
    {
        f->amplitude.trip.high = v;
    }
    else if (15 == attribute)
    {
        f->amplitude.normalization.value = v;
    }
    else if (16 == attribute)
    {
        f->amplitude.normalization.high = v;
    }
    else if (17 == attribute)
    {
        f->amplitude.filter = v;
    }
    else if (18 == attribute)
    {
        f->frequency.max = v;
    }
    else if (19 == attribute)
    {
        f->delay.drop_out = v;
    }
    else if (20 == attribute)
    {
        f->delay.pull_in = v;
    }
    else if (21 == attribute)
    {
        f->frequency.sensitivity = v;
    }
    else if (22 == attribute)
    {
        f->quality_threshold = v;
    }
}

uint16_t function_set_temp_get(uint16_t functor, uint8_t attribute, uint16_t *min, uint16_t *max)
{
    struct function_set *f = 0;
    if (0 == functor)
    {
        f = &_ta;
    }
    else if (1 == functor)
    {
        f = &_tb;
    }
    else if (2 == functor)
    {
        f = &_tc;
    }
    else if (3 == functor)
    {
        f = &_td;
    }
    if (0 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 80;
        return f->intensity.trip.pull_in;
    }
    else if (1 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 80;
        return f->intensity.trip.drop_out;
    }
    else if (2 == attribute)
    {
        return f->intensity.trip.high;
    }
    else if (3 == attribute)
    {
        if (min)
            *min = 1;
        if (max)
            *max = 100;
        return f->intensity.normalization.value;
    }
    else if (4 == attribute)
    {
        return f->intensity.normalization.high;
    }
    else if (5 == attribute)
    {
        if (min)
            *min = 0;
        if (max)
            *max = 10;
        return f->intensity.filter;
    }
    else if (6 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 100;
        return f->frequency.trip.pull_in;
    }
    else if (7 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 100;
        return f->frequency.trip.drop_out;
    }
    else if (8 == attribute)
    {
        return f->frequency.trip.high;
    }
    else if (9 == attribute)
    {
        if (min)
            *min = 1;
        if (max)
            *max = 100;
        return f->frequency.normalization.value;
    }
    else if (10 == attribute)
    {
        return f->frequency.normalization.high;
    }
    else if (11 == attribute)
    {
        if (min)
            *min = 0;
        if (max)
            *max = 10;
        return f->frequency.filter;
    }
    else if (12 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 80;
        return f->amplitude.trip.pull_in;
    }
    else if (13 == attribute)
    {
        if (min)
            *min = 5;
        if (max)
            *max = 80;
        return f->amplitude.trip.drop_out;
    }
    else if (14 == attribute)
    {
        return f->amplitude.trip.high;
    }
    else if (15 == attribute)
    {
        if (min)
            *min = 0;
        if (max)
            *max = 100;
        return f->amplitude.normalization.value;
    }
    else if (16 == attribute)
    {
        return f->amplitude.normalization.high;
    }
    else if (17 == attribute)
    {
        if (min)
            *min = 0;
        if (max)
            *max = 10;
        return f->amplitude.filter;
    }
    else if (18 == attribute)
    {
        return f->frequency.max;
    }
    else if (19 == attribute)
    {
        if (min)
            *min = 2;
        if (max)
            *max = 40;
        return f->delay.drop_out;
    }
    else if (20 == attribute)
    {
        if (min)
            *min = 2;
        if (max)
            *max = 100;
        return f->delay.pull_in;
    }
    else if (21 == attribute)
    {
        if (min)
            *min = 10;
        if (max)
            *max = 100;
        return f->frequency.sensitivity;
    }
    else if (22 == attribute)
    {
        return f->quality_threshold;
    }
	return 0;
}

void function_set_default_load(void)
{
    memcpy(&_fsa, &_default, sizeof(_default));
    memcpy(&_fsb, &_default, sizeof(_default));
    memcpy(&_fsc, &_default, sizeof(_default));
    memcpy(&_fsd, &_default, sizeof(_default));
    _switch = FUNCTION_SET_SWITCH_OFF;
    _active_ab = 0;
    _active_cd = 0;
    _alternative_flame_logic = 0;
    _high_limit_enable = 0;
    _ac_amplitude_enable = 0;
    
    memcpy(&_ta, &_fsa, sizeof(_ta));
    memcpy(&_tb, &_fsb, sizeof(_tb));
    memcpy(&_tc, &_fsc, sizeof(_tc));
    memcpy(&_td, &_fsd, sizeof(_td));
    _switch_temp = _switch;
    _high_limit_enable_temp = _high_limit_enable;
    _ac_amplitude_enable_temp = _ac_amplitude_enable;
    
    eeprom_write(EEPROM_SETTINGS_FSA, &_fsa, sizeof(_fsa));
    eeprom_write(EEPROM_SETTINGS_FSB, &_fsb, sizeof(_fsb));
    eeprom_write(EEPROM_SETTINGS_FSC, &_fsc, sizeof(_fsc));
    eeprom_write(EEPROM_SETTINGS_FSD, &_fsd, sizeof(_fsd));
    eeprom_write(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
    if (FUNCTION_SET_SWITCH_SERIAL_LINE == _switch)
    {
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
        eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
    }
    eeprom_write(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
    eeprom_write(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
    eeprom_write(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
}

void function_set_configure_apply(void)
{
    memcpy(&_fsa, &_ta, sizeof(_ta));
    memcpy(&_fsb, &_tb, sizeof(_tb));
    memcpy(&_fsc, &_tc, sizeof(_tc));
    memcpy(&_fsd, &_td, sizeof(_td));
    _switch = _switch_temp;
    _high_limit_enable = _high_limit_enable_temp;
    _ac_amplitude_enable = _ac_amplitude_enable_temp;
    
    eeprom_write(EEPROM_SETTINGS_FSA, &_fsa, sizeof(_fsa));
    eeprom_write(EEPROM_SETTINGS_FSB, &_fsb, sizeof(_fsb));
    eeprom_write(EEPROM_SETTINGS_FSC, &_fsc, sizeof(_fsc));
    eeprom_write(EEPROM_SETTINGS_FSD, &_fsd, sizeof(_fsd));
    eeprom_write(EEPROM_SETTINGS_FS_SWITCH, &_switch, 1);
    eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_AB, &_active_ab, 1);
    eeprom_write(EEPROM_SETTINGS_FS_ACTIVE_CD, &_active_cd, 1);
    eeprom_write(EEPROM_SETTINGS_FS_ALTER_FLAME_LOGIC, &_alternative_flame_logic, 1);
    eeprom_write(EEPROM_SETTINGS_FS_HIGH_LIMIT_ENABLE, &_high_limit_enable, 1);
    eeprom_write(EEPROM_SETTINGS_FS_AC_AMPLITUDE_ENABLE, &_ac_amplitude_enable, 1);
}
