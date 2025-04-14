#include "timer.h"
#include "main.h"

uint32_t timer_start()
{
    uint32_t now = HAL_GetTick();
    if (now)
        return now;
    return 1;
}

uint32_t timer_diff(uint32_t t)
{
    if (!t)
        return 0;
    uint32_t now = HAL_GetTick();
    if (now >= t)
        return now - t;
    return ((uint32_t)-1) - t + now;
}

void timer_delay(uint32_t us)
{
    us *= 100;
    while (us--)
    {
        __NOP();
    }
}
