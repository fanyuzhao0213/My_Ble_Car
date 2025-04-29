#ifndef MY_BLE_MAC_H  // 如果没有定义 MY_HEADER_H
#define MY_BLE_MAC_H  // 定义它，避免重复包含

#include "main.h"

extern void my_ble_mac_init(void);

typedef struct
{
    uint8_t   type;
    int8_t    rssi;
    uint8_t   addr[BLE_GAP_ADDR_LEN];
    uint8_t   addr_s[BLE_GAP_ADDR_LEN];
} ble_device_t;


#endif


