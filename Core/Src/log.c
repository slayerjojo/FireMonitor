#include "log.h"
#include "eeprom.h"
#include "flash.h"
#include "timer.h"
#include "main.h"

static const uint8_t _log_level = 1;

static uint8_t _sector_max = 3;
static uint32_t _offset = 0;
static char _buffer[1024] = {0};
static const char *_colors[] = {RTT_CTRL_TEXT_BRIGHT_GREEN, RTT_CTRL_TEXT_BRIGHT_WHITE, RTT_CTRL_TEXT_BRIGHT_YELLOW, RTT_CTRL_TEXT_BRIGHT_RED};

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
    LOG_INF("log offset:%u", _offset);
    start = 0;
    while (start < 10)
    {
        uint8_t opcode = log_opcode_read(start);
        if (opcode == 0xff)
        {
            break;
        }
        uint32_t timestamp = log_timestamp_read(start);
        uint16_t param = log_param_read(start);

        LOG_INF("\toffset:-%u op:%u param:%04x timestamp:%u", start, opcode, param, timestamp);
        timer_sleep(100);
        start++;
    }
    log_append(0x00, 0x0000);
}

void log_update(void)
{
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

void log_print(uint8_t action, uint8_t level, const char *func, uint16_t line, const char *fmt, ...)
{
    if (action < level)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(_buffer, 1023, fmt, ap);
    va_end(ap);

    SEGGER_RTT_printf(0, "%s%5u|%s:%u|%s\n"RTT_CTRL_RESET, _colors[action], timer_start(), func, line, _buffer);
}

void log_raw_print(uint8_t action, uint8_t level, const char *fmt, ...)
{
    if (action < level)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(_buffer, 1023, fmt, ap);
    va_end(ap);

    SEGGER_RTT_printf(0, "%s%s"RTT_CTRL_RESET, _colors[action], _buffer);
}

void log_hex_print(uint8_t action, uint8_t level, const char *func, uint16_t line, uint8_t *data, uint16_t size, const char *fmt, ...)
{
    if (action < level)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(_buffer, 1024, fmt, ap);
    va_end(ap);

    SEGGER_RTT_printf(0, "%s%5u|%s:%u|%s", _colors[action], timer_start(), func, line, _buffer);
    for (int i = 0; i < size; i++)
    {
        SEGGER_RTT_printf(0, "%02x ", data[i]);
    }
    SEGGER_RTT_printf(0, "|");
    for (int i = 0; i < size; i++)
    {
        SEGGER_RTT_printf(0, "%c ", data[i] ? data[i] : ' ');
    }
    SEGGER_RTT_printf(0, RTT_CTRL_RESET"\n");
}
