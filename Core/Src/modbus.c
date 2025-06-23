#include "modbus.h"
#include "eeprom.h"
#include "main.h"
#include "crc16.h"
#include "usart.h"
#include "function_set.h"
#include "operating_mode.h"
#include "relay.h"
#include "error.h"
#include "analog_output.h"
#include "sensor.h"
#include "tempareture.h"
#include <string.h>

static uint8_t _temp[2] = {1, 1};
static uint8_t _address[2] = {1, 1};
static uint8_t _configure_count = 0;
static uint8_t _autotuning = 0;
static uint16_t _id[4 * 7] = {'U', '1', 0, 0, };
static uint16_t _last_id[4 * 7] = {'U', '1', 0, 0};
static uint16_t _unknown_register[] = {0};
static uint8_t _resp[3 + 125 * 2 + 2] = {0};

struct modbus_register
{
    uint16_t reg;
    uint16_t gap;
    uint16_t (*reader)(struct modbus_register *r, uint16_t reg);
    void (*writer)(struct modbus_register *r, uint16_t reg, uint16_t v);
    union {
        struct {
            uint8_t start;
        }id;
        struct {
            uint8_t sensor;
            uint8_t secondary;
        }flame_status;
        struct {
            uint8_t functor;
        }function_set;
        struct {
            uint8_t id;
        }active_function_set;
        struct {
            uint8_t type;
        }is_fault;
        struct{
            uint16_t lo;
            uint16_t hi;
        }combine;
        struct{
            uint8_t id;
            uint8_t secondary;
        }sensor;
        struct {
            uint16_t offset;
        }unknown;
        uint8_t relay;
    };
};

uint16_t modbus_register_reader_unknown(struct modbus_register *r, uint16_t reg)
{
    SEGGER_RTT_printf(1, "read unknown register:%u v:%u\n", reg, _unknown_register[r->unknown.offset]);
    return _unknown_register[r->unknown.offset];
}

void modbus_register_writer_unknown(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    SEGGER_RTT_printf(1, "writer unknown register:%u old:%u -> v:%u\n", reg, _unknown_register[r->unknown.offset], v);
    _unknown_register[r->unknown.offset] = v;
}

uint16_t modbus_register_reader_flame_status(struct modbus_register *r, uint16_t reg)
{
    return relay_flame_status_get(r->flame_status.secondary);
}

uint16_t modbus_register_reader_is_function_set_used(struct modbus_register *r, uint16_t reg)
{
    if (0 == r->function_set.functor)
        return 0 == function_set_active_ab();
    else if (1 == r->function_set.functor)
        return 1 == function_set_active_ab();
    if (2 == r->function_set.functor)
        return 0 == function_set_active_cd();
    return 1 == function_set_active_cd();
}

uint16_t modbus_register_reader_is_fault(struct modbus_register *r, uint16_t reg)
{
    return is_error(r->is_fault.type);
}

uint16_t modbus_register_reader_is_factory_default(struct modbus_register *r, uint16_t reg)
{
    return _configure_count;
}

uint16_t modbus_register_reader_is_flame_relay_overridden(struct modbus_register *r, uint16_t reg)
{
    return flame_relay_overridden();
}

void modbus_register_writer_flame_relay_overridden(struct modbus_register *r, uint16_t v, uint16_t reg)
{
    flame_relay_overridden_set(!!v);
}

uint16_t modbus_register_reader_is_autotune_procedure(struct modbus_register *r, uint16_t reg)
{
    return _autotuning;
}

void modbus_register_writer_autotune_start(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    _autotuning = v;
}

uint16_t modbus_register_reader_is_safe_relay_used_for_safe(struct modbus_register *r, uint16_t reg)
{
    return SAFE_RELAY_USAGE_SAFE == safe_relay_usage();
}

void modbus_register_writer_clear_counters(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    relay_clear_counters();
}

uint16_t modbus_register_reader_sensor_count(struct modbus_register *r, uint16_t reg)
{
    return 1;
}

