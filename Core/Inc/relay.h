#ifndef __RELAY_H__
#define __RELAY_H__

#include "main.h"

enum {
    SAFE_RELAY_USAGE_SAFE = 0,
    SAFE_RELAY_USAGE_SECOND_FLAME,
    SAFE_RELAY_USAGE_QUALITY,
    SAFE_RELAY_USAGE_CORE_TEMPERATURE,
    SAFE_RELAY_USAGE_FLAME_TEMPERATURE,
};

void relay_init(void);
void relay_update(void);
uint8_t relay_flame_status_get(uint8_t id);
uint8_t flame_relay_feedback(void);
void flame_relay_set(uint8_t v);
void flame_relay_overridden_set(uint8_t overridden);
uint8_t flame_relay_overridden(void);
uint8_t safe_relay_feedback(void);
void safe_relay_set(uint8_t v);
uint16_t safe_relay_temperature_threshold_get(void);
void safe_relay_temperature_threshold_set(uint16_t v);
uint8_t safe_relay_usage(void);
void safe_relay_usage_set(uint8_t usage);
uint8_t safe_relay_usage_temp_get(void);
void safe_relay_usage_ts(uint8_t usage);
uint16_t relay_action_count(uint8_t relay);
void relay_clear_counters(void);
void relay_default_load(void);
void relay_configure_apply(void);

#endif
