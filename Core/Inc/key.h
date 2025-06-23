#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>

enum {
    KEY_UP = 0,
    KEY_LEFT,
    KEY_DOWN,
    KEY_RIGHT,
    MAX_KEY,
};

enum {
    KEY_EVENT_CLICK = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_PRESS_LONG,
    KEY_EVENT_RELEASE,
};

typedef void (*KEY_HANDLER)(uint8_t key, uint8_t action);

void key_init(void);
void key_update(void);

void key_handler_set(KEY_HANDLER handler);

#endif
