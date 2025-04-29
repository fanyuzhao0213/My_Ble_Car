/****************************************Copyright (c)************************************************              
**--------------File Info-----------------------------------------------------------------------------
** File name         : main.c
** Last modified Date: 2020-9-25        
** Last Version      :		   
** Descriptions      : 使用的SDK版本-SDK_17.0.2
**						
**----------------------------------------------------------------------------------------------------
** Descriptions      : 串口透传长包传输（最大传输长度244个字节），本例中使用的设备名称是中文设备名称：串口透传
**                   ：增加了对通知是否使能的判断
**                   ：为防止在未连接、通知未使能的情况下串口数据干扰程序，只有通知使能后才初始化串口接收串口数据，通知关闭后，关闭串口
**---------------------------------------------------------------------------------------------------*/

#include "uart.h"
#include "my_fds.h"
#include "my_ble_scan.h"
#include "my_ble.h"
#include "my_ble_mac.h"

char sensor_ble_code[16]= "ABCDEFFYZ1234567";
char sensor_password[16]= "2025042412001234";

//发送的最大数据长度
uint16_t   m_ble_uarts_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            

//该变量用于保存连接句柄，初始值设置为无连接
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; 

//设备名称数组  中文名称：串口透传
const char device_name[12] = {0xE4,0xB8,0xB2,0xE5,0x8F,0xA3,0xE9,0x80,0x8F,0xE4,0xBC,0xA0};


//初始化电源管理模块
static void power_management_init(void)
{
    ret_code_t err_code;
	  //初始化电源管理
    err_code = nrf_pwr_mgmt_init();
	  //检查函数返回的错误代码
    APP_ERROR_CHECK(err_code);
}

//初始化指示灯
static void leds_init(void)
{
    ret_code_t err_code;
    //初始化BSP指示灯
    err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);

}
//初始化APP定时器模块
static void timers_init(void)
{
    //初始化APP定时器模块
    ret_code_t err_code = app_timer_init();
	  //检查返回值
    APP_ERROR_CHECK(err_code);

    //加入创建用户定时任务的代码，创建用户定时任务。 

}
static void log_init(void)
{
    //初始化log程序模块
	  ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    //设置log输出终端（根据sdk_config.h中的配置设置输出终端为UART或者RTT）
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

//空闲状态处理函数。如果没有挂起的日志操作，则睡眠直到下一个事件发生后唤醒系统
static void idle_state_handle(void)
{
    //处理挂起的log
	  if (NRF_LOG_PROCESS() == false)
    {
        //运行电源管理，该函数需要放到主循环里面执行
			  nrf_pwr_mgmt_run();
    }
}


//主函数
int main(void)
{
	ret_code_t err_code;
	//初始化log程序模块
	log_init();
	//使能中断之前，初始化异步SVCI接口到Bootloader
//	err_code = ble_dfu_buttonless_async_svci_init();
//	APP_ERROR_CHECK(err_code);
	
	uart_config();						//初始化串口
	timers_init();						//初始化APP定时器
	my_flash_init();
	leds_init();						//初始唤按键和指示灯
	power_management_init();			//初始化电源管理
	my_ble_init();						//协议栈以及GATT,GAP,广播等初始化
	my_ble_mac_init();					//MAC地址初始化
    my_scan_init();	    				//初始化扫描
	NRF_LOG_INFO("[MAIN] PROJECT_SW_VERSION : %s",PROJECT_SW_VERSION);  
	NRF_LOG_INFO("[MAIN] DEVICE_NAME : %s",DEVICE_NAME);  
	advertising_start();	//启动广播
//    scan_start();		  //启动扫描
//	nrf_gpio_cfg_output(13);	//led pin
//	nrf_gpio_pin_write(13,1);

	nrf_gpio_pin_write(LED_4,1);
//	fds_once_storage(WRITE_ID_CONFIG_INFO);
	//主循环
	while(true)
	{
		my_nus_data_recv_handler_task();			//从机TASK
		//处理挂起的LOG和运行电源管理
		idle_state_handle();
	}
}
