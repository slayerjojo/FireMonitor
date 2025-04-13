#include "function_set.h"
#include "main.h"
#include "led.h"

void function_set_init(void)
{
    function_set_get();
}

void function_set_update(void)
{
}

uint8_t function_set_get(void)
{
    uint8_t di1 = !!HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin);
    uint8_t di2 = !!HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin);

    led_set(LED_1_RED, di1 ? 0xffff : 0x0000, 0xffff);
    led_set(LED_2_RED, di2 ? 0xffff : 0x0000, 0xffff);

    SEGGER_RTT_printf(0, "function_set %u %u\n", di1, di2);
    return (di2 << 1) + di1;
}

uint8_t function_set_temp_get(uint8_t functor, uint8_t attribute, uint16_t *min, uint16_t *max)
{
    return 0;
}

void function_set_temp_set(uint8_t functor, uint8_t attribute, uint8_t value)
{
}

void function_set_temp_default(void)
{
}

void function_set_apply(void)
{
}
