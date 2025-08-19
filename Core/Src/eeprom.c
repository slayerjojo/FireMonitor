#include "eeprom.h"
#include "main.h"
#include "timer.h"

#define EEPROM_PAGE_SIZE 256
#define EEPROM_SIZE (2 * 1024 * 1024 / 8)
#define EEPROM_TIMEOUT 1000

extern I2C_HandleTypeDef hi2c2;
static uint32_t _timer = 0;

void eeprom_write(uint16_t addr, const void *data, uint16_t size)
{
    while (size)
    {
        uint32_t wait = timer_diff(_timer);
        if (wait < 20)
        {
            HAL_Delay(20 - wait);
        }

        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
        uint16_t length = size;
        if ((addr + length) / EEPROM_PAGE_SIZE > addr / EEPROM_PAGE_SIZE)
            length = (addr / EEPROM_PAGE_SIZE + 1) * EEPROM_PAGE_SIZE - addr;
        HAL_I2C_Mem_Write(&hi2c2, 0xa0, addr, I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, length, EEPROM_TIMEOUT);
        addr += length;
        data += length;
        size -= length;
        
        timer_delay(10);
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);

        _timer = timer_start();
    }
}

void eeprom_read(uint16_t addr, void *data, uint16_t size)
{
    uint32_t wait = timer_diff(_timer);
    if (wait < 20)
    {
        HAL_Delay(20 - wait);
    }
    while (size)
    {
        uint16_t length = size;
        if ((addr + length) / EEPROM_PAGE_SIZE > addr / EEPROM_PAGE_SIZE)
            length = (addr / EEPROM_PAGE_SIZE + 1) * EEPROM_PAGE_SIZE - addr;
        HAL_I2C_Mem_Read(&hi2c2, 0xa1, addr, I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, length, EEPROM_TIMEOUT);
        addr += length;
        data += length;
        size -= length;
    }
}
