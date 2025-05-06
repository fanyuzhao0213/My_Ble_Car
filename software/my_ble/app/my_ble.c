#include "my_ble.h"
#include "check.h"
#include "uart.h"

bool uart_enabled = false;

NRF_BLE_GATT_DEF(m_gatt);                                //��������Ϊm_gatt��GATTģ��ʵ��
NRF_BLE_QWR_DEF(m_qwr);                                  //����һ������Ϊm_qwr���Ŷ�д��ʵ��
BLE_ADVERTISING_DEF(m_advertising);                      //��������Ϊm_advertising�Ĺ㲥ģ��ʵ��
BLE_UARTS_DEF(m_uarts, NRF_SDH_BLE_TOTAL_LINK_COUNT);    //��������Ϊm_uarts�Ĵ���͸������ʵ��

#define NUS_DATA_REC_BUFF_MAX  		50
#define MY_BLE_BUFF_SIZE_MAX        200            //200

uint16_t    m_recv_large_len = 0;
static char m_recv_flag = 0;
uint8_t     m_recv_data[MY_BLE_BUFF_SIZE_MAX];


uint8_t TxBuffer_APP[100]={0}; 		//APP���ͻ���buff
static uint8_t nus_data_rcv_buff[NUS_DATA_REC_BUFF_MAX][20];
static uint8_t nus_data_rcv_buff_len = 0;
static uint16_t nus_data_write_p = 0;
static uint16_t nus_data_read_p = 0;
static uint16_t nus_data_rcv_flag = 0; //in handle ++; in analysis --

//���崮��͸������UUID�б�
ble_uuid_t m_adv_uuids[]=                                          
{
    {BLE_UUID_UARTS_SERVICE, UARTS_SERVICE_UUID_TYPE}
};

//�Ŷ�д���¼������������ڴ����Ŷ�д��ģ��Ĵ���
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    //���������
	APP_ERROR_HANDLER(nrf_error);
}

//�ػ�׼����������ڹرչ����У�����1��ļ�����ô˺�����ֱ����������true������������trueʱ����ʾӦ�ó�����׼���ø�λΪDFUģʽ
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}


//ע�����ȼ�Ϊ0��Ӧ�ó���رմ������
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);
//SoftDevice״̬�������¼�������
static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
			  //����Softdevice�ڸ�λ֮ǰ�Ѿ����ã���֮bootloader����ʱӦ����CRC
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //����system offģʽ
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

//ע��SoftDevice״̬�����ߣ�����SoftDevice״̬�ı���߼����ı�ʱ����SoftDevice�¼�
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};

//��ȡ�㲥ģʽ������ͳ�ʱʱ��
static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}

//�Ͽ���ǰ���ӣ��豸׼������bootloader֮ǰ����Ҫ�ȶϿ�����
static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);
    //�Ͽ���ǰ����
    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}

//DFU�¼��������������Ҫ��DFU�¼���ִ�в�������������Ӧ���¼�������봦�����
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE://���¼�ָʾ�豸����׼������bootloader
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
            //��ֹ�豸�ڶϿ�����ʱ�㲥
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
			//���ӶϿ����豸���Զ����й㲥
            config.ble_adv_on_disconnect_disabled = true;
			//�޸Ĺ㲥����
            ble_advertising_modes_config_set(&m_advertising, &config);

			//�Ͽ���ǰ�Ѿ����ӵ������������豸�����豸�̼����³ɹ�������ֹ������Ҫ������ʱ���շ������ָʾ
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER://���¼�ָʾ�������غ��豸������bootloader
            //���Ӧ�ó�����������Ҫ���浽Flash��ͨ��app_shutdown_handler����flase���ӳٸ�λ���Ӷ���֤������ȷд�뵽Flash
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED://���¼�ָʾ����bootloaderʧ��
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
			//����bootloaderʧ�ܣ�Ӧ�ó�����Ҫ��ȡ������ʩ���������⣬��ʹ��APP_ERROR_CHECK��λ�豸
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR://���¼�ָʾ������Ӧʧ��
            NRF_LOG_ERROR("Request to send a response to client failed.");
			//������Ӧʧ�ܣ�Ӧ�ó�����Ҫ��ȡ������ʩ���������⣬��ʹ��APP_ERROR_CHECK��λ�豸
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}



//GAP������ʼ�����ú���������Ҫ��GAP�����������豸���ƣ������������ѡ���Ӳ���
void gap_params_init(void)
{
    ret_code_t              err_code;
	  //�������Ӳ����ṹ�����
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    //����GAP�İ�ȫģʽ
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //����GAP�豸���ƣ�ʹ��Ӣ���豸����
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)DEVICE_NAME,
                                              strlen(DEVICE_NAME));
	