uint16_t modbus_register_reader_alternative_flame_logic(struct modbus_register *r, uint16_t reg)
{
    return function_set_alternative_flame_logic_get();
}

void modbus_register_writer_alternative_flame_logic(struct modbus_register *r, uint16_t v, uint16_t reg)
{
    function_set_alternative_flame_logic_set(!!v);
}

uint16_t modbus_register_reader_safe_relay_usage(struct modbus_register *r, uint16_t reg)
{
    return safe_relay_usage();
}

void modbus_register_writer_safe_relay_usage(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    function_set_alternative_flame_logic_set(!!v);
}

uint16_t modbus_register_reader_operating_mode(struct modbus_register *r, uint16_t reg)
{
    return operating_mode_get();
}

void modbus_register_writer_operating_mode(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    operating_mode_set(v);
}

uint16_t modbus_register_reader_function_set_switch(struct modbus_register *r, uint16_t reg)
{
    return function_set_switch_get();
}

void modbus_register_writer_function_set_switch(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    function_set_switch(v);
}

uint16_t modbus_register_reader_enable_high_limit(struct modbus_register *r, uint16_t reg)
{
    return is_function_set_enable_high_limit();
}

void modbus_register_writer_enable_high_limit(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    function_set_enable_high_limit(!!v);
}

uint16_t modbus_register_reader_enable_ac_amplitude(struct modbus_register *r, uint16_t reg)
{
    return is_function_set_enable_ac_amplitude();
}

void modbus_register_writer_enable_ac_amplitude(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    function_set_enable_ac_amplitude(!!v);
}

uint16_t modbus_register_reader_id(struct modbus_register *r, uint16_t reg)
{
    return _id[r->id.start + reg - r->reg];
}

void modbus_register_writer_id(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    _last_id[r->id.start + reg - r->reg] = _id[r->id.start + reg - r->reg];
    _id[r->id.start + reg - r->reg] = v;
}

uint16_t modbus_register_reader_last_id(struct modbus_register *r, uint16_t reg)
{
    return _last_id[r->id.start + reg - r->reg];
}

uint16_t modbus_register_reader_ao_mode(struct modbus_register *r, uint16_t reg)
{
    return analog_output_mode_get();
}

void modbus_register_writer_ao_mode(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    analog_output_mode_set(v);
}

uint16_t modbus_register_reader_ao_source(struct modbus_register *r, uint16_t reg)
{
    return analog_output_source_get();
}

void modbus_register_writer_ao_source(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    analog_output_source_set(v);
}

uint16_t modbus_register_reader_active_function_set(struct modbus_register *r, uint16_t reg)
{
    if (0 == r->active_function_set.id)
        return function_set_active_ab();
    return function_set_active_cd();
}

void modbus_register_writer_active_function_set(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    if (0 == r->active_function_set.id)
        function_set_active_ab_set(v);
    else
        function_set_active_cd_set(v);
}

uint16_t modbus_register_reader_safe_temperature_threshold(struct modbus_register *r, uint16_t reg)
{
    return safe_relay_temperature_threshold_get();
}

void modbus_register_writer_safe_temperature_threshold(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    safe_relay_temperature_threshold_set(v);
}

uint16_t modbus_register_reader_function_set_attribute(struct modbus_register *r, uint16_t reg)
{
    return function_set_attribute_get(r->function_set.functor, reg - r->reg);
}

void modbus_register_writer_function_set_attribute(struct modbus_register *r, uint16_t reg, uint16_t v)
{
    function_set_attribute_set(r->function_set.functor, reg - r->reg, v);
}

uint16_t modbus_register_reader_download(struct modbus_register *r, uint16_t reg)
{
    return 0;
}

void modbus_register_writer_download(struct modbus_register *r, uint16_t reg, uint16_t action)
{
}

static uint16_t modbus_register_reader_combine(struct modbus_register *r, uint16_t reg);

