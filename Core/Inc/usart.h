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
uint32_t usart_comm_baud(void);
uint32_t usart_comm_baud_temp(void);
void usart_comm_baud_ts(uint32_t baud);
void usart_comm_baud_apply(void);

#endif
