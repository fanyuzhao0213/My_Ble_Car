#include "my_ble.h"
#include "check.h"
#include "uart.h"

bool uart_enabled = false;

NRF_BLE_GATT_DEF(m_gatt);                                //定义名称为m_gatt的GATT模块实例
NRF_BLE_QWR_DEF(m_qwr);                                  //定义一个名称为m_qwr的排队写入实例
BLE_ADVERTISING_DEF(m_advertising);                      //定义名称为m_advertising的广播模块实例
BLE_UARTS_DEF(m_uarts, NRF_SDH_BLE_TOTAL_LINK_COUNT);    //定义名称为m_uarts的串口透传服务实例

#define NUS_DATA_REC_BUFF_MAX  		50
#define MY_BLE_BUFF_SIZE_MAX        200            //200

uint16_t    m_recv_large_len = 0;
static char m_recv_flag = 0;
uint8_t     m_recv_data[MY_BLE_BUFF_SIZE_MAX];


uint8_t TxBuffer_APP[100]={0}; 		//APP发送缓冲buff
static uint8_t nus_data_rcv_buff[NUS_DATA_REC_BUFF_MAX][20];
static uint8_t nus_data_rcv_buff_len = 0;
static uint16_t nus_data_write_p = 0;
static uint16_t nus_data_read_p = 0;
static uint16_t nus_data_rcv_flag = 0; //in handle ++; in analysis --

//定义串口透传服务UUID列表
ble_uuid_t m_adv_uuids[]=                                          
{
    {BLE_UUID_UARTS_SERVICE, UARTS_SERVICE_UUID_TYPE}
};

//排队写入事件处理函数，用于处理排队写入模块的错误
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    //检查错误代码
	APP_ERROR_HANDLER(nrf_error);
}

//关机准备处理程序。在关闭过程中，将以1秒的间隔调用此函数，直到函数返回true。当函数返回true时，表示应用程序已准备好复位为DFU模式
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


//注册优先级为0的应用程序关闭处理程序
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);
//SoftDevice状态监视者事件处理函数
static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
			  //表明Softdevice在复位之前已经禁用，告之bootloader启动时应跳过CRC
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //进入system off模式
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

//注册SoftDevice状态监视者，用于SoftDevice状态改变或者即将改变时接收SoftDevice事件
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};

//获取广播模式、间隔和超时时间
static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}

//断开当前连接，设备准备进入bootloader之前，需要先断开连接
static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);
    //断开当前连接
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

//DFU事件处理函数。如果需要在DFU事件中执行操作，可以在相应的事件里面加入处理代码
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE://该事件指示设备正在准备进入bootloader
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
            //阻止设备在断开连接时广播
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
			//连接断开后设备不自动进行广播
            config.ble_adv_on_disconnect_disabled = true;
			//修改广播配置
            ble_advertising_modes_config_set(&m_advertising, &config);

			//断开当前已经连接的所有其他绑定设备。在设备固件更新成功（或中止）后，需要在启动时接收服务更改指示
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER://该事件指示函数返回后设备即进入bootloader
            //如果应用程序有数据需要保存到Flash，通过app_shutdown_handler返回flase以延迟复位，从而保证数据正确写入到Flash
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED://该事件指示进入bootloader失败
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
			//进入bootloader失败，应用程序需要采取纠正措施来处理问题，如使用APP_ERROR_CHECK复位设备
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR://该事件指示发送响应失败
            NRF_LOG_ERROR("Request to send a response to client failed.");
			//发送响应失败，应用程序需要采取纠正措施来处理问题，如使用APP_ERROR_CHECK复位设备
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}



//GAP参数初始化，该函数配置需要的GAP参数，包括设备名称，外观特征、首选连接参数
void gap_params_init(void)
{
    ret_code_t              err_code;
	  //定义连接参数结构体变量
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    //设置GAP的安全模式
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //设置GAP设备名称，使用英文设备名称
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)DEVICE_NAME,
                                              strlen(DEVICE_NAME));
	
//	//设置GAP设备名称，这里使用了中文设备名称
//    err_code = sd_ble_gap_device_name_set(&sec_mode,
//                                          (const uint8_t *)device_name,
//                                          sizeof(device_name));
//																					
    //检查函数返回的错误代码
		APP_ERROR_CHECK(err_code);
																				
    //设置首选连接参数，设置前先清零gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;//最小连接间隔
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;//最小连接间隔
    gap_conn_params.slave_latency     = SLAVE_LATENCY;    //从机延迟
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT; //监督超时
    //调用协议栈API sd_ble_gap_ppcp_set配置GAP参数
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
																					
}
//GATT事件处理函数，该函数中处理MTU交换事件
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    //如果是MTU交换事件
	  if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //设置串口透传服务的有效数据长度（MTU-opcode-handle）
		m_ble_uarts_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_uarts_max_data_len, m_ble_uarts_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}
