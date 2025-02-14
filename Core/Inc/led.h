#ifndef __LED_H__
#define __LED_H__

#include "main.h"

enum {
    LED_1_RED = 0,
    LED_1_GREEN,
    LED_2_RED,
    LED_2_GREEN,
    MAX_LED,
};

void led_init(void);
void led_update(void);

void led_set(uint8_t led, uint16_t status, uint16_t repeat);

#endif
