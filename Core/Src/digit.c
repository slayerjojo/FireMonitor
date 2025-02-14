#include "digit.h"

static GPIO_TypeDef *GPIO_Segs[] = {
    LED_SEGA_GPIO_Port,
    LED_SEGB_GPIO_Port,
    LED_SEGC_GPIO_Port,
    LED_SEGD_GPIO_Port,
    LED_SEGE_GPIO_Port,
    LED_SEGF_GPIO_Port,
    LED_SEGG_GPIO_Port,
    LED_SEGDP_GPIO_Port,
};
static uint16_t PIN_Segs[] = {
    LED_SEGA_Pin,
    LED_SEGB_Pin,
    LED_SEGC_Pin,
    LED_SEGD_Pin,
    LED_SEGE_Pin,
    LED_SEGF_Pin,
    LED_SEGG_Pin,
    LED_SEGDP_Pin,
};

static GPIO_TypeDef *GPIO_Bits[] = {
    LED_SEG1_GPIO_Port,
    LED_SEG2_GPIO_Port,
    LED_SEG3_GPIO_Port,
};
static uint16_t PIN_Bits[] = {
    LED_SEG1_Pin,
    LED_SEG2_Pin,
    LED_SEG3_Pin,
};

void digit_set(uint8_t *codes)
{
    uint8_t i = 0;
    for (i = 0; i < 3; i++)
    {
        HAL_GPIO_WritePin(GPIO_Bits[i], PIN_Bits[i], GPIO_PIN_SET);
        uint8_t seg = 0;
        for (seg = 0; seg < 8; seg++)
        {
            HAL_GPIO_WritePin(GPIO_Segs[seg], PIN_Segs[seg], codes[i] & (1 << seg) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(GPIO_Bits[i], PIN_Bits[i], GPIO_PIN_RESET);
    }
}
