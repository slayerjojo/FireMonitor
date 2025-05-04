#ifndef __USART_H__
#define __USART_H__

#include <stdint.h>

enum {
    COMM_PROTOCOL_MODBUS,
    COMM_PROTOCOL_PROFIBUS,
};

void usart_init(void);
void usart_update(void);
void usart_send(uint8_t *buffer, uint16_t size);
uint8_t usart_comm_protocol(void);
uint8_t usart_comm_device_address(uint8_t address);
uint8_t usart_comm_baud(void);
void usart_comm_device_address_set(uint8_t protocol, uint8_t addr, uint8_t temp);
void usart_comm_baud_set(uint8_t protocol, uint8_t baud);
void usart_comm_device_address_apply(uint8_t protocol, uint8_t index);
void usart_comm_baud_apply(uint8_t protocol);
#endif
