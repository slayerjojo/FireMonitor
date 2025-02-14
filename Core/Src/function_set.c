#include "function_set.h"
#include "main.h"

void function_set_init(void)
{
}

void function_set_update(void)
{
}

uint8_t function_set_get(void)
{
    uint8_t dl1 = !!HAL_GPIO_ReadPin(DL1_GPIO_Port, DL1_Pin);
    uint8_t dl2 = !!HAL_GPIO_ReadPin(DL2_GPIO_Port, DL2_Pin);

    return (dl2 << 1) + dl1;
}
