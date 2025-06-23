#include "digit.h"
#include "main.h"
#include "timer.h"
#include <string.h>

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

static uint8_t _sel = 0;
static uint8_t _pos = 0;
static uint32_t _timer = 1;

static char _codes[7] = {0};

void digit_set(const char *codes)
{
    strcpy(_codes, codes);
}

void digit_init(void)
{
    _pos = 0;
    _sel = 0;
}

void digit_update(void)
{
    if (timer_diff(_timer) < 5)
        return;
    _timer = timer_start();

    if (_sel >= 3)
    {
        _sel = 0;
        _pos = 0;
    }
        
    uint8_t code = _codes[_pos];
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
    _pos++;
    if ('.' == _codes[_pos])
    {
        code |= 0b10000000;
        _pos++;
    }
    code = ~code;

    HAL_GPIO_WritePin(GPIO_Bits[0], PIN_Bits[0], GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO_Bits[1], PIN_Bits[1], GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO_Bits[2], PIN_Bits[2], GPIO_PIN_SET);
    for (uint8_t seg = 0; seg < 8; seg++)
    {
        HAL_GPIO_WritePin(GPIO_Segs[seg], PIN_Segs[seg], (code & (1 << seg)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(GPIO_Bits[_sel], PIN_Bits[_sel], GPIO_PIN_RESET);
    _sel++;
}
