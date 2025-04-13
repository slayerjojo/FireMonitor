#include "eeprom.h"
#include "main.h"

#define EEPROM_PAGE_SIZE 256
#define EEPROM_SIZE (2 * 1024 * 1024 / 8)
#define EEPROM_TIMEOUT 1000

extern I2C_HandleTypeDef hi2c2;

void eeprom_write(uint16_t addr, const uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
    uint16_t pos = 0;
    while (size)
    {
        uint16_t length = size;
        if (length > EEPROM_PAGE_SIZE - pos % EEPROM_PAGE_SIZE)
            length = EEPROM_PAGE_SIZE - pos % EEPROM_PAGE_SIZE;
        HAL_I2C_Mem_Write(&hi2c2, 0xa0, addr + pos, I2C_MEMADD_SIZE_16BIT, (uint8_t *)data + pos, length, EEPROM_TIMEOUT);
        pos += length;
        size -= length;
    }
    HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
}

void eeprom_read(uint16_t addr, uint8_t *data, uint16_t size)
{
    uint16_t pos = 0;
    while (size)
    {
        uint16_t length = size;
        if (length > EEPROM_PAGE_SIZE - pos % EEPROM_PAGE_SIZE)
            length = EEPROM_PAGE_SIZE - pos % EEPROM_PAGE_SIZE;
        HAL_I2C_Mem_Read(&hi2c2, 0xa1, addr + pos, I2C_MEMADD_SIZE_16BIT, data + pos, length, EEPROM_TIMEOUT);
        pos += length;
        size -= length;
    }
}
