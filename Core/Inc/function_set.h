#ifndef __FUNCTION_SET_H__
#define __FUNCTION_SET_H__

#include <stdint.h>

enum {
    FUNCTION_SET_AB = 0,
    FUNCTION_SET_CD,
    MAX_FUNCTION_SET,
};

enum {
    FUNCTION_SET_SWITCH_OFF = 0,
    FUNCTION_SET_SWITCH_DIGITAL_INPUTS,
    FUNCTION_SET_SWITCH_SERIAL_LINE,
};

struct function_set {
    struct {
        struct {
            uint16_t pull_in;
            uint16_t drop_out;
            uint16_t high;
        }trip;
        struct {
            uint16_t value;
            uint16_t high;
        }normalization;
        uint16_t filter;
    }intensity;
    struct {
        struct {
            uint16_t pull_in;
            uint16_t drop_out;
            uint16_t high;
        }trip;
        struct {
            uint16_t value;
            uint16_t high;
        }normalization;
        uint16_t filter;
        uint16_t max;
        uint16_t sensitivity;
    }frequency;
    struct {
        struct {
            uint16_t pull_in;
            uint16_t drop_out;
            uint16_t high;
        }trip;
        struct {
            uint16_t value;
            uint16_t high;
        }normalization;
        uint16_t filter;
    }amplitude;
    struct {
        uint16_t pull_in;
        uint16_t drop_out;
    }delay;
    uint16_t quality_threshold;
    uint8_t function_set_id;
};

void function_set_init(void);
void function_set_update(void);

uint8_t function_set_dinput(uint8_t i);

struct function_set *function_set_get(uint8_t fs);

uint8_t function_set_active_ab(void);
uint8_t function_set_active_cd(void);
void function_set_active_ab_set(uint8_t v);
void function_set_active_cd_set(uint8_t v);
uint8_t function_set_flame_status_get(uint8_t sensor, struct function_set *fs, uint8_t prev_status);
uint8_t function_set_alternative_flame_logic_get(void);
void function_set_alternative_flame_logic_set(uint8_t v);
uint8_t function_set_switch_temp_get(void);
void function_set_switch_ts(uint8_t v);
uint8_t function_set_switch_get(void);
void function_set_switch(uint8_t v);
uint8_t is_function_set_enable_high_limit(void);
void function_set_enable_high_limit(uint8_t v);
uint8_t function_set_enable_high_limit_temp_get(void);
void function_set_enable_high_limit_ts(uint8_t v);
uint8_t is_function_set_enable_ac_amplitude(void);
void function_set_enable_ac_amplitude(uint8_t v);
uint8_t function_set_enable_ac_amplitude_temp_get(void);
void function_set_enable_ac_amplitude_ts(uint8_t v);
uint16_t function_set_attribute_get(uint8_t functor, uint8_t attribute);
void function_set_attribute_set(uint8_t functor, uint8_t attribute, uint16_t v);
void function_set_attribute_ts(uint8_t functor, uint8_t attribute, uint16_t v);
uint16_t function_set_temp_get(uint16_t functor, uint8_t attribute, uint16_t *min, uint16_t *max);
void function_set_default_load(void);
void function_set_configure_apply(void);

#endif
