/****************************************Copyright (c)************************************************
** File          name:my_ble_uarts.c
** Last modified Date:          
** Last       Version:		   
** Descriptions      :����͸�������ļ�			
**---------------------------------------------------------------------------------------------------*/

#include "my_ble_uarts_c.h"

#include "sdk_common.h"
#include <stdlib.h>

#include "ble.h"
#include "ble_gattc.h"
#include "ble_srv_common.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME ble_nus_c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

/**@brief Function for intercepting the errors of GATTC and the BLE GATT Queue.
 *
 * @param[in] nrf_error   Error code.
 * @param[in] p_ctx       Parameter from the event handler.
 * @param[in] conn_handle Connection handle.
 */
static void gatt_error_handler(uint32_t   nrf_error,
                               void     * p_ctx,
                               uint16_t   conn_handle)
{
    ble_uarts_c_t * p_ble_uarts_c = (ble_uarts_c_t *)p_ctx;

    NRF_LOG_DEBUG("A GATT Client error has occurred on conn_handle: 0X%X", conn_handle);

    if (p_ble_uarts_c->error_handler != NULL)
    {
        p_ble_uarts_c->error_handler(nrf_error);
    }
}

//����͸��DB�����¼�������
void ble_uarts_c_on_db_disc_evt(ble_uarts_c_t * p_ble_uarts_c, ble_db_discovery_evt_t * p_evt)
{
    ble_uarts_c_evt_t uarts_c_evt;
    memset(&uarts_c_evt,0,sizeof(ble_uarts_c_evt_t));

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    //�ж�UUID�ǲ��Ǵ���͸����UUID������ǵĻ�����ʾ����͸�����������
    if (    (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
        &&  (p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_UARTS_SERVICE)
        &&  (p_evt->params.discovered_db.srv_uuid.type == p_ble_uarts_c->uuid_type))
    {
        //��ȡ������������ô���͸���ͻ����¼�������ʱ��Ϊ�������ݸ��ú��������������
		    for (uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case BLE_UUID_UARTS_RX_CHARACTERISTIC:
                    uarts_c_evt.handles.uarts_rx_handle = p_chars[i].characteristic.handle_value;
                    break;

                case BLE_UUID_UARTS_TX_CHARACTERISTIC:
                    uarts_c_evt.handles.uarts_tx_handle = p_chars[i].characteristic.handle_value;
                    uarts_c_evt.handles.uarts_tx_cccd_handle = p_chars[i].cccd_handle;
                    break;

                default:
                    break;
            }
        }
		    //���ô���͸���ͻ����¼����������ú����ǳ�ʼ������͸���ͻ���ʱע��ģ�
        if (p_ble_uarts_c->evt_handler != NULL)
        {
            uarts_c_evt.conn_handle = p_evt->conn_handle;//�������Ӿ��
            uarts_c_evt.evt_type    = BLE_UARTS_C_EVT_DISCOVERY_COMPLETE;//�����¼�����
            p_ble_uarts_c->evt_handler(p_ble_uarts_c, &uarts_c_evt);//�����¼�������
        }
    }
}
//�ú��������Э��ջ���յ���Handle Value Notification�����ֵ֪ͨ������������Ƿ������ԶԶ��豸�Ĵ���͸��TX���Ե�֪ͨ�� 
//����ǣ��˺������������ݲ�����ת����Ӧ�ó�����Ӧ�ó���������
static void on_hvx(ble_uarts_c_t * p_ble_uarts_c, ble_evt_t const * p_ble_evt)
{
    //�Ǵ���͸��TX���Ե�֪ͨ
    if (   (p_ble_uarts_c->handles.uarts_tx_handle != BLE_GATT_HANDLE_INVALID)
        && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_uarts_c->handles.uarts_tx_handle)
        && (p_ble_uarts_c->evt_handler != NULL))
    {
        ble_uarts_c_evt_t ble_uarts_c_evt;
        //�����¼�����
        ble_uarts_c_evt.evt_type = BLE_UARTS_C_EVT_NUS_TX_EVT;
			  //p_dataָ�����ݴ�ŵĵ�ַ
        ble_uarts_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
			  //��ȡ���ݳ���
        ble_uarts_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;
        //���ô���͸���ͻ����¼����������ú����ǳ�ʼ������͸���ͻ���ʱע��ģ�
        p_ble_uarts_c->evt_handler(p_ble_uarts_c, &ble_uarts_c_evt);
        NRF_LOG_DEBUG("Client sending data.");
    }
}
//��ʼ������͸���ͻ���
uint32_t ble_uarts_c_init(ble_uarts_c_t * p_ble_uarts_c, ble_uarts_c_init_t * p_ble_uarts_c_init)
{
    uint32_t      err_code;
    ble_uuid_t    uart_uuid;
    ble_uuid128_t uarts_base_uuid = UARTS_BASE_UUID;
    //��麯�������Ƿ�Ϸ�
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c_init);
	  VERIFY_PARAM_NOT_NULL(p_ble_uarts_c_init->p_gatt_queue);
    //���Զ����UUID����д�뵽Э��ջ
    err_code = sd_ble_uuid_vs_add(&uarts_base_uuid, &p_ble_uarts_c->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    uart_uuid.type = p_ble_uarts_c->uuid_type;
    uart_uuid.uuid = BLE_UUID_UARTS_SERVICE;
    //��ʼ������͸���ͻ���ʵ���������Ӿ��������ֵ�����ʼ��Ϊ��Ч�������¼��ص�������GATT Queueָ�����GATT Queueʵ��
    p_ble_uarts_c->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ble_uarts_c->evt_handler             = p_ble_uarts_c_init->evt_handler;
	  p_ble_uarts_c->error_handler           = p_ble_uarts_c->error_handler;
    p_ble_uarts_c->handles.uarts_tx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_uarts_c->handles.uarts_rx_handle = BLE_GATT_HANDLE_INVALID;
	  p_ble_uarts_c->p_gatt_queue            = p_ble_uarts_c_init->p_gatt_queue;
    //������͸�������UUIDע���DB����ģ��
    return ble_db_discovery_evt_register(&uart_uuid);
}

