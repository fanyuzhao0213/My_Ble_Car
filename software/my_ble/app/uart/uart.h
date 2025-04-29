#ifndef UART_H  // ���û�ж��� MY_HEADER_H
#define UART_H  // �������������ظ�����

#include "main.h"


extern uint8_t     TxBuffer_MCU[100];                     //MCU���ͻ���buff
extern void my_dada_pack_mcu(uint8_t Send_ID);
void uart_config(void);

void uart_event_handle(app_uart_evt_t * p_event);

void uart_reconfig(void);
#endif

