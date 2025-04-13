#include "usart.h"
#include "main.h"
#include "timer.h"

#define MAX_UART_BUFFER 128

extern UART_HandleTypeDef huart1;

static uint8_t _buffer[MAX_UART_BUFFER + 64] = {0};
static uint16_t _pos = 0;
static uint16_t _append = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)//call this when idle
{
    if (huart == &huart1)
    {
        _buffer[_append] = size;
        _append += 1 + size;
        if (_append >= MAX_UART_BUFFER)
            _append = 0;
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

static void usart_send(uint8_t *buffer, uint16_t size)
{
    HAL_UART_Transmit_DMA(&huart1, buffer, size);
}

void usart_init(void)
{
    SEGGER_RTT_printf(0, "usart initialized\n");

    HAL_GPIO_WritePin(MODBUS_PV_GPIO_Port, MODBUS_PV_Pin, GPIO_PIN_SET);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, _buffer + _append + 1, MAX_UART_BUFFER + 64 - _append - 1);
}

void usart_update(void)
{
    uint8_t buffer[10] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    static uint32_t timer = 1;
    if (timer_diff(timer) > 1000)
    {
        timer = timer_start();

        usart_send(buffer, 10);
    }
    if (_pos != _append)
    {
        for (int i = 0; i < _buffer[_pos]; i++)
        {
            SEGGER_RTT_printf(0, "%02x ", _buffer[_pos + 1 + i]);
        }
        SEGGER_RTT_printf(0, "\n");

        _pos += 1 + _buffer[_pos];
        if (_pos >= MAX_UART_BUFFER)
            _pos = 0;
    }
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
