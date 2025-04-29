#ifndef MY_BLE_H  // 如果没有定义 MY_HEADER_H
#define MY_BLE_H  // 定义它，避免重复包含

#include "main.h"

//定义串口透传服务UUID列表
extern ble_uuid_t m_adv_uuids[];
 
//发送的最大数据长度
extern uint16_t  m_ble_uarts_max_data_len;            
extern bool 	uart_enabled;
//该变量用于保存连接句柄，初始值设置为无连接
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


