#include "led.h"
#include "main.h"
#include "timer.h"

static struct {
    GPIO_TypeDef *gpio;
    uint16_t pin;
    uint16_t status;
    uint16_t repeat;
}_leds[] = {
    {
        .gpio = LED1_R_GPIO_Port,
        .pin = LED1_R_Pin,
        .status = 0,
        .repeat = 0,
    },
    {
        .gpio = LED1_G_GPIO_Port,
        .pin = LED1_G_Pin,
        .status = 0,
        .repeat = 0,
    },
    {
        .gpio = LED2_R_GPIO_Port,
        .pin = LED2_R_Pin,
        .status = 0,
        .repeat = 0,
    },
    {
        .gpio = LED2_G_GPIO_Port,
        .pin = LED2_G_Pin,
        .status = 0,
        .repeat = 0,
    },
};
static uint32_t _timer = 1;

void led_init(void)
{
}

void led_update(void)
{
    if (timer_diff(_timer) <= 100)
    {
        _timer = timer_start();

        for (uint8_t led = 0; led < MAX_LED; led++)
        {
            if (_leds[led].status)
            {
                HAL_GPIO_WritePin(_leds[led].gpio, _leds[led].pin, _leds[led].status & 1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
                _leds[led].status >>= 1;
            }
            else
            {
                HAL_GPIO_WritePin(_leds[led].gpio, _leds[led].pin, GPIO_PIN_SET);
            }
        }
    }
}

void led_set(uint8_t led, uint16_t status, uint16_t repeat)
{
    _leds[led].status = status;
    _leds[led].repeat = repeat;
}
