#ifndef MY_BLE_SCAN_H  // 如果没有定义 MY_HEADER_H
#define MY_BLE_SCAN_H  // 定义它，避免重复包含


#include "main.h"

extern void my_scan_init(void);
extern void scan_start(void);
extern void scan_stop(void);
#endif