//	//����GAP�豸���ƣ�����ʹ���������豸����
//    err_code = sd_ble_gap_device_name_set(&sec_mode,
//                                          (const uint8_t *)device_name,
//                                          sizeof(device_name));
//																					
    //��麯�����صĴ������
		APP_ERROR_CHECK(err_code);
																				
    //������ѡ���Ӳ���������ǰ������gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;//��С���Ӽ��
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;//��С���Ӽ��
    gap_conn_params.slave_latency     = SLAVE_LATENCY;    //�ӻ��ӳ�
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT; //�ල��ʱ
    //����Э��ջAPI sd_ble_gap_ppcp_set����GAP����
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
																					
}
//GATT�¼����������ú����д���MTU�����¼�
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    //�����MTU�����¼�
	  if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //���ô���͸���������Ч���ݳ��ȣ�MTU-opcode-handle��
		m_ble_uarts_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_uarts_max_data_len, m_ble_uarts_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}
//��ʼ��GATT����ģ��
void gatt_init(void)
{
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);    //��ʼ��GATT����ģ��
    APP_ERROR_CHECK(err_code);		//��麯�����صĴ������
	err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);	  //����ATT MTU�Ĵ�С,�������õ�ֵΪ247
    APP_ERROR_CHECK(err_code);		//��麯�����صĴ������
}

//����͸���¼��ص�����������͸�������ʼ��ʱע��
void uarts_data_handler(ble_uarts_evt_t * p_evt)
{
	//�ж��¼�����:���յ��������¼�
    if (p_evt->type == BLE_UARTS_EVT_RX_DATA)
    {
		g_AppConnectTimeCount = 0;
//        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
//        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
//                                nus_data_rcv_buff_len = p_evt->params.rx_data.length;
//                                memcpy(nus_data_rcv_buff[nus_data_write_p++],p_evt->params.rx_data.p_data,nus_data_rcv_buff_len);
//                                if(nus_data_write_p >= NUS_DATA_REC_BUFF_MAX)
//                                        nus_data_write_p = 0;
//                                nus_data_rcv_flag++;                    
        uint32_t err_code;
        //���ڴ�ӡ�����յ�����
		NRF_LOG_HEXDUMP_INFO(p_evt->params.rx_data.p_data,p_evt->params.rx_data.length);
        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }		
                                //Debug_LOG("*****len %d",p_evt->params.rx_data.length);
    }
		//�ж��¼�����:���;����¼������¼��ں����������õ�����ǰ�����ڸ��¼��з�תָʾ��D4��״̬��ָʾ���¼��Ĳ���
    if (p_evt->type == BLE_UARTS_EVT_TX_RDY)
    {
		nrf_gpio_pin_toggle(LED_4);
	}
}


/* ���պ��� main �е��� ��nus_data_rcv_flag ���� */
void my_nus_data_recv_handler_task(void)
{
	uint8_t data_buff[20];
	
	if(nus_data_rcv_flag == 0 && nus_data_read_p == nus_data_write_p)
		return;
	while(nus_data_rcv_flag || nus_data_read_p != nus_data_write_p)
	{
		if(nus_data_rcv_flag)
			nus_data_rcv_flag--;
		memcpy(data_buff,nus_data_rcv_buff[nus_data_read_p++],20);
		if(nus_data_read_p >= NUS_DATA_REC_BUFF_MAX)
			nus_data_read_p = 0;
		if(nus_data_read_p == nus_data_write_p && nus_data_read_p == 0)
			memset(nus_data_rcv_buff,0,NUS_DATA_REC_BUFF_MAX*20);		
		my_data_analysis(data_buff,20);
	}
}

/*�ظ�APPЭ��ָ���װ����*/

void my_data_pack_app(uint8_t Send_ID)
{
    static uint16_t len=0;
    TxBuffer_APP[0] = 0xAA;
    TxBuffer_APP[1] = 0xAA;
    switch(Send_ID)
    {
        case  0xA1:
            len = 0x06;
            break;
        case  0xA2:
            len = 0x06;
            break;
        case  0xA3:
            len = 0x06;
            break;
		case  0xA4:
            len = 0x06;
            break;
        case  0xA5:
            len = 0x06;
            break;
        case  0xA6:
            len = 0x06;
            break;
		case  0xB1:
            len = 0x06;
            break;
        default :
                ;
    }
    TxBuffer_APP[2] = len;
    TxBuffer_APP[3] = Send_ID;
    TxBuffer_APP[len-1] = my_check_code_calculate(&TxBuffer_APP[3],len-4);
	NRF_LOG_INFO("[BLE] send BLE ID : %02x",Send_ID);  
	NRF_LOG_HEXDUMP_INFO(TxBuffer_APP,len);
	my_ble_send(TxBuffer_APP,len,m_conn_handle);
    len=0;
    memset(TxBuffer_APP,0,100);
}

