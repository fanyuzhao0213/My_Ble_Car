#ifndef MY_BLE_H  // ���û�ж��� MY_HEADER_H
#define MY_BLE_H  // �������������ظ�����

#include "main.h"

//���崮��͸������UUID�б�
extern ble_uuid_t m_adv_uuids[];
 
//���͵�������ݳ���
extern uint16_t  m_ble_uarts_max_data_len;            
extern bool 	uart_enabled;
//�ñ������ڱ������Ӿ������ʼֵ����Ϊ������
extern uint16_t m_conn_handle; 

extern void my_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle);

void advertising_start(void);
void ble_stack_init(void);
void advertising_init(void);
void conn_params_init(void);
void services_init(void);
void gatt_init(void);
void gap_params_init(void);

extern void my_ble_init(void);
void my_nus_data_recv_handler_task(void);
void my_data_analysis(uint8_t const* data, uint16_t len);
#endif