void ble_uarts_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_uarts_c_t * p_ble_uarts_c = (ble_uarts_c_t *)p_context;
    //�������Ƿ���ȷ
    if ((p_ble_uarts_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
    //������Ӿ���Ƿ���Ч
    if ( (p_ble_uarts_c->conn_handle != BLE_CONN_HANDLE_INVALID)
       &&(p_ble_uarts_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }
    //�ж��¼�����
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX://֪ͨ��ָʾ�¼�
            on_hvx(p_ble_uarts_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED://���ӶϿ��¼�
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_uarts_c->conn_handle
                    && p_ble_uarts_c->evt_handler != NULL)
            {
                ble_uarts_c_evt_t uarts_c_evt;
                //�����¼�����
                uarts_c_evt.evt_type = BLE_UARTS_C_EVT_DISCONNECTED;
							  //����͸��ʵ���е����Ӿ������Ϊ��Ч
                p_ble_uarts_c->conn_handle = BLE_CONN_HANDLE_INVALID;
							  //���ô���͸���ͻ����¼����������ú����ǳ�ʼ������͸���ͻ���ʱע��ģ�
                p_ble_uarts_c->evt_handler(p_ble_uarts_c, &uarts_c_evt);
            }
            break;

        default:
            break;
    }
}

//ʹ��Notify
static uint32_t cccd_configure(ble_uarts_c_t * p_ble_uarts_c, bool notification_enable)
{
		nrf_ble_gq_req_t cccd_req;
		//ȡ��д������ݣ�ʹ��/�ر�Notify��
    uint8_t          cccd[BLE_CCCD_VALUE_LEN];
    uint16_t         cccd_val = notification_enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    memset(&cccd_req, 0, sizeof(nrf_ble_gq_req_t));
    cccd[0] = LSB_16(cccd_val);
    cccd[1] = MSB_16(cccd_val);
    //��ʼ��д�����
    cccd_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    cccd_req.error_handler.cb            = gatt_error_handler;
    cccd_req.error_handler.p_ctx         = p_ble_uarts_c;
    cccd_req.params.gattc_write.handle   = p_ble_uarts_c->handles.uarts_tx_cccd_handle;//���Ծ������Ϊ�ͻ�����������������(CCCD)�ľ��
    cccd_req.params.gattc_write.len      = BLE_CCCD_VALUE_LEN;//д������ݳ���
    cccd_req.params.gattc_write.offset   = 0;//ƫ��������Ϊ0
    cccd_req.params.gattc_write.p_value  = cccd;//ָ��д�������
    cccd_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_REQ;//д����������Ӧ
    cccd_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;//ִ��׼���õ�д
    //ִ��д����
    return nrf_ble_gq_item_add(p_ble_uarts_c->p_gatt_queue, &cccd_req, p_ble_uarts_c->conn_handle);
}

//дCCCD���ͻ�����������������������ʹ��/�ر�tx������֪ͨ
uint32_t ble_uarts_c_tx_notif_enable(ble_uarts_c_t * p_ble_uarts_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);//��麯�������Ƿ�Ϸ�
    //�����Ӿ����Ч��CCCD�����Ч�����ش�����룺��ǰ״̬��Ч��������ִ�в���
    if ( (p_ble_uarts_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_uarts_c->handles.uarts_tx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
		//дCCCD
    return cccd_configure(p_ble_uarts_c, true);
}
//BLE��������
uint32_t ble_uarts_c_string_send(ble_uarts_c_t * p_ble_uarts_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);

    nrf_ble_gq_req_t write_req;

    memset(&write_req, 0, sizeof(nrf_ble_gq_req_t));
    //������ݳ����Ƿ���ȷ
    if (length > BLE_UARTS_MAX_DATA_LEN)
    {
        NRF_LOG_WARNING("Content too long.");
        return NRF_ERROR_INVALID_PARAM;
    }
		//������Ӿ���Ƿ���Ч
    if (p_ble_uarts_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_WARNING("Connection handle invalid.");
        return NRF_ERROR_INVALID_STATE;
    }
    //��ʼ��д�����
    write_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    write_req.error_handler.cb            = gatt_error_handler;
    write_req.error_handler.p_ctx         = p_ble_uarts_c;
    write_req.params.gattc_write.handle   = p_ble_uarts_c->handles.uarts_rx_handle;//���Ծ������Ϊ����͸��rx�������
    write_req.params.gattc_write.len      = length;//д������ݳ���
    write_req.params.gattc_write.offset   = 0;//ƫ��������Ϊ0
    write_req.params.gattc_write.p_value  = p_string;//ָ��д�������
    write_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_CMD;//д���������Ӧ
    write_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;//ִ��׼���õ�д
    //ִ��д����
    return nrf_ble_gq_item_add(p_ble_uarts_c->p_gatt_queue, &write_req, p_ble_uarts_c->conn_handle);
}

//������
uint32_t ble_uarts_c_handles_assign(ble_uarts_c_t               * p_ble_uarts,
                                    uint16_t                    conn_handle,
                                    ble_uarts_c_handles_t const * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts);
    //�������Ӿ��������͸���ͻ���ʵ��
    p_ble_uarts->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {   //����CCCD���������͸���ͻ���ʵ��
        p_ble_uarts->handles.uarts_tx_cccd_handle = p_peer_handles->uarts_tx_cccd_handle;
		    //����TX����ֵ������͸���ͻ���ʵ��
        p_ble_uarts->handles.uarts_tx_handle      = p_peer_handles->uarts_tx_handle;
		    //����RX����ֵ������͸���ͻ���ʵ��
        p_ble_uarts->handles.uarts_rx_handle      = p_peer_handles->uarts_rx_handle;
    }
    return nrf_ble_gq_conn_handle_register(p_ble_uarts->p_gatt_queue, conn_handle);
}



