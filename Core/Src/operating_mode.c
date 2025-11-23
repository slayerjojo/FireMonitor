#include "operating_mode.h"
#include "main.h"
#include "eeprom.h"

static const uint8_t _log_level = 1;

static uint8_t _mode = 1;
static uint8_t _temp = 0;

void operating_mode_init(void)
{
    eeprom_read(EEPROM_SETTINGS_OM_MODE, &_mode, 1);
    if (0xff == _mode)
    {
        _mode = 1;
        eeprom_write(EEPROM_SETTINGS_OM_MODE, &_mode, 1);
    }
    _temp = _mode;
    LOG_INF("operating mode:%u", _mode);
}

uint8_t operating_mode_temp_get(void)
{
    return _temp;
}

void operating_mode_ts(uint8_t mode)
{
    _temp = mode;
}

uint8_t operating_mode_get(void)
{
    return _mode;
}

void operating_mode_set(uint8_t mode)
{
    _mode = mode;
    eeprom_write(EEPROM_SETTINGS_OM_MODE, &_mode, 1);
}

void operating_mode_default_load(void)
{
    _mode = 1;
    eeprom_write(EEPROM_SETTINGS_OM_MODE, &_mode, 1);
}

void operating_mode_configure_apply(void)
{
    _mode = _temp;
    eeprom_write(EEPROM_SETTINGS_OM_MODE, &_mode, 1);
}
