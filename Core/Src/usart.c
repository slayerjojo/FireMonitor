#include "usart.h"
#include "main.h"
#include "timer.h"
#include "modbus.h"
#include "eeprom.h"
#include "crc16.h"

#define MAX_UART_BUFFER 256

extern UART_HandleTypeDef huart1;

static uint8_t _buffer[MAX_UART_BUFFER * 2] = {0};
static uint16_t _pos = 0;
static uint16_t _append = 0;
uint32_t _baud = 9600;
static uint32_t _temp = 9600;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)//call this when idle
{
    if (huart == &huart1)
    {
        _buffer[_append] = size;
        _append += 1 + size;
        if (_append >= MAX_UART_BUFFER)
        {
            _append = 0;
        }
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER * 2 - _append - 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER * 2 - _append - 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MODBUS_RTS_GPIO_Port, MODBUS_RTS_Pin, GPIO_PIN_RESET);
}

void usart_send(uint8_t *buffer, uint16_t size)
{
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MODBUS_RTS_GPIO_Port, MODBUS_RTS_Pin, GPIO_PIN_SET);
    HAL_UART_Transmit_DMA(&huart1, buffer, size);
}

void usart_baud_init(void)
{
    eeprom_read(EEPROM_SETTINGS_USART_BAUD, &_baud, 4);
    if (0xffffffff == _baud)
    {
        _baud = 9600;
    }
    _temp = _baud;
    SEGGER_RTT_printf(0, "usart initialized(baud:%u)\n", _baud);
}

void usart_init(void)
{
    modbus_init();

    _pos = _append = 0;
    HAL_GPIO_WritePin(MODBUS_PV_GPIO_Port, MODBUS_PV_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MODBUS_RTS_GPIO_Port, MODBUS_RTS_Pin, GPIO_PIN_RESET);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER * 2 - _append - 1);
}

void usart_update(void)
{
    if (_pos == _append)
        return;

    SEGGER_RTT_printf(0, "usart recv:");
    for (int i = 0; i < _buffer[_pos]; i++)
    {
        SEGGER_RTT_printf(0, "%02x ", _buffer[_pos + 1 + i]);
    }
    SEGGER_RTT_printf(0, "\n");

    modbus_recv(&_buffer[_pos + 1], _buffer[_pos]);
    _pos += 1 + _buffer[_pos];
    if (_pos > MAX_UART_BUFFER)
        _pos = 0;
}

uint8_t usart_comm_protocol(void)
{
    return COMM_PROTOCOL_MODBUS;
}

uint32_t usart_comm_baud(void)
{
    return _baud;
}

uint32_t usart_comm_baud_temp(void)
{
    return _temp;
}

void usart_comm_baud_ts(uint32_t baud)
{
    _temp = baud;
}

void usart_comm_baud_apply(void)
{
    _baud = _temp;
    eeprom_write(EEPROM_SETTINGS_USART_BAUD, &_baud, 4);
}
