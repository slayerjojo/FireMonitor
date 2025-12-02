#include "eeprom.h"
#include "main.h"
#include "timer.h"

static const uint8_t _log_level = 1;

#define EEPROM_PAGE_SIZE 256
#define EEPROM_SIZE (2 * 1024 * 1024 / 8)
#define EEPROM_TIMEOUT 1000

struct eeprom_cache
{
    struct eeprom_cache *next;
    uint16_t addr;
    uint16_t size;
    uint8_t data[];
} *_caches = 0;

extern I2C_HandleTypeDef hi2c2;
static uint32_t _timer_write = 0;
static uint32_t _timer_cache = 0;

void eeprom_write(uint16_t addr, const void *data, uint16_t size)
{
    struct eeprom_cache *cache = _caches;
    while (cache && cache->next)
    {
        if (cache->addr == addr && cache->size == size)
        {
            memcpy(cache->data, data, size);
            return;
        }
        cache = cache->next;
    }
    if (!cache)
    {
        _caches = cache = malloc(sizeof(struct eeprom_cache) + size);
        if (!cache)
        {
            LOG_ERR("out of memory %u", size);
            return;
        }
    }
    else
    {
        cache->next = malloc(sizeof(struct eeprom_cache) + size);
        if (!cache->next)
        {
            LOG_ERR("out of memory %u", size);
            return;
        }
        cache = cache->next;
    }
    cache->next = 0;
    cache->addr = addr;
    cache->size = size;
    memcpy(cache->data, data, size);
    if (!_timer_cache)
    {
        _timer_cache = timer_start();
    }
}

void eeprom_read(uint16_t addr, void *data, uint16_t size)
{
    struct eeprom_cache *cache = _caches;
    while (cache)
    {
        if (cache->addr == addr)
        {
            memcpy(data, cache->data, size);
            return;
        }
        cache = cache->next;
    }
    uint32_t wait = timer_diff(_timer_write);
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
    LOG_WRN("cached");
}

void eeprom_flush(uint16_t addr, const void *data, uint16_t size)
{
    while (size)
    {
        uint32_t wait = timer_diff(_timer_write);
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

        _timer_write = timer_start();
    }
}

void eeprom_update(void)
{
    if (_timer_cache && timer_diff(_timer_cache) > 3000)
    {
        if (!_caches)
        {
            LOG_WRN("eeprom saved");
            _timer_cache = 0;
            return;
        }
        eeprom_flush(_caches->addr, _caches->data, _caches->size);
        struct eeprom_cache *cache = _caches;
        _caches = _caches->next;
        free(cache);
    }
}