uint16_t modbus_register_reader_sensor_intensity_and_flicker_frequency(struct modbus_register *r, uint16_t reg)
{
    struct function_set *f = function_set_get(r->sensor.secondary);
    uint16_t v = 0;
    v += sensor_intensity_get(r->sensor.id, f->intensity.filter);
    v += sensor_flicker_frequency_get(r->sensor.id, f->frequency.filter, f->frequency.sensitivity, f->frequency.max) << 8;
    return v;
}

uint16_t modbus_register_reader_sensor_amplitude_and_quality(struct modbus_register *r, uint16_t reg)
{
    struct function_set *f = function_set_get(r->sensor.secondary);
    uint16_t v = 0;
    v += sensor_amplitude_get(r->sensor.id, f->amplitude.filter);
    v += sensor_quality_get(r->sensor.id, f) << 8;
    return v;
}

uint16_t modbus_register_reader_sensor_spare_and_type(struct modbus_register *r, uint16_t reg)
{
    uint16_t v = 0;
    v += sensor_type_get() << 8;
    return v;
}

uint16_t modbus_register_reader_sensor_channel_1_raw(struct modbus_register *r, uint16_t reg)
{
    return sensor_intensity_raw_get(0);
}

uint16_t modbus_register_reader_sensor_type(struct modbus_register *r, uint16_t reg)
{
    return sensor_type_get();
}

uint16_t modbus_register_reader_temperature(struct modbus_register *r, uint16_t reg)
{
    return core_tempareture_get();
}

uint16_t modbus_register_reader_fw_major(struct modbus_register *r, uint16_t reg)
{
    return 'C';
}

uint16_t modbus_register_reader_fw_minor(struct modbus_register *r, uint16_t reg)
{
    return '3';
}

uint16_t modbus_register_reader_FPGA_major(struct modbus_register *r, uint16_t reg)
{
    return 'B';
}

uint16_t modbus_register_reader_FPGA_minor(struct modbus_register *r, uint16_t reg)
{
    return '1';
}

uint16_t modbus_register_reader_sensor_intensity_raw_get(struct modbus_register *r, uint16_t reg)
{
    return sensor_intensity_raw_get(r->sensor.id);
}

uint16_t modbus_register_reader_relay_action_count(struct modbus_register *r, uint16_t reg)
{
    return relay_action_count(r->relay);
}

