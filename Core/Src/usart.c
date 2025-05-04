#include "usart.h"
#include "main.h"
#include "timer.h"
#include "crc16.h"

#define MAX_UART_BUFFER 128

extern UART_HandleTypeDef huart1;

static uint8_t _buffer[MAX_UART_BUFFER + 64] = {0};
static uint16_t _pos = 0;
static uint16_t _append = 0;

static struct modbus
{
    uint8_t address;
    uint8_t address2;
}_modbus = {0};

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
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER + 64 - _append - 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER + 64 - _append - 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_RESET);
}

void usart_send(uint8_t *buffer, uint16_t size)
{
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_SET);
    HAL_UART_Transmit_DMA(&huart1, buffer, size);
}

static void modbus_handler(uint8_t opcode, uint8_t *buffer, uint8_t size)
{
}

void usart_init(void)
{
    SEGGER_RTT_printf(0, "usart initialized\n");

    _pos = _append = 0;
    HAL_GPIO_WritePin(MODBUS_PV_GPIO_Port, MODBUS_PV_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_Pin, GPIO_PIN_RESET);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER + 64 - _append - 1);
}

void usart_update(void)
{
    static uint32_t _timer = 1;
    if (timer_diff(_timer) > 1000)
    {
        _timer = timer_start();
        usart_send("abcdef\n", 7);
    }
    if (_pos == _append)
        return;

    SEGGER_RTT_printf(0, "usart:");
    for (int i = 0; i < _buffer[_pos]; i++)
    {
        SEGGER_RTT_printf(0, "%02x ", _buffer[_pos + 1 + i]);
    }
    SEGGER_RTT_printf(0, "\n");
    _pos += 1 + _buffer[_pos];
    /*
    if (_append > _pos)
    {
        if (_append - _pos < 5)
            return;
        if (_pos + 1 + 1 + 1 + _buffer[_pos + 2] + 2 > _append)
            return;
    }
    else
    {
        if (_size - _pos + _append < 5)
            return;
        if (_pos + 1 + 1 + 1 + _buffer[(_pos + 1 + 1) % _size] + 2 > _append)
            return;
        if (_pos + 1 + 1 + 1 + _buffer[(_pos + 1 + 1) % _size] + 2 > _size)
            memcpy(&_buffer[_size], &_buffer[0], _pos + 1 + 1 + 1 + _buffer[(_pos + 1 + 1) % _size] + 2 - _size);
    }
    if (crc16(&_buffer[_pos], 5 + _buffer[_pos + 1 + 1]) == (((uint16_t)_buffer[_pos + 3 + _buffer[_pos + 2] + 1]) << 8) + _buffer[_pos + 3 + _buffer[_pos + 2]])
    {
        if (_modbus.address == _buffer[_pos] || _modbus.address2 == _buffer[_pos])
        {
            modbus_handler(_buffer[_pos + 1], &_buffer[_pos + 3], _buffer[_pos + 2]);
        }
    }
    else
    {
        SEGGER_RTT_printf(0, "modbus_check failed. data:");
        for (int i = 0; i < 3 + _buffer[_pos + 2] + 2; i++)
        {
            SEGGER_RTT_printf(0, "%02x ", _buffer[_pos + i]);
        }
        SEGGER_RTT_printf(0, "\n");
    }
    _pos = (_pos + 3 + _buffer[_pos + 2] + 2) % _size;
    */
}

uint8_t usart_comm_protocol(void)
{
    return 0;
}

uint8_t usart_comm_device_address(uint8_t address)
{
    return 0;
}

uint8_t usart_comm_baud(void)
{
    return 0;
}

void usart_comm_device_address_set(uint8_t protocol, uint8_t addr, uint8_t temp)
{
}

void usart_comm_baud_set(uint8_t protocol, uint8_t baud)
{
}

void usart_comm_device_address_apply(uint8_t protocol, uint8_t index)
{
}

void usart_comm_baud_apply(uint8_t protocol)
{
}