//初始化GATT程序模块
void gatt_init(void)
{
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);    //初始化GATT程序模块
    APP_ERROR_CHECK(err_code);		//检查函数返回的错误代码
	err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);	  //设置ATT MTU的大小,这里设置的值为247
    APP_ERROR_CHECK(err_code);		//检查函数返回的错误代码
}

//串口透传事件回调函数，串口透出服务初始化时注册
void uarts_data_handler(ble_uarts_evt_t * p_evt)
{
	//判断事件类型:接收到新数据事件
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
        //串口打印出接收的数据
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
//		//判断事件类型:发送就绪事件，该事件在后面的试验会用到，当前我们在该事件中翻转指示灯D4的状态，指示该事件的产生
//    if (p_evt->type == BLE_UARTS_EVT_TX_RDY)
//    {
//		nrf_gpio_pin_toggle(LED_4);
//	}
}


/* 接收函数 main 中调用 ，nus_data_rcv_flag 触发 */
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

/*回复APP协议指令封装函数*/

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

/*APP数据解析*/
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


/* 蓝牙接收函数 （从机模式）  用于接收APP端数据  */
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
//服务初始化，包含初始化排队写入模块和初始化应用程序使用的服务
void services_init(void)
{
    ret_code_t         err_code;
	//定义串口透传初始化结构体
	ble_uarts_init_t     uarts_init;
	//定义排队写入初始化结构体变量
    nrf_ble_qwr_init_t qwr_init = {0};
	ble_dfu_buttonless_init_t dfus_init = {0};

    //排队写入事件处理函数
    qwr_init.error_handler = nrf_qwr_error_handler;
    //初始化排队写入模块
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
		//检查函数返回值
    APP_ERROR_CHECK(err_code);
    
		
	/*------------------以下代码初始化串口透传服务-------------*/
	//清零串口透传服务初始化结构体
	memset(&uarts_init, 0, sizeof(uarts_init));
	//设置串口透传事件回调函数
    uarts_init.data_handler = uarts_data_handler;
    //初始化串口透传服务
    err_code = ble_uarts_init(&m_uarts, &uarts_init);
    APP_ERROR_CHECK(err_code);
//	/*------------------初始化串口透传服务-END-----------------*/
//	
//	//初始化DFU服务
//	dfus_init.evt_handler = ble_dfu_evt_handler;

//    err_code = ble_dfu_buttonless_init(&dfus_init);
//    APP_ERROR_CHECK(err_code);
	
}

//连接参数协商模块事件处理函数
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    //判断事件类型，根据事件类型执行动作
	  //连接参数协商失败，断开当前连接
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
		//连接参数协商成功
		if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
       //功能代码;
    }
}

//连接参数协商模块错误处理事件，参数nrf_error包含了错误代码，通过nrf_error可以分析错误信息
static void conn_params_error_handler(uint32_t nrf_error)
{
    //检查错误代码
	  APP_ERROR_HANDLER(nrf_error);
}