static struct modbus_register _registers[] = {
    {
        .reg = 3000,
        .reader = modbus_register_reader_flame_status,
        .flame_status.sensor = 0,
    },
    {
        .reg = 3001,
        .reader = modbus_register_reader_flame_status,
        .flame_status.sensor = 1,
        .flame_status.secondary = 1,
    },
    {
        .reg = 3002,
        .reader = modbus_register_reader_is_function_set_used,
        .function_set.functor = 0,
    },
    {
        .reg = 3003,
        .reader = modbus_register_reader_is_function_set_used,
        .function_set.functor = 1,
    },
    {
        .reg = 3004,
        .reader = modbus_register_reader_is_function_set_used,
        .function_set.functor = 2,
    },
    {
        .reg = 3005,
        .reader = modbus_register_reader_is_function_set_used,
        .function_set.functor = 3,
    },
    {
        .reg = 3006,
        .reader = modbus_register_reader_is_fault,
        .is_fault.type = FAULT_TYPE_COMBINED_UNIT,
    },
    {
        .reg = 3007,
        .reader = modbus_register_reader_is_fault,
        .is_fault.type = FAULT_TYPE_SCANNER,
    },
    {
        .reg = 3008,
        .reader = modbus_register_reader_is_fault,
        .is_fault.type = FAULT_TYPE_SENSOR_0,
    },
    {
        .reg = 3009,
        .reader = modbus_register_reader_is_fault,
        .is_fault.type = FAULT_TYPE_SENSOR_1,
    },
    {
        .reg = 3010,
        .reader = modbus_register_reader_is_factory_default,
    },
    {
        .reg = 3011,
        .reader = modbus_register_reader_is_flame_relay_overridden,
    },
    {
        .reg = 3012,
        .reader = modbus_register_reader_is_autotune_procedure,
    },
    {
        .reg = 3013,
        .reader = modbus_register_reader_flame_status,
        .flame_status.sensor = 0,
        .flame_status.secondary = 1,
    },
    {
        .reg = 3014,
        .reader = modbus_register_reader_is_safe_relay_used_for_safe,
    },
    {
        .reg = 3101,
        .writer = modbus_register_writer_autotune_start,
    },
    {
        .reg = 3108,
        .writer = modbus_register_writer_flame_relay_overridden,
    },
    {
        .reg = 3109,
        .writer = modbus_register_writer_clear_counters,
    },
    {
        .reg = 4007,
        .reader = modbus_register_reader_sensor_count,
    },
    {
        .reg = 4008,
        .reader = modbus_register_reader_alternative_flame_logic,
        .writer = modbus_register_writer_alternative_flame_logic,
    },
    {
        .reg = 4009,
        .reader = modbus_register_reader_safe_relay_usage,
        .writer = modbus_register_writer_safe_relay_usage,
    },
    {
        .reg = 4010,
        .reader = modbus_register_reader_operating_mode,
        .writer = modbus_register_writer_operating_mode,
    },
    {
        .reg = 4011,
        .reader = modbus_register_reader_function_set_switch,
        .writer = modbus_register_writer_function_set_switch,
    },
    {
        .reg = 4012,
        .reader = modbus_register_reader_enable_high_limit,
        .writer = modbus_register_writer_enable_high_limit,
    },
    {
        .reg = 4013,
        .reader = modbus_register_reader_enable_ac_amplitude,
        .writer = modbus_register_writer_enable_ac_amplitude,
    },
    {
        .reg = 4014,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 0,
    },
    {
        .reg = 4018,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 4,
    },
    {
        .reg = 4022,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 8,
    },
    {
        .reg = 4026,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 12,
    },
    {
        .reg = 4030,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 16,
    },
    {
        .reg = 4034,
        .reader = modbus_register_reader_ao_mode,
        .writer = modbus_register_writer_ao_mode,
    },
    {
        .reg = 4035,
        .reader = modbus_register_reader_ao_source,
        .writer = modbus_register_writer_ao_source,
    },
    {
        .reg = 4036,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 0,
    },
    {
        .reg = 4037,
        .reader = modbus_register_reader_active_function_set,
        .writer = modbus_register_writer_active_function_set,
        .active_function_set.id = 0,
    },
    {
        .reg = 4038,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 1,
    },
    {
        .reg = 4039,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 2,
    },
    {
        .reg = 4040,
        .reader = modbus_register_reader_safe_temperature_threshold,
        .writer = modbus_register_writer_safe_temperature_threshold,
    },
    {
        .reg = 4041,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 3,
    },
    {
        .reg = 4042,
        .reader = modbus_register_reader_active_function_set,
        .writer = modbus_register_writer_active_function_set,
        .active_function_set.id = 1,
    },
    {
        .reg = 4043,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 4,
    },
    {
        .reg = 4044,
        .gap = 3,
        .reader = modbus_register_reader_last_id,
        .id.start = 0,
    },
    {
        .reg = 4048,
        .gap = 3,
        .reader = modbus_register_reader_last_id,
        .id.start = 4,
    },
    {
        .reg = 4052,
        .gap = 3,
        .reader = modbus_register_reader_last_id,
        .id.start = 8,
    },
    {
        .reg = 4056,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 20,
    },
    {
        .reg = 4060,
        .gap = 3,
        .reader = modbus_register_reader_id,
        .writer = modbus_register_writer_id,
        .id.start = 24,
    },
    {
        .reg = 4064,
        .gap = 5,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 5,
    },
    {
        .reg = 4070,
        .gap = 22,
        .reader = modbus_register_reader_function_set_attribute,
        .writer = modbus_register_writer_function_set_attribute,
        .function_set.functor = 0,
    },
    {
        .reg = 4110,
        .gap = 22,
        .reader = modbus_register_reader_function_set_attribute,
        .writer = modbus_register_writer_function_set_attribute,
        .function_set.functor = 1,
    },
    {
        .reg = 4150,
        .gap = 22,
        .reader = modbus_register_reader_function_set_attribute,
        .writer = modbus_register_writer_function_set_attribute,
        .function_set.functor = 2,
    },
    {
        .reg = 4190,
        .gap = 22,
        .reader = modbus_register_reader_function_set_attribute,
        .writer = modbus_register_writer_function_set_attribute,
        .function_set.functor = 3,
    },
    {
        .reg = 4600,
        .gap = 34,
        .reader = modbus_register_reader_unknown,
        .writer = modbus_register_writer_unknown,
        .unknown.offset = 11,
    },
    {
        .reg = 4999,
        .reader = modbus_register_reader_download,
        .writer = modbus_register_writer_download,
    },
    {
        .reg = 5100,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 3000,
            .hi = 3001,
        },
    },
    {
        .reg = 5101,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 3013,
        },
    },
    {
        .reg = 5102,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 4037,
        },
    },
    {
        .reg = 5103,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 3007,
            .hi = 3006,
        },
    },
    {
        .reg = 5104,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 3008,
            .hi = 3009,
        },
    },
    {
        .reg = 5105,
        .reader = modbus_register_reader_combine,
        .combine = {
            .lo = 3010,
        },
    },
    {
        .reg = 5106,
        .reader = modbus_register_reader_sensor_intensity_and_flicker_frequency,
        .sensor.id = 0,
        .sensor.secondary = 0,
    },
    {
        .reg = 5107,
        .reader = modbus_register_reader_sensor_amplitude_and_quality,
        .sensor.id = 0,
    },
    {
        .reg = 5112,
        .reader = modbus_register_reader_sensor_intensity_and_flicker_frequency,
        .sensor.id = 0,
        .sensor.secondary = 0,
    },
    {
        .reg = 5113,
        .reader = modbus_register_reader_sensor_amplitude_and_quality,
        .sensor.id = 0,
    },
    {
        .reg = 5118,
        .reader = modbus_register_reader_sensor_spare_and_type,
    },
    {
        .reg = 0x7f00,
        .reader = modbus_register_reader_sensor_channel_1_raw,
    },
    {
        .reg = 0x7f0b,
        .reader = modbus_register_reader_sensor_type,
    },
    {
        .reg = 0x7f0c,
        .reader = modbus_register_reader_temperature,
    },
    {
        .reg = 0x7f0d,
        .reader = modbus_register_reader_fw_major,
    },
    {
        .reg = 0x7f0e,
        .reader = modbus_register_reader_fw_minor,
    },
    {
        .reg = 0x7f0f,
        .reader = modbus_register_reader_FPGA_major,
    },
    {
        .reg = 0x7f10,
        .reader = modbus_register_reader_FPGA_minor,
    },
    {
        .reg = 0x7f11,
        .reader = modbus_register_reader_sensor_intensity_raw_get,
        .sensor.id = 0,
    },
    {
        .reg = 0x7f12,
        .reader = modbus_register_reader_sensor_intensity_raw_get,
        .sensor.id = 1,
    },
    {
        .reg = 0x7f1f,
        .reader = modbus_register_reader_relay_action_count,
        .relay = 0,
    },
    {
        .reg = 0x7f20,
        .reader = modbus_register_reader_relay_action_count,
        .relay = 1,
    },
};

