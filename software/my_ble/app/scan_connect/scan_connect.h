#ifndef SCAN_CONNECT_H  // ���û�ж��� MY_HEADER_H
#define SCAN_CONNECT_H  // �������������ظ�����


#include "main.h"
//SCAN��Ҫ���õ�ͷ�ļ�
#include "nrf_ble_scan.h"

void scan_evt_handler(scan_evt_t const * p_scan_evt);
void scan_start(void);
void scan_init(void);

#endif


