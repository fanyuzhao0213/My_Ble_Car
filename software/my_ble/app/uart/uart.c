#include "uart.h"
#include "my_ble.h"
#include "check.h"

uint8_t     TxBuffer_MCU[100]= {0};                     //MCU���ͻ���buff


/*����MCU����*/
void my_ble_recv_large_cc(uint8_t const* data, uint16_t len)
{
    uint8_t     i=0,j=0;
    uint32_t    key_num = 0;
    uint8_t     idFlagcount = 0;
	NRF_LOG_INFO("[UART] REV MCU ID : %02x",data[3]);  
    if(data[0]==0xAA && data[1]==0xAA)
    {
        switch(data[3])
        {
            case 0xA1:
				break;
			case 0xA2:	
				break;
			case 0xA3:	
				break;
			case 0xA4:	
				break;	
				break;
			case 0xA5:	
				break;
            default:
				break;
		}
	}
}		
//�����¼��ص����������ڳ�ʼ��ʱע�ᣬ�ú������ж��¼����Ͳ����д���
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t  data_array[1024];
    static uint8_t  index = 0;
    static uint8_t  rece_flag = 0;
    static uint8_t  rece_len = 0;
    uint8_t  ret = 0;
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            switch (rece_flag)
            {
                case 0:
                    if(data_array[index] ==0xAA)
                    {
                        rece_flag++;
                        index++;
                    }
                    else
                    {
                        rece_flag=0;
                        index = 0;
                    }
                    break;
                case 1:
                    if(data_array[index] ==0xAA)
                    {
                        rece_flag++;
                        index++;
                    }
                    else
                    {
                        rece_flag=0;
                        index = 0;
                    }
                    break;
                case 2:
                        rece_flag++;
                        rece_len = data_array[index];
                        index++;
                    break;
                case 3:
                    if(rece_len > index+1)
                    {
                        index++;
                    }
                    else
                    {
                        ret = my_check_code_analysis(&data_array[3],rece_len-3);
                        if(ret != 0){
                            rece_flag = 0;
                            rece_len =0;
                            index = 0;
                            return;
                        }
						NRF_LOG_INFO("[UART] REV MCU ID : %02x",data_array[3]);  
						NRF_LOG_HEXDUMP_INFO(data_array,rece_len);
//                        my_ble_recv_large_cc(data_array,rece_len);//����MCU����
						my_uart_ble_send(data_array,rece_len,m_conn_handle);
                        rece_flag = 0;
                        rece_len =0;
                        index = 0;
                    }
                    break;
                default:
                    rece_flag = 0;
                    index = 0;
                    break;
                }
            break;
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
        //ͨѶ�����¼������������
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        default:
            break;
    }
}

//��������
void uart_config(void)
{
	uint32_t err_code;
	
	//���崮��ͨѶ�������ýṹ�岢��ʼ��
	const app_uart_comm_params_t comm_params =
	{
		RX_PIN_NUMBER,//����uart��������
		TX_PIN_NUMBER,//����uart��������
		RTS_PIN_NUMBER,//����uart RTS���ţ����عرպ���Ȼ������RTS��CTS���ţ����������������ԣ������������������ţ����������Կ���ΪIOʹ��
		CTS_PIN_NUMBER,//����uart CTS����
		APP_UART_FLOW_CONTROL_DISABLED,//�ر�uartӲ������
		false,//��ֹ��ż����
		NRF_UART_BAUDRATE_115200//uart����������Ϊ115200bps
	};
	//��ʼ�����ڣ�ע�ᴮ���¼��ص�����
	APP_UART_FIFO_INIT(&comm_params,
						 UART_RX_BUF_SIZE,
						 UART_TX_BUF_SIZE,
						 uart_event_handle,
						 APP_IRQ_PRIORITY_LOWEST,
						 err_code);

	APP_ERROR_CHECK(err_code);

}

/*�������MCU��������*/
void my_dada_pack_mcu(uint8_t Send_ID)
{
    static uint16_t len=0;
    TxBuffer_MCU[0] = 0xAA;
    TxBuffer_MCU[1] = 0xAA;
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
        default :
                ;
    }
    TxBuffer_MCU[2] = len;
    TxBuffer_MCU[3] = Send_ID;
    TxBuffer_MCU[len-1] = my_check_code_calculate(&TxBuffer_MCU[4],len-5);
	NRF_LOG_INFO("[BLE] send MCU ID : %02x",Send_ID);  
	NRF_LOG_HEXDUMP_INFO(TxBuffer_MCU,len);
    for (uint32_t i = 0; i < len; i++)
    {
//		Debug_LOG("send data to mcu,%02x",TxBuffer_MCU[i]);
        app_uart_put(TxBuffer_MCU[i]);
    }
}

void uart_reconfig(void)
{
	if(uart_enabled == false)//��ʼ������
	{
		uart_config();
		uart_enabled = true;
	}
	else
	{
		app_uart_close();//����ʼ������
		uart_enabled = false;
	}
}




