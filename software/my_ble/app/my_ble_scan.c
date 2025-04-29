#include "my_ble_scan.h"



//�豸���ƹ��������ݣ����и��豸���ƵĴӻ����ᱻ�������˵�
static char const m_target_periph_name[] = "Washer-Aqua-SR";

//�豸��ַ���������ݣ��õ�ַ���ᱻ�������˵�
static ble_gap_addr_t const m_target_periph_addr =
{
    //��ַ����Ϊ�����̬��ַ
    .addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
//	.addr      = {0xCC, 0x05, 0xE8, 0xCC, 0xBA, 0xE5}
	.addr      = {0x58, 0x1A, 0x38, 0x63, 0x89, 0xF9}
};


NRF_BLE_SCAN_DEF(m_scan);           //��������Ϊm_scan��ɨ����ʵ��     

//ɨ�����
static ble_gap_scan_params_t const m_scan_param =
{
    .active        = 0x01,//ʹ������ɨ��
    .interval      = NRF_BLE_SCAN_SCAN_INTERVAL,//ɨ��������Ϊ��160*0.625ms = 100ms
    .window        = NRF_BLE_SCAN_SCAN_WINDOW,//ɨ�贰������Ϊ��80*0.625ms = 50ms
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,//���ճ��˲��Ƿ��������Ķ���㲥֮������й㲥����
    .timeout       = 0,//ɨ�賬ʱʱ������Ϊ0����ɨ�費��ʱ
    .scan_phys     = BLE_GAP_PHY_1MBPS,// 1 Mbps PHY
};

//����ɨ��
void scan_start(void)
{
    ret_code_t ret;
    //����ɨ����m_scan
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
	NRF_LOG_INFO("start scan");
}

//ֹͣɨ��
void scan_stop(void)
{
    nrf_ble_scan_stop();
    NRF_LOG_INFO("stop scan");
}

// ͳһ��ʽ��ӡMAC��ַ�ĸ�����
#define PRINT_MAC(addr) NRF_LOG_INFO("MAC: %02X:%02X:%02X:%02X:%02X:%02X", \
        (addr)[0], (addr)[1], (addr)[2], (addr)[3], (addr)[4], (addr)[5])
//Ӧ�ó���ɨ���¼�������
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	//��תLEDָʾ��D4��״̬��ָʾɨ�赽�豸
	nrf_gpio_pin_toggle(LED_4);
	// ��ȡ�����ֶΣ������ظ����룩
    const ble_gap_addr_t *peer_addr = &p_scan_evt->params.p_not_found->peer_addr;
    int8_t rssi = p_scan_evt->params.p_not_found->rssi;
    //�ж��¼�����
    switch(p_scan_evt->scan_evt_id)
    {
        //ɨ������ƥ���ַ������
		case NRF_BLE_SCAN_EVT_FILTER_MATCH:
        {
			NRF_LOG_INFO("Filter match event");
            PRINT_MAC(peer_addr->addr);
            NRF_LOG_INFO("RSSI: %ddBm", rssi);
			// ��ӡ���õĹ���MAC������У�
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
		//ɨ�����ݲ�ƥ���ַ������
		case NRF_BLE_SCAN_EVT_NOT_FOUND:
        {

//			// ��ȡ�㲥������
//            uint8_t * adv_data = p_scan_evt->params.p_not_found->data.p_data;
//            uint8_t  adv_len = p_scan_evt->params.p_not_found->data.len;
            NRF_LOG_INFO("Device found (no filter match)");
            PRINT_MAC(peer_addr->addr);
//			// ��ӡ���õĹ���MAC������У�
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
//			// ��ӡ�㲥������
//            NRF_LOG_INFO("Received advertising data (length: %d):", adv_len);
//            NRF_LOG_HEXDUMP_INFO(adv_data, adv_len);  // ��ӡ�㲥����ʮ����������
//            
//			
//			// ��ӡ�豸 RSSI ��ַ				
//			NRF_LOG_INFO("p_scan_evt->params.p_not_found->rssi :%d",p_scan_evt->params.p_not_found->rssi);

        } break;
	 
		//ɨ�賬ʱ�¼�
         case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
         {
			NRF_LOG_INFO("Scan timed out.");
			//����ɨ��
			scan_start();
         } break;
        default:
             break;
    }
}
//ɨ���ʼ��
void my_scan_init(void)
{
    ret_code_t          err_code;
	//����ɨ���ʼ���ṹ�����
    nrf_ble_scan_init_t init_scan;
    //�����㣬������
    memset(&init_scan, 0, sizeof(init_scan));
    //�Զ���������Ϊfalse
    init_scan.connect_if_match = false;
	//ʹ�ó�ʼ���ṹ���е�ɨ���������ɨ����������p_scan_paramָ�����ɨ�����
	init_scan.p_scan_param     = &m_scan_param;
    //conn_cfg_tag����Ϊ1
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    //��ʼ��ɨ����
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
	
//	//��ɨ�������һ���豸���ƹ�����
//    err_code = nrf_ble_scan_filter_set(&m_scan,
//                                           SCAN_NAME_FILTER,
//                                           m_target_periph_name);
//    APP_ERROR_CHECK(err_code);
//	//ʹ���豸���ƹ�����
//	err_code = nrf_ble_scan_filters_enable(&m_scan,
//                                           NRF_BLE_SCAN_NAME_FILTER,
//                                          false);
//	//��麯������ֵ
//    APP_ERROR_CHECK(err_code);
	
	
	//��ɨ�������һ���豸��ַ������
	err_code = nrf_ble_scan_filter_set(&m_scan,
                                          SCAN_ADDR_FILTER,
                                          m_target_periph_addr.addr);
    APP_ERROR_CHECK(err_code);
	//ʹ���豸��ַ������
	err_code = nrf_ble_scan_filters_enable(&m_scan,
                                           NRF_BLE_SCAN_ADDR_FILTER,
                                          false);
	APP_ERROR_CHECK(err_code);								  
}