/*APP���ݽ���*/
void my_com_recv_handler(uint8_t const* data, uint16_t len)
{
    //char *decrypted = rsa_decrypt((char*)data, len, priv);
    uint8_t commandID = data[0];//decrypted[0];
	TxBuffer_MCU[4] = data[1];
    switch(commandID)
    {
        case  0xA1:
            my_data_pack_app(0xA1);
			my_dada_pack_mcu(0xA1);
            break;
		case  0xA2:
            my_data_pack_app(0xA2);
			my_dada_pack_mcu(0xA2);
            break;
		case  0xA3:
            my_data_pack_app(0xA3);
			my_dada_pack_mcu(0xA3);
            break;
		case  0xA4:
            my_data_pack_app(0xA4);
			my_dada_pack_mcu(0xA4);
            break;
		case  0xA5:
            my_data_pack_app(0xA5);
			my_dada_pack_mcu(0xA5);
            break;
		case  0xB1:
            my_data_pack_app(0xA5);
            break;
        default :
                ;
    }
}


/* �������պ��� ���ӻ�ģʽ��  ���ڽ���APP������  */
void my_data_analysis(uint8_t const* data, uint16_t len)
{
    uint8_t error;
    static uint16_t recv_buff_point;
    static uint8_t recv_count,recv_size;
    if(m_recv_flag == 0)
    {
        m_recv_large_len = data[2];
        if(data[0]==0xAA && data[1]==0xAA && m_recv_large_len<=MY_BLE_BUFF_SIZE_MAX)
        {

            m_recv_flag = 1;
            memset(m_recv_data,0,MY_BLE_BUFF_SIZE_MAX);
            recv_count = m_recv_large_len/20;
            recv_buff_point = 0;
            recv_size = m_recv_large_len%20;
        }
    }
    if(m_recv_flag == 1)
    {
        if(recv_count>0)
        {
            memcpy(m_recv_data+recv_buff_point,data,20);
            recv_count--;
            recv_buff_point = recv_buff_point + 20;
        }
        if(recv_count == 0 && recv_size == 0)
        {
            uint8_t error = my_check_code_analysis(&m_recv_data[3],m_recv_large_len-3);
            if(error == 0)
            {
				NRF_LOG_HEXDUMP_INFO(m_recv_data,m_recv_large_len);
                my_com_recv_handler(&m_recv_data[3],m_recv_large_len-3);
            }
            else
            {
                NRF_LOG_INFO("[BLE REC] BLE APP int analyse failed!");
                my_data_pack_app(0xB1);
            }
            m_recv_flag = 0;
        }
        else if(recv_count == 0 && recv_size != 0)
        {
            if(m_recv_large_len >20)
                m_recv_flag = 2;
            else
            {
                memcpy(m_recv_data+recv_buff_point,data,recv_size);
                uint8_t error = my_check_code_analysis(&m_recv_data[3],m_recv_large_len-3);
                if(error == 0)
                {
					NRF_LOG_HEXDUMP_INFO(m_recv_data,m_recv_large_len);
                    my_com_recv_handler(&m_recv_data[3],m_recv_large_len-4);
                }
                else
                {
                    NRF_LOG_INFO("[BLE REC] BLE APP analyse failed!");
                    my_data_pack_app(0xB1);
                }
                m_recv_flag = 0;
            }
        }
    }
    else  if(m_recv_flag == 2)
    {
        memcpy(m_recv_data+recv_buff_point,data,recv_size);
        error =my_check_code_analysis(&m_recv_data[3],m_recv_large_len-3);
        if(error == 0)
        {
			NRF_LOG_HEXDUMP_INFO(m_recv_data,m_recv_large_len);
            my_com_recv_handler(&m_recv_data[3],m_recv_large_len-4);
        }
        else
        {
            NRF_LOG_INFO("[BLE REC] BLE APP encrypt analyse failed!");
            my_data_pack_app(0xB1);
        }
        m_recv_flag = 0;
    }
}
//�����ʼ����������ʼ���Ŷ�д��ģ��ͳ�ʼ��Ӧ�ó���ʹ�õķ���
void services_init(void)
{
    ret_code_t         err_code;
	//���崮��͸����ʼ���ṹ��
	ble_uarts_init_t     uarts_init;
	//�����Ŷ�д���ʼ���ṹ�����
    nrf_ble_qwr_init_t qwr_init = {0};
	ble_dfu_buttonless_init_t dfus_init = {0};

    //�Ŷ�д���¼�������
    qwr_init.error_handler = nrf_qwr_error_handler;
    //��ʼ���Ŷ�д��ģ��
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
		//��麯������ֵ
    APP_ERROR_CHECK(err_code);
    
		
	/*------------------���´����ʼ������͸������-------------*/
	//���㴮��͸�������ʼ���ṹ��
	memset(&uarts_init, 0, sizeof(uarts_init));
	//���ô���͸���¼��ص�����
    uarts_init.data_handler = uarts_data_handler;
    //��ʼ������͸������
    err_code = ble_uarts_init(&m_uarts, &uarts_init);
    APP_ERROR_CHECK(err_code);
//	/*------------------��ʼ������͸������-END-----------------*/
//	
//	//��ʼ��DFU����
//	dfus_init.evt_handler = ble_dfu_evt_handler;

//    err_code = ble_dfu_buttonless_init(&dfus_init);
//    APP_ERROR_CHECK(err_code);
	
}