static uint16_t modbus_register_reader_combine(struct modbus_register *r, uint16_t reg)
{
    uint16_t v = 0;
    for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
    {
        if (_registers[i].reg <= r->combine.hi && r->combine.hi <= (_registers[i].reg + _registers[i].gap))
        {
            v += _registers[i].reader(&_registers[i], r->combine.hi) << 8;
        }
        if (_registers[i].reg <= r->combine.lo && r->combine.lo <= (_registers[i].reg + _registers[i].gap))
        {
            v += _registers[i].reader(&_registers[i], r->combine.lo);
        }
    }
    return v;
}

void modbus_recv(uint8_t *protocol, int size)
{
    if (_address[0] != protocol[0] && _address[1] != protocol[0])
    {
        SEGGER_RTT_printf(0, "address %u error\n", protocol[0]);
        return;
    }
    uint16_t crc = crc16(protocol, size - 2);
    if ((crc >> 8) != protocol[size - 2] || (crc & 0xff) != protocol[size - 1])
    {
        SEGGER_RTT_printf(0, "modbus_check failed. crc:%04x data:", crc);
        for (int i = 0; i < size; i++)
        {
            SEGGER_RTT_printf(0, "%02x ", protocol[i]);
        }
        SEGGER_RTT_printf(0, "\n");
        return;
    }
    if (0x01 == protocol[1] || 0x02 == protocol[1]) //read coils
    {
        uint16_t reg = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t count = ((uint16_t)protocol[4] << 8) + protocol[5];
        
        SEGGER_RTT_printf(0, "read coils reg:%u count:%u\n", reg, count);

        _resp[0] = protocol[0];
        _resp[1] = protocol[1];
        _resp[2] = count / 8 + !!(count % 8);
        for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
        {
            for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
            {
                if (reg <= (_registers[i].reg + gap) && (_registers[i].reg + gap) < (reg + count) && _registers[i].reader)
                {
                    uint8_t pos = _registers[i].reg + gap - reg;
                    uint16_t v = _registers[i].reader(&_registers[i], _registers[i].reg + gap);
                    SEGGER_RTT_printf(0, "\treg:%u v:%u\n", _registers[i].reg + gap, v);
                    if (v)
                    {
                        _resp[3 + pos / 8] |= 1 << (pos % 8);
                    }
                }
            }
        }
        uint16_t crc = crc16(_resp, 3 + _resp[2]);
        _resp[3 + _resp[2]] = crc >> 8;
        _resp[3 + _resp[2] + 1] = crc;
        usart_send(_resp, 3 + _resp[2] + 2);
    }
    else if (0x03 == protocol[1] || 0x04 == protocol[1])
    {
        uint16_t reg = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t count = ((uint16_t)protocol[4] << 8) + protocol[5];

        SEGGER_RTT_printf(0, "read holding/input registers reg:%u count:%u\n", reg, count);

        _resp[0] = protocol[0];
        _resp[1] = protocol[1];
        _resp[2] = count * 2;
        for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
        {
            for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
            {
                if (reg <= (_registers[i].reg + gap) && (_registers[i].reg + gap) < (reg + count) && _registers[i].reader)
                {
                    uint8_t pos = _registers[i].reg + gap - reg;
                    uint16_t v = _registers[i].reader(&_registers[i], _registers[i].reg + gap);
                    SEGGER_RTT_printf(0, "\treg:%u v:%u\n", _registers[i].reg + gap, v);
                    if (v)
                    {
                        _resp[3 + pos * 2 + 0] = v >> 8;
                        _resp[3 + pos * 2 + 1] = v;
                    }
                }
            }
        }
        uint16_t crc = crc16(_resp, 3 + _resp[2]);
        _resp[3 + _resp[2]] = crc >> 8;
        _resp[3 + _resp[2] + 1] = crc;
        usart_send(_resp, 3 + _resp[2] + 2);
    }
    else if (0x05 == protocol[1])
    {
        uint16_t reg = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t v = ((uint16_t)protocol[4] << 8) + protocol[5];

        SEGGER_RTT_printf(0, "write single coil reg:%u value:%u\n", reg, v);

        for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
        {
            for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
            {
                if ((_registers[i].reg + gap) == reg && _registers[i].writer)
                {
                    _registers[i].writer(&_registers[i], _registers[i].reg + gap, !!v);
                }
            }
        }
        usart_send(protocol, size);
    }
    else if (0x06 == protocol[1])
    {
        uint16_t reg = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t v = ((uint16_t)protocol[4] << 8) + protocol[5];
        
        SEGGER_RTT_printf(0, "write single register:%u value:%u\n", reg, v);

        for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
        {
            for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
            {
                if ((_registers[i].reg + gap) == reg && _registers[i].writer)
                {
                    SEGGER_RTT_printf(0, "\treg:%u v:%u\n", _registers[i].reg + gap, v);
                    _registers[i].writer(&_registers[i], _registers[i].reg + gap, v);
                }
            }
        }
        usart_send(protocol, size);
    }
    else if (0x0f == protocol[1])
    {
        uint16_t start = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t count = ((uint16_t)protocol[4] << 8) + protocol[5];
        uint8_t length = protocol[6];

        SEGGER_RTT_printf(0, "write multiple coils start:%u count:%u length:%u\n", start, count, length);

        uint8_t pos = 0;
        for (int reg = start; reg < start + count; reg++)
        {
            uint16_t v = protocol[7 + pos / 8] & (1 << pos % 8);
            pos++;
        
            SEGGER_RTT_printf(0, "\treg:%u value:%u\n", reg, v);

            for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
            {
                for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
                {
                    if ((_registers[i].reg + gap) == reg && _registers[i].writer)
                    {
                        SEGGER_RTT_printf(0, "\treg:%u v:%u\n", _registers[i].reg + gap, v);
                        _registers[i].writer(&_registers[i], _registers[i].reg + gap, v);
                    }
                }
            }
        }
        usart_send(protocol, 6);
    }
    else if (0x10 == protocol[1])
    {
        uint16_t start = ((uint16_t)protocol[2] << 8) + protocol[3];
        uint16_t count = ((uint16_t)protocol[4] << 8) + protocol[5];
        uint16_t length = ((uint16_t)protocol[6] << 8) + protocol[7];
        
        SEGGER_RTT_printf(0, "write multiple register start:%u count:%u length:%u\n", start, count, length);

        uint8_t pos = 0;
        for (int reg = start; reg < start + count; reg++)
        {
            for (int i = 0; i < sizeof(_registers) / sizeof(_registers[0]); i++)
            {
                for (uint8_t gap = 0; gap <= _registers[i].gap; gap++)
                {
                    if ((_registers[i].reg + gap) == reg && _registers[i].writer)
                    {
                        uint16_t v = (((uint16_t)protocol[8 + pos * 2]) << 8) + protocol[8 + pos * 2 + 1];
                        pos++;

                        SEGGER_RTT_printf(0, "\treg:%u value:%u\n", reg, v);
                        _registers[i].writer(&_registers[i], _registers[i].reg + gap, v);
                    }
                }
            }
        }
        usart_send(protocol, 6);
    }
}

uint8_t modbus_address_get(uint8_t idx)
{
    return _address[idx];
}

uint8_t modbus_address_temp_get(uint8_t idx)
{
    return _temp[idx];
}

void modbus_address_ts(uint8_t idx, uint8_t address)
{
    _temp[idx] = address;
}

void modbus_address_apply(void)
{
    memcpy(_address, _temp, sizeof(_temp));
    eeprom_write(EEPROM_SETTINGS_MODBUS_ADDR, _address, sizeof(_address));
}

void modbus_init(void)
{
    eeprom_read(EEPROM_SETTINGS_MODBUS_ADDR, _address, 2);
    if (_address[0] == 0xff || _address[1] == 0xff)
    {
        _address[0] = _address[1] = 1;
    }
    memcpy(_temp, _address, sizeof(_address));
    SEGGER_RTT_printf(0, "modbus initialized. address:%u %u\n", _address[0], _address[1]);
}
