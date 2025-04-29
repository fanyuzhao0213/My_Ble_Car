#include "scan_connect.h"


NRF_BLE_SCAN_DEF(m_scan);           //定义名称为m_scan的扫描器实例 


//设备名称数组  英文名称
static char const m_target_periph_name[] = "Washer-Aqua-Mu";

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
static void scan_start(void)
{
    ret_code_t ret;
    //启动扫描器m_scan
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
}
//应用程序扫描事件处理函数
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	ret_code_t err_code;
	
    //判断事件类型
    switch(p_scan_evt->scan_evt_id)
    {
         //连接错误事件
		case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
		{
              err_code = p_scan_evt->params.connecting_err.err_code;
              APP_ERROR_CHECK(err_code);
		} break;
			
		//扫描数据匹配过滤器，打印匹配信息
		case NRF_BLE_SCAN_EVT_FILTER_MATCH:
        {
			ble_gap_evt_adv_report_t const * p_adv =
			p_scan_evt->params.filter_match.p_adv_report;
             //打印出MAC地址区分不同的从机
			NRF_LOG_INFO("MAC ADDR:%02X%02X%02X%02X%02X%02X ",
                      p_adv->peer_addr.addr[0],
                      p_adv->peer_addr.addr[1],
                      p_adv->peer_addr.addr[2],
                      p_adv->peer_addr.addr[3],
                      p_adv->peer_addr.addr[4],
                      p_adv->peer_addr.addr[5]
                      ); 
			NRF_LOG_INFO("device name filter match\r\n");
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
static void scan_init(void)
{
    ret_code_t          err_code;
	  //定义扫描初始化结构体变量
    nrf_ble_scan_init_t init_scan;
	  
    //先清零扫描初始化结构体，再配置
    memset(&init_scan, 0, sizeof(init_scan));
    //自动连接设置为false
    init_scan.connect_if_match = true;
	//使用初始化结构体中的扫描参数配置扫描器，这里p_scan_param指向定义的扫描参数
	init_scan.p_scan_param     = &m_scan_param;
    //conn_cfg_tag设置为1
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    //初始化扫描器
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
	
    APP_ERROR_CHECK(err_code);
	//向扫描器添加设备名称过滤器
	err_code = nrf_ble_scan_filter_set(&m_scan,
                                       SCAN_NAME_FILTER,
                                       m_target_periph_name);
    APP_ERROR_CHECK(err_code);

	//使能UUID过滤器
	err_code = nrf_ble_scan_filters_enable(&m_scan,
                                           NRF_BLE_SCAN_NAME_FILTER,
                                          false);
		//检查函数返回值
    APP_ERROR_CHECK(err_code);
}