//���Ӳ���Э��ģ���¼�������
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    //�ж��¼����ͣ������¼�����ִ�ж���
	  //���Ӳ���Э��ʧ�ܣ��Ͽ���ǰ����
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
		//���Ӳ���Э�̳ɹ�
		if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
       //���ܴ���;
    }
}

//���Ӳ���Э��ģ��������¼�������nrf_error�����˴�����룬ͨ��nrf_error���Է���������Ϣ
static void conn_params_error_handler(uint32_t nrf_error)
{
    //���������
	  APP_ERROR_HANDLER(nrf_error);
}


//���Ӳ���Э��ģ���ʼ��
void conn_params_init(void)
{
    ret_code_t             err_code;
	  //�������Ӳ���Э��ģ���ʼ���ṹ��
    ble_conn_params_init_t cp_init;
    //����֮ǰ������
    memset(&cp_init, 0, sizeof(cp_init));
    //����ΪNULL����������ȡ���Ӳ���
    cp_init.p_conn_params                  = NULL;
	  //���ӻ�����֪ͨ���״η������Ӳ�����������֮���ʱ������Ϊ5��
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	  //ÿ�ε���sd_ble_gap_conn_param_update()�����������Ӳ������������֮��ļ��ʱ������Ϊ��30��
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	  //�������Ӳ���Э��ǰ�������Ӳ���Э�̵�����������Ϊ��3��
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	  //���Ӳ������´������¼���ʼ��ʱ
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	  //���Ӳ�������ʧ�ܲ��Ͽ�����
    cp_init.disconnect_on_fail             = false;
	  //ע�����Ӳ��������¼����
    cp_init.evt_handler                    = on_conn_params_evt;
	  //ע�����Ӳ������´����¼����
    cp_init.error_handler                  = conn_params_error_handler;
    //���ÿ⺯���������Ӳ������³�ʼ���ṹ��Ϊ�����������ʼ�����Ӳ���Э��ģ��
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

//�㲥�¼�������
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    //�жϹ㲥�¼�����
    switch (ble_adv_evt)
    {
        //���ٹ㲥�����¼������ٹ㲥�������������¼�
		case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
			//���ù㲥ָʾ��Ϊ���ڹ㲥��D1ָʾ����˸��
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        //�㲥IDLE�¼����㲥��ʱ���������¼�
        case BLE_ADV_EVT_IDLE:
			//���ù㲥ָʾ��Ϊ�㲥ֹͣ��D1ָʾ��Ϩ��
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}
//�㲥��ʼ��
void advertising_init(void)
{
    ret_code_t             err_code;
	  //����㲥��ʼ�����ýṹ�����
    ble_advertising_init_t init;
    //����֮ǰ������
    memset(&init, 0, sizeof(init));
    //�豸�������ͣ�ȫ��
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
	  //�Ƿ������ۣ�����
    init.advdata.include_appearance      = false;
	  //Flag:һ��ɷ���ģʽ����֧��BR/EDR
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	  //UUID�ŵ�ɨ����Ӧ����
	  init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;
	
    //���ù㲥ģʽΪ���ٹ㲥
    init.config.ble_adv_fast_enabled  = true;
	  //���ù㲥����͹㲥����ʱ��
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    //�㲥�¼��ص�����
    init.evt_handler = on_adv_evt;
    //��ʼ���㲥
    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);
    //���ù㲥���ñ�ǡ�APP_BLE_CONN_CFG_TAG�����ڸ��ٹ㲥���õı�ǣ�����Ϊδ��Ԥ����һ���������ڽ�����SoftDevice�汾�У�
		//����ʹ��sd_ble_gap_adv_set_configure()�����µĹ㲥����
		//��ǰSoftDevice�汾��S132 V7.0.1�汾��֧�ֵ����㲥������Ϊ1�����APP_BLE_CONN_CFG_TAGֻ��д1��
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

//BLE�¼�������
void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    //�ж�BLE�¼����ͣ������¼�����ִ����Ӧ����
    switch (p_ble_evt->header.evt_id)
    {
        //�Ͽ������¼�
		case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;    
			//��ӡ��ʾ��Ϣ
			NRF_LOG_INFO("Disconnected.");
			g_AppConnectFlag = 0;
			g_AppConnectTimeCount = 0;
            break;
				
        //�����¼�
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
			//����ָʾ��״̬Ϊ����״̬����ָʾ��D1����
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
			g_AppConnectFlag = 1;
			g_AppConnectTimeCount = 0;
			//�������Ӿ��
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			NRF_LOG_INFO("BLE_GAP_EVT_CONNECTED m_conn_handle : %d",m_conn_handle);
			//�����Ӿ��������Ŷ�д��ʵ����������Ŷ�д��ʵ���͸����ӹ��������������ж�����ӵ�ʱ��ͨ��������ͬ���Ŷ�д��ʵ�����ܷ��㵥�������������
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
				
        //PHY�����¼�
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
						//��ӦPHY���¹��
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
		//��ȫ���������¼�
		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            //��֧�����
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
				 
		//ϵͳ���Է������ڵȴ���
		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            //ϵͳ����û�д洢������ϵͳ����
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
        //GATT�ͻ��˳�ʱ�¼�
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout.");
				    //�Ͽ���ǰ����
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
				
        //GATT��������ʱ�¼�
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout.");
				    //�Ͽ���ǰ����
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

void my_ble_disconnect(void)
{
	if(g_AppConnectFlag == 0)
		return;	
    // ���öϿ����ӵĺ���
    uint32_t err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", m_conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", m_conn_handle);
    }
}

