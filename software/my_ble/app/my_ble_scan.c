#include "my_ble_scan.h"



//设备名称过滤器数据，具有该设备名称的从机不会被过滤器滤掉
static char const m_target_periph_name[] = "Washer-Aqua-SR";

//设备地址过滤器数据，该地址不会被过滤器滤掉
static ble_gap_addr_t const m_target_periph_addr =
{
    //地址类型为随机静态地址
    .addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
//	.addr      = {0xCC, 0x05, 0xE8, 0xCC, 0xBA, 0xE5}
	.addr      = {0x58, 0x1A, 0x38, 0x63, 0x89, 0xF9}
};


NRF_BLE_SCAN_DEF(m_scan);           //定义名称为m_scan的扫描器实例     

//扫描参数
static ble_gap_scan_params_t const m_scan_param =
{
    .active        = 0x01,//使用主动扫描
    .interval      = NRF_BLE_SCAN_SCAN_INTERVAL,//扫描间隔设置为：160*0.625ms = 100ms
    .window        = NRF_BLE_SCAN_SCAN_WINDOW,//扫描窗口设置为：80*0.625ms = 50ms
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,//接收除了不是发给本机的定向广播之外的所有广播数据
    .timeout       = 0,//扫描超时时间设置为0，即扫描不超时
    .scan_phys     = BLE_GAP_PHY_1MBPS,// 1 Mbps PHY
};

//启动扫描
void scan_start(void)
{
    ret_code_t ret;
    //启动扫描器m_scan
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
	NRF_LOG_INFO("start scan");
}

//停止扫描
void scan_stop(void)
{
    nrf_ble_scan_stop();
    NRF_LOG_INFO("stop scan");
}

// 统一格式打印MAC地址的辅助宏
#define PRINT_MAC(addr) NRF_LOG_INFO("MAC: %02X:%02X:%02X:%02X:%02X:%02X", \
        (addr)[0], (addr)[1], (addr)[2], (addr)[3], (addr)[4], (addr)[5])
//应用程序扫描事件处理函数
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	//翻转LED指示灯D4的状态，指示扫描到设备
	nrf_gpio_pin_toggle(LED_4);
	// 提取公共字段（减少重复代码）
    const ble_gap_addr_t *peer_addr = &p_scan_evt->params.p_not_found->peer_addr;
    int8_t rssi = p_scan_evt->params.p_not_found->rssi;
    //判断事件类型
    switch(p_scan_evt->scan_evt_id)
    {
        //扫描数据匹配地址过滤器
		case NRF_BLE_SCAN_EVT_FILTER_MATCH:
        {
			NRF_LOG_INFO("Filter match event");
            PRINT_MAC(peer_addr->addr);
            NRF_LOG_INFO("RSSI: %ddBm", rssi);
			// 打印设置的过滤MAC（如果有）
            if (m_scan.scan_filters.addr_filter.addr_filter_enabled)
            {
                NRF_LOG_INFO("Filter MAC (expected):");
                PRINT_MAC(m_scan.scan_filters.addr_filter.target_addr[0].addr);
            }
            else
            {
                NRF_LOG_INFO("No MAC filter set (name filter?)");
            }
			scan_stop();

        } break;
		//扫描数据不匹配地址过滤器
		case NRF_BLE_SCAN_EVT_NOT_FOUND:
        {

//			// 获取广播包数据
//            uint8_t * adv_data = p_scan_evt->params.p_not_found->data.p_data;
//            uint8_t  adv_len = p_scan_evt->params.p_not_found->data.len;
            NRF_LOG_INFO("Device found (no filter match)");
            PRINT_MAC(peer_addr->addr);
//			// 打印设置的过滤MAC（如果有）
//            if (m_scan.scan_filters.addr_filter.addr_filter_enabled)
//            {
//                NRF_LOG_INFO("Filter MAC (expected):");
//                PRINT_MAC(m_scan.scan_filters.addr_filter.target_addr[0].addr);
//				NRF_LOG_INFO("m_scan.scan_filters.addr_filter.target_addr[0].addr_type:%d",m_scan.scan_filters.addr_filter.target_addr[0].addr_type);
//            }
//            else
//            {
//                NRF_LOG_INFO("No MAC filter set (name filter?)");
//            }
//			// 打印广播包内容
//            NRF_LOG_INFO("Received advertising data (length: %d):", adv_len);
//            NRF_LOG_HEXDUMP_INFO(adv_data, adv_len);  // 打印广播包的十六进制内容
//            
//			
//			// 打印设备 RSSI 地址				
//			NRF_LOG_INFO("p_scan_evt->params.p_not_found->rssi :%d",p_scan_evt->params.p_not_found->rssi);

        } break;
	 
		//扫描超时事件
         case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
         {
			NRF_LOG_INFO("Scan timed out.");
			//重启扫描
			scan_start();
         } break;
        default:
             break;
    }
}
//扫描初始化
void my_scan_init(void)
{
    ret_code_t          err_code;
	//定义扫描初始化结构体变量
    nrf_ble_scan_init_t init_scan;
    //先清零，再配置
    memset(&init_scan, 0, sizeof(init_scan));
    //自动连接设置为false
    init_scan.connect_if_match = false;
	//使用初始化结构体中的扫描参数配置扫描器，这里p_scan_param指向定义的扫描参数
	init_scan.p_scan_param     = &m_scan_param;
    //conn_cfg_tag设置为1
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    //初始化扫描器
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
	
//	//向扫描器添加一个设备名称过滤器
//    err_code = nrf_ble_scan_filter_set(&m_scan,
//                                           SCAN_NAME_FILTER,
//                                           m_target_periph_name);
//    APP_ERROR_CHECK(err_code);
//	//使能设备名称过滤器
//	err_code = nrf_ble_scan_filters_enable(&m_scan,
//                                           NRF_BLE_SCAN_NAME_FILTER,
//                                          false);
//	//检查函数返回值
//    APP_ERROR_CHECK(err_code);
	
	
	//向扫描器添加一个设备地址过滤器
	err_code = nrf_ble_scan_filter_set(&m_scan,
                                          SCAN_ADDR_FILTER,
                                          m_target_periph_addr.addr);
    APP_ERROR_CHECK(err_code);
	//使能设备地址过滤器
	err_code = nrf_ble_scan_filters_enable(&m_scan,
                                           NRF_BLE_SCAN_ADDR_FILTER,
                                          false);
	APP_ERROR_CHECK(err_code);								  
}



