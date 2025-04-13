#include "digit.h"
#include "main.h"

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
    LED_SEL1_GPIO_Port,
    LED_SEL2_GPIO_Port,
    LED_SEL3_GPIO_Port,
};

static uint16_t PIN_Bits[] = {
    LED_SEL1_Pin,
    LED_SEL2_Pin,
    LED_SEL3_Pin,
};

static const uint8_t digit_number_codes[] = {
    0b00111111,// 0
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,// 9
};

static const uint8_t digit_char_codes[] = {
    0b01110111,// a
    0b01111100,
    0b00111001,
    0b01011110,
    0b01111001,
    0b01110001,
    0b00111101,
    0b01110110,
    0b00000100,
    0b00011110,
    0b01111010,
    0b00111000,
    0b01010101,
    0b01010100,
    0b01011100,
    0b01110011,
    0b01100111,
    0b01010000,
    0b01101101,
    0b01111000,
    0b00111110,
    0b01111110,
    0b01101010,
    0b00110110,
    0b01101110,
    0b01001001,// z
};

static const uint8_t digit_sign_codes[] = {
    0b00000000,// space
    0b11000011,// tempareture degree
    0b01000000,// -
};

void digit_set(const char *codes)
{
    uint8_t pos = 0;
    uint8_t i = 0;
    while (codes[i] && pos < 3)
    {
        uint8_t code = codes[i];
        if ('0' <= code && code <= '9')
        {
            code = code - '0';
            code = digit_number_codes[code];
        }
        else if ('a' <= code && code <= 'z')
        {
            code = code - 'a';
            code = digit_char_codes[code];
        }
        else if ('A' <= code && code <= 'Z')
        {
            code = code - 'A';
            code = digit_char_codes[code];
        }
        else if (' ' == code)
        {
            code = digit_sign_codes[0];
        }
        else if ('^' == code)
        {
            code = digit_sign_codes[1];
        }
        else if ('-' == code)
        {
            code = digit_sign_codes[2];
        }
        i++;
        if ('.' == codes[i])
        {
            code |= 0b10000000;
            i++;
        }
        
        HAL_GPIO_WritePin(GPIO_Bits[pos], PIN_Bits[pos], GPIO_PIN_SET);
        for (uint8_t seg = 0; seg < 8; seg++)
        {
            HAL_GPIO_WritePin(GPIO_Segs[seg], PIN_Segs[seg], (code & (1 << seg)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(GPIO_Bits[pos], PIN_Bits[pos], GPIO_PIN_RESET);

        pos++;
    }
}
