#ifndef SCAN_CONNECT_H  // 如果没有定义 MY_HEADER_H
#define SCAN_CONNECT_H  // 定义它，避免重复包含


#include "main.h"
//SCAN需要引用的头文件
#include "nrf_ble_scan.h"

void scan_evt_handler(scan_evt_t const * p_scan_evt);
void scan_start(void);
void scan_init(void);

#endif


