/****************************************Copyright (c)************************************************              
**--------------File Info-----------------------------------------------------------------------------
** File name         : main.c
** Last modified Date: 2020-9-25        
** Last Version      :		   
** Descriptions      : ʹ�õ�SDK�汾-SDK_17.0.2
**						
**----------------------------------------------------------------------------------------------------
** Descriptions      : ����͸���������䣨����䳤��244���ֽڣ���������ʹ�õ��豸�����������豸���ƣ�����͸��
**                   �������˶�֪ͨ�Ƿ�ʹ�ܵ��ж�
**                   ��Ϊ��ֹ��δ���ӡ�֪ͨδʹ�ܵ�����´������ݸ��ų���ֻ��֪ͨʹ�ܺ�ų�ʼ�����ڽ��մ������ݣ�֪ͨ�رպ󣬹رմ���
**---------------------------------------------------------------------------------------------------*/

#include "uart.h"
#include "my_fds.h"
#include "my_ble_scan.h"
#include "my_ble.h"
#include "my_ble_mac.h"

char sensor_ble_code[16]= "ABCDEFFYZ1234567";
char sensor_password[16]= "2025042412001234";

//���͵�������ݳ���
uint16_t   m_ble_uarts_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            

//�ñ������ڱ������Ӿ������ʼֵ����Ϊ������
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; 

//�豸��������  �������ƣ�����͸��
const char device_name[12] = {0xE4,0xB8,0xB2,0xE5,0x8F,0xA3,0xE9,0x80,0x8F,0xE4,0xBC,0xA0};


//��ʼ����Դ����ģ��
static void power_management_init(void)
{
    ret_code_t err_code;
	  //��ʼ����Դ����
    err_code = nrf_pwr_mgmt_init();
	  //��麯�����صĴ������
    APP_ERROR_CHECK(err_code);
}

//��ʼ��ָʾ��
static void leds_init(void)
{
    ret_code_t err_code;
    //��ʼ��BSPָʾ��
    err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);

}
//��ʼ��APP��ʱ��ģ��
static void timers_init(void)
{
    //��ʼ��APP��ʱ��ģ��
    ret_code_t err_code = app_timer_init();
	  //��鷵��ֵ
    APP_ERROR_CHECK(err_code);

    //���봴���û���ʱ����Ĵ��룬�����û���ʱ���� 

}
static void log_init(void)
{
    //��ʼ��log����ģ��
	  ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    //����log����նˣ�����sdk_config.h�е�������������ն�ΪUART����RTT��
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

//����״̬�����������û�й������־��������˯��ֱ����һ���¼���������ϵͳ
static void idle_state_handle(void)
{
    //��������log
	  if (NRF_LOG_PROCESS() == false)
    {
        //���е�Դ�����ú�����Ҫ�ŵ���ѭ������ִ��
			  nrf_pwr_mgmt_run();
    }
}


//������
int main(void)
{
	ret_code_t err_code;
	//��ʼ��log����ģ��
	log_init();
	//ʹ���ж�֮ǰ����ʼ���첽SVCI�ӿڵ�Bootloader
//	err_code = ble_dfu_buttonless_async_svci_init();
//	APP_ERROR_CHECK(err_code);
	
	uart_config();						//��ʼ������
	timers_init();						//��ʼ��APP��ʱ��
	my_flash_init();
	leds_init();						//��ʼ��������ָʾ��
	power_management_init();			//��ʼ����Դ����
	my_ble_init();						//Э��ջ�Լ�GATT,GAP,�㲥�ȳ�ʼ��
	my_ble_mac_init();					//MAC��ַ��ʼ��
    my_scan_init();	    				//��ʼ��ɨ��
	NRF_LOG_INFO("[MAIN] PROJECT_SW_VERSION : %s",PROJECT_SW_VERSION);  
	NRF_LOG_INFO("[MAIN] DEVICE_NAME : %s",DEVICE_NAME);  
	advertising_start();	//�����㲥
//    scan_start();		  //����ɨ��
//	nrf_gpio_cfg_output(13);	//led pin
//	nrf_gpio_pin_write(13,1);

	nrf_gpio_pin_write(LED_4,1);
//	fds_once_storage(WRITE_ID_CONFIG_INFO);
	//��ѭ��
	while(true)
	{
		my_nus_data_recv_handler_task();			//�ӻ�TASK
		//��������LOG�����е�Դ����
		idle_state_handle();
	}
}
