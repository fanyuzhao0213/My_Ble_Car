#ifndef UART_H  // 如果没有定义 MY_HEADER_H
#define UART_H  // 定义它，避免重复包含

#include "main.h"


extern uint8_t     TxBuffer_MCU[100];                     //MCU发送缓冲buff
extern void my_dada_pack_mcu(uint8_t Send_ID);
void uart_config(void);

void uart_event_handle(app_uart_evt_t * p_event);

void uart_reconfig(void);
#endif

