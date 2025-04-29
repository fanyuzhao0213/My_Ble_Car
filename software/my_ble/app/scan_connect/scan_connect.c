#include "scan_connect.h"


NRF_BLE_SCAN_DEF(m_scan);           //��������Ϊm_scan��ɨ����ʵ�� 


//�豸��������  Ӣ������
static char const m_target_periph_name[] = "Washer-Aqua-Mu";

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
static void scan_start(void)
{
    ret_code_t ret;
    //����ɨ����m_scan
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
}
//Ӧ�ó���ɨ���¼�������
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	ret_code_t err_code;
	
    //�ж��¼�����
    switch(p_scan_evt->scan_evt_id)
    {
         //���Ӵ����¼�
		case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
		{
              err_code = p_scan_evt->params.connecting_err.err_code;
              APP_ERROR_CHECK(err_code);
		} break;
			
		//ɨ������ƥ�����������ӡƥ����Ϣ
		case NRF_BLE_SCAN_EVT_FILTER_MATCH:
        {
			ble_gap_evt_adv_report_t const * p_adv =
			p_scan_evt->params.filter_match.p_adv_report;
             //��ӡ��MAC��ַ���ֲ�ͬ�Ĵӻ�
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
static void scan_init(void)
{
    ret_code_t          err_code;
	  //����ɨ���ʼ���ṹ�����
    nrf_ble_scan_init_t init_scan;
	  
    //������ɨ���ʼ���ṹ�壬������
    memset(&init_scan, 0, sizeof(init_scan));
    //�Զ���������Ϊfalse
    init_scan.connect_if_match = true;
	//ʹ�ó�ʼ���ṹ���е�ɨ���������ɨ����������p_scan_paramָ�����ɨ�����
	init_scan.p_scan_param     = &m_scan_param;
    //conn_cfg_tag����Ϊ1
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    //��ʼ��ɨ����
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
	
    APP_ERROR_CHECK(err_code);
	//��ɨ��������豸���ƹ�����
	err_code = nrf_ble_scan_filter_set(&m_scan,
                                       SCAN_NAME_FILTER,
                                       m_target_periph_name);
    APP_ERROR_CHECK(err_code);

	//ʹ��UUID������
	err_code = nrf_ble_scan_filters_enable(&m_scan,
                                           NRF_BLE_SCAN_NAME_FILTER,
                                          false);
		//��麯������ֵ
    APP_ERROR_CHECK(err_code);
}