//��ʼ��BLEЭ��ջ
void ble_stack_init(void)
{
    ret_code_t err_code;
    //����ʹ��SoftDevice���ú����л����sdk_config.h�ļ��е�Ƶʱ�ӵ����������õ�Ƶʱ��
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    
    //���屣��Ӧ�ó���RAM��ʼ��ַ�ı���
    uint32_t ram_start = 0;
	  //ʹ��sdk_config.h�ļ���Ĭ�ϲ�������Э��ջ����ȡӦ�ó���RAM��ʼ��ַ�����浽����ram_start
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    //ʹ��BLEЭ��ջ
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    //ע��BLE�¼��ص�����
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

//�����㲥���ú������õ�ģʽ����͹㲥��ʼ�������õĹ㲥ģʽһ��
void advertising_start(void)
{
	//ʹ�ù㲥��ʼ�������õĹ㲥ģʽ�����㲥
	ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
	//��麯�����صĴ������
	APP_ERROR_CHECK(err_code);
}



/* 
* �ӻ�ģʽ ���͵�������  
* data in m_send_data
* data len is m_send_len
*/
void my_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle)
{
	uint32_t err_code;
	do
	{
		err_code = ble_uarts_data_send(&m_uarts, data, &len, conn_handle);
		if ((err_code != NRF_ERROR_INVALID_STATE) &&
				(err_code != NRF_ERROR_RESOURCES) &&
				(err_code != NRF_ERROR_NOT_FOUND))
		{
				APP_ERROR_CHECK(err_code);
		}
	} while (err_code == NRF_ERROR_RESOURCES);
	NRF_LOG_INFO("my ble send return : %d\r\n",err_code);

}


void my_ble_init(void)
{
	ble_stack_init();	//��ʼ��Э��ջ
	gap_params_init();	//����GAP����
	gatt_init();		//��ʼ��GATT
	services_init();	//��ʼ������
	advertising_init();	//��ʼ���㲥
	conn_params_init();	//���Ӳ���Э�̳�ʼ��
}

