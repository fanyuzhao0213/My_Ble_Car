#include "my_ble_mac.h"

ble_gap_addr_t device_addr;
ble_device_t my_device,recv_device;

int my_ble_mac_set()
{
    device_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
    int ret = sd_ble_gap_addr_set(&device_addr);
    return ret;
}

void my_ble_mac_get(void)
{
	ret_code_t err_code;
    /* init m_device mac */
    err_code =  sd_ble_gap_addr_get(&device_addr);
	//地址读取成功，串口打印出地址
	if(err_code == NRF_SUCCESS)
	{
		//打印地址类型
		NRF_LOG_INFO("sd_ble_gap_addr_get Address Type: %02X",device_addr.addr_type);
		//打印地址
		NRF_LOG_INFO("sd_ble_gap_addr_get Address: %02X:%02X:%02X:%02X:%02X:%02X",
								 device_addr.addr[0], device_addr.addr[1],
								 device_addr.addr[2], device_addr.addr[3],
								 device_addr.addr[4], device_addr.addr[5]);
	}
}

void my_ble_mac_init(void)
{
    my_ble_mac_get();
    /* get m_device mac */
    int ret  = my_ble_mac_set();
    nrf_delay_ms(50);
	my_ble_mac_get();
}



