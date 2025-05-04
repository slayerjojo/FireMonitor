#include "relay.h"
#include "timer.h"
#include "error.h"

static struct {
    uint32_t action;
    uint32_t feedback;
    uint32_t delay;
}_frame_pickup = {
    .action = 0,
    .feedback = 0,
    .delay = 2000,
}, _frame_dropout = {
    .action = 0,
    .feedback = 0,
    .delay = 2000,
};

static struct {
    uint32_t feedback;
    uint8_t v;
}_safe = {0};

void relay_init(void)
{
    SEGGER_RTT_printf(0, "relay initialized\n");
}

void relay_update(void)
{
    if (timer_diff(_frame_pickup.action) > _frame_pickup.delay)
    {
        SEGGER_RTT_printf(0, "frame pickup action\n");

        _frame_pickup.action = 0;
        HAL_GPIO_WritePin(RELAY_FRAME_GPIO_Port, RELAY_FRAME_Pin, GPIO_PIN_SET);
        _frame_pickup.feedback = timer_start();
    }
    if (timer_diff(_frame_pickup.feedback) > 100)
    {
        SEGGER_RTT_printf(0, "frame pickup feedback\n");
        _frame_pickup.feedback = 0;
        if (GPIO_PIN_SET != HAL_GPIO_ReadPin(RELAY_FRAME_FB_GPIO_Port, RELAY_FRAME_FB_Pin))
            error_set(ERROR_RELAY_FRAME_PICKUP);
    }
    if (timer_diff(_frame_dropout.action) > _frame_dropout.delay)
    {
        SEGGER_RTT_printf(0, "frame dropout action\n");

        _frame_dropout.action = 0;
        HAL_GPIO_WritePin(RELAY_FRAME_GPIO_Port, RELAY_FRAME_Pin, GPIO_PIN_RESET);
        _frame_dropout.feedback = timer_start();
    }
    if (timer_diff(_frame_dropout.feedback) > 100)
    {
        SEGGER_RTT_printf(0, "frame dropout feedback\n");

        _frame_dropout.feedback = 0;
        if (GPIO_PIN_RESET != HAL_GPIO_ReadPin(RELAY_FRAME_FB_GPIO_Port, RELAY_FRAME_FB_Pin))
            error_set(ERROR_RELAY_FRAME_DROPOUT);
    }
    if (timer_diff(_safe.feedback) > 100)
    {
        SEGGER_RTT_printf(0, "safe feedback\n");

        _safe.feedback = 0;
        if (_safe.v != HAL_GPIO_ReadPin(RELAY_SAFE_GPIO_Port, RELAY_SAFE_Pin))
            error_set(ERROR_RELAY_SAFE);
    }
}

void relay_frame_set(uint8_t v)
{
    if (v)
    {
        if (!_frame_pickup.action)
            _frame_pickup.action = timer_start();
        _frame_dropout.action = 0;
        _frame_dropout.feedback = 0;
    }
    else
    {
        _frame_pickup.action = 0;
        _frame_pickup.feedback = 0;
        if (!_frame_dropout.action)
            _frame_dropout.action = timer_start();
    }
}

void relay_frame_pickup_set(uint8_t delay)
{
    _frame_pickup.delay = delay;
    if (!_frame_pickup.delay)
        _frame_pickup.delay = 400;
    if (_frame_pickup.delay > 10000)
        _frame_pickup.delay = 10000;
}

void relay_frame_pickup_add(int delay)
{
    if (delay < 0 && -delay > _frame_pickup.delay)
        delay = -_frame_pickup.delay;
    _frame_pickup.delay += delay;
    if (!_frame_pickup.delay)
        _frame_pickup.delay = 400;
    if (_frame_pickup.delay > 10000)
        _frame_pickup.delay = 10000;
}

void relay_frame_dropout_set(uint8_t delay)
{
    _frame_dropout.delay = delay;
    if (!_frame_dropout.delay)
        _frame_dropout.delay = 400;
    if (_frame_dropout.delay > 10000)
        _frame_dropout.delay = 10000;
}

void relay_frame_dropout_add(int delay)
{
    if (delay < 0 && -delay > _frame_dropout.delay)
        delay = -_frame_dropout.delay;
    _frame_dropout.delay += delay;
    if (!_frame_dropout.delay)
        _frame_dropout.delay = 400;
    if (_frame_dropout.delay > 10000)
        _frame_dropout.delay = 10000;
}

void relay_safe_set(uint8_t v)
{
    SEGGER_RTT_printf(0, "relay safe set:%u\n", v);
    if (v)
    {
        _safe.v = GPIO_PIN_SET;
    }
    else
    {
        _safe.v = GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(RELAY_SAFE_GPIO_Port, RELAY_SAFE_Pin, _safe.v);
    _safe.feedback = timer_start();
}
