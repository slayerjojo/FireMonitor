#ifndef __TIMER_H__
#define __TIMER_H__

#include "main.h"

uint32_t timer_start(void);
uint32_t timer_diff(uint32_t t);
void timer_sleep(uint32_t ms);
void timer_delay(uint32_t us);

#endif
