#include "log.h"
#include "eeprom.h"
#include "flash.h"
#include "timer.h"
#include "main.h"

static uint8_t _sector_max = 3;
static uint32_t _offset = 0;
static uint32_t _offset_show = 0;

void log_init(void)
{
    uint8_t start = 0;
    eeprom_read(EEPROM_SETTINGS_LOG, &start, 1);
    if (start == 0xff)
    {
        start = 0;
        _offset = start * 4096;
        flash_sector_erase(_offset);
    }
    while(1)
    {
        flash_read(_offset, &start, 1);
        if (start == 0xff)
        {
            break;
        }
        _offset += 8;
        if (!(_offset % 4096))
        {
            start = _offset / 4096;
            if (start >= _sector_max)
            {
                start = 0;
                _offset = 0;
            }
            flash_sector_erase(_offset);
            eeprom_write(EEPROM_SETTINGS_LOG, &start, 1);
        }
    }
    SEGGER_RTT_printf(0, "log offset:%u\n", _offset);
    log_append(0x00, 0x0000);
    _offset_show = 0;
}

void log_update(void)
{
    if (_offset_show < 10)
    {
        uint8_t opcode = log_opcode_read(_offset_show);
        if (opcode == 0xff)
        {
            _offset_show = 4096 * _sector_max / 8;
            return;
        }
        uint32_t timestamp = log_timestamp_read(_offset_show);
        uint16_t param = log_param_read(_offset_show);

        SEGGER_RTT_printf(0, "\toffset:-%u op:%u param:%04x timestamp:%u\n", _offset_show, opcode, param, timestamp);

        _offset_show++;
    }
}

void log_append(uint8_t opcode, uint16_t param)
{
    uint32_t now = timer_start();
    flash_write(_offset + 0, &now, 4);
    flash_write(_offset + 4, &opcode, 1);
    flash_write(_offset + 5, &param, 2);

    _offset += 8;
    if (!(_offset % 4096))
    {
        uint8_t start = _offset / 4096;
        if (start >= _sector_max)
        {
            start = 0;
            _offset = 0;
        }
        flash_sector_erase(_offset);
        eeprom_write(EEPROM_SETTINGS_LOG, &start, 1);
    }
}

uint8_t log_opcode_read(uint32_t offset)
{
    uint8_t opcode = 0;
    if (_offset >= (offset + 1) * 8)
    {
        offset = _offset - (offset + 1) * 8;
    }
    else
    {
        offset = _sector_max * 4096 - (offset + 1) * 8 + _offset;
    }
    flash_read(offset + 4, &opcode, 1);
    return opcode;
}

uint32_t log_timestamp_read(uint32_t offset)
{
    uint32_t timestamp = 0;
    if (_offset >= (offset + 1) * 8)
    {
        offset = _offset - (offset + 1) * 8;
    }
    else
    {
        offset = _sector_max * 4096 - (offset + 1) * 8 + _offset;
    }
    flash_read(offset + 0, &timestamp, 4);
    return timestamp;
}

uint16_t log_param_read(uint32_t offset)
{
    uint16_t param = 0;
    if (_offset >= (offset + 1) * 8)
    {
        offset = _offset - (offset + 1) * 8;
    }
    else
    {
        offset = _sector_max * 4096 - (offset + 1) * 8 + _offset;
    }
    flash_read(offset + 5, &param, 2);
    return param;
}