//连接参数协商模块初始化
void conn_params_init(void)
{
    ret_code_t             err_code;
	  //定义连接参数协商模块初始化结构体
    ble_conn_params_init_t cp_init;
    //配置之前先清零
    memset(&cp_init, 0, sizeof(cp_init));
    //设置为NULL，从主机获取连接参数
    cp_init.p_conn_params                  = NULL;
	  //连接或启动通知到首次发起连接参数更新请求之间的时间设置为5秒
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	  //每次调用sd_ble_gap_conn_param_update()函数发起连接参数更新请求的之间的间隔时间设置为：30秒
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	  //放弃连接参数协商前尝试连接参数协商的最大次数设置为：3次
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	  //连接参数更新从连接事件开始计时
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	  //连接参数更新失败不断开连接
    cp_init.disconnect_on_fail             = false;
	  //注册连接参数更新事件句柄
    cp_init.evt_handler                    = on_conn_params_evt;
	  //注册连接参数更新错误事件句柄
    cp_init.error_handler                  = conn_params_error_handler;
    //调用库函数（以连接参数更新初始化结构体为输入参数）初始化连接参数协商模块
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

//广播事件处理函数
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    //判断广播事件类型
    switch (ble_adv_evt)
    {
        //快速广播启动事件：快速广播启动后会产生该事件
		case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
			//设置广播指示灯为正在广播（D1指示灯闪烁）
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        //广播IDLE事件：广播超时后会产生该事件
        case BLE_ADV_EVT_IDLE:
			//设置广播指示灯为广播停止（D1指示灯熄灭）
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}
void advertising_init(void) 
{ 
    ret_code_t err_code; 
    //定义广播初始化配置结构体变量 
    ble_advertising_init_t init; 
    //配置之前先清零 
    memset(&init, 0, sizeof(init)); 

    // 获取设备MAC地址
    ble_gap_addr_t device_addr;
    err_code = sd_ble_gap_addr_get(&device_addr);
    APP_ERROR_CHECK(err_code);

    // 配置制造商特定数据
    ble_advdata_manuf_data_t manuf_specific_data;
    manuf_specific_data.company_identifier = 0x1234;  // 使用自定义公司标识符
    manuf_specific_data.data.p_data = &device_addr.addr[0];
    manuf_specific_data.data.size = 6;

    //设备名称类型：全称 
    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    //是否包含外观：不包含 
    init.advdata.include_appearance = false;
    //添加制造商特定数据
    init.advdata.p_manuf_specific_data = &manuf_specific_data;
    //Flag:一般可发现模式，不支持BR/EDR 
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    //UUID放到扫描响应里面 
    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]); 
    init.srdata.uuids_complete.p_uuids = m_adv_uuids; 

    //设置广播模式为快速广播 
    init.config.ble_adv_fast_enabled = true; 
    //设置广播间隔和广播持续时间 
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL; 
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION; 
    //广播事件回调函数 
    init.evt_handler = on_adv_evt; 
    //初始化广播 
    err_code = ble_advertising_init(&m_advertising, &init); 
    APP_ERROR_CHECK(err_code); 
    //设置广播配置标记
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG); 
} 

//BLE事件处理函数
void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    //判断BLE事件类型，根据事件类型执行相应操作
    switch (p_ble_evt->header.evt_id)
    {
        //断开连接事件
		case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;    
			//打印提示信息
			NRF_LOG_INFO("Disconnected.");
			g_AppConnectFlag = 0;
			g_AppConnectTimeCount = 0;
            break;
				
        //连接事件
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
			//设置指示灯状态为连接状态，即指示灯D1常亮
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
			g_AppConnectFlag = 1;
			g_AppConnectTimeCount = 0;
			//保存连接句柄
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			NRF_LOG_INFO("BLE_GAP_EVT_CONNECTED m_conn_handle : %d",m_conn_handle);
			//将连接句柄分配给排队写入实例，分配后排队写入实例和该连接关联，这样，当有多个连接的时候，通过关联不同的排队写入实例，很方便单独处理各个连接
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
				
        //PHY更新事件
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
						//响应PHY更新规程
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
		//安全参数请求事件
		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            //不支持配对
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
				 
		//系统属性访问正在等待中
		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            //系统属性没有存储，更新系统属性
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
        //GATT客户端超时事件
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout.");
				    //断开当前连接
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
				
        //GATT服务器超时事件
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout.");
				    //断开当前连接
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
    // 调用断开连接的函数
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

//初始化BLE协议栈
void ble_stack_init(void)
{
    ret_code_t err_code;
	
    //请求使能SoftDevice，该函数中会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    err_code = nrf_sdh_enable_request();
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_INFO("SoftDevice enable failed with error: %d\r\n", err_code);
		// 可以添加更多错误处理
		return;
	}
    APP_ERROR_CHECK(err_code);
    
    //定义保存应用程序RAM起始地址的变量
    uint32_t ram_start = 0;
	  //使用sdk_config.h文件的默认参数配置协议栈，获取应用程序RAM起始地址，保存到变量ram_start
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    //使能BLE协议栈
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    //注册BLE事件回调函数
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

//启动广播，该函数所用的模式必须和广播初始化中设置的广播模式一样
void advertising_start(void)
{
	//使用广播初始化中设置的广播模式启动广播
	ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
	//检查函数返回的错误代码
	APP_ERROR_CHECK(err_code);
}



/* 
* 从机模式 发送单包数据  
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

/* 
* 从机模式 发送单包数据  
* data in m_send_data
* data len is m_send_len
*/
void my_uart_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle)
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
	ble_stack_init();	//初始化协议栈
	gap_params_init();	//配置GAP参数
	gatt_init();		//初始化GATT
	services_init();	//初始化服务
	advertising_init();	//初始化广播
	conn_params_init();	//连接参数协商初始化
}

