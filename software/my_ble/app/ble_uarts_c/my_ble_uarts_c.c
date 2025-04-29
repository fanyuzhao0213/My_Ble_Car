/****************************************Copyright (c)************************************************
** File          name:my_ble_uarts.c
** Last modified Date:          
** Last       Version:		   
** Descriptions      :串口透传服务文件			
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

//串口透传DB发现事件处理函数
void ble_uarts_c_on_db_disc_evt(ble_uarts_c_t * p_ble_uarts_c, ble_db_discovery_evt_t * p_evt)
{
    ble_uarts_c_evt_t uarts_c_evt;
    memset(&uarts_c_evt,0,sizeof(ble_uarts_c_evt_t));

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    //判断UUID是不是串口透传的UUID，如果是的话，表示串口透传服务发现完成
    if (    (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
        &&  (p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_UARTS_SERVICE)
        &&  (p_evt->params.discovered_db.srv_uuid.type == p_ble_uarts_c->uuid_type))
    {
        //获取特征句柄，调用串口透传客户端事件处理函数时作为参数传递给该函数，由其分配句柄
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
		    //调用串口透传客户端事件处理函数（该函数是初始化串口透传客户端时注册的）
        if (p_ble_uarts_c->evt_handler != NULL)
        {
            uarts_c_evt.conn_handle = p_evt->conn_handle;//保存连接句柄
            uarts_c_evt.evt_type    = BLE_UARTS_C_EVT_DISCOVERY_COMPLETE;//设置事件类型
            p_ble_uarts_c->evt_handler(p_ble_uarts_c, &uarts_c_evt);//调用事件处理函数
        }
    }
}
//该函数处理从协议栈接收到的Handle Value Notification（句柄值通知），并检查它是否是来自对端设备的串口透传TX特性的通知。 
//如果是，此函数将解码数据并将其转发到应用程序，由应用程序处理数据
static void on_hvx(ble_uarts_c_t * p_ble_uarts_c, ble_evt_t const * p_ble_evt)
{
    //是串口透传TX特性的通知
    if (   (p_ble_uarts_c->handles.uarts_tx_handle != BLE_GATT_HANDLE_INVALID)
        && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_uarts_c->handles.uarts_tx_handle)
        && (p_ble_uarts_c->evt_handler != NULL))
    {
        ble_uarts_c_evt_t ble_uarts_c_evt;
        //设置事件类型
        ble_uarts_c_evt.evt_type = BLE_UARTS_C_EVT_NUS_TX_EVT;
			  //p_data指向数据存放的地址
        ble_uarts_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
			  //获取数据长度
        ble_uarts_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;
        //调用串口透传客户端事件处理函数（该函数是初始化串口透传客户端时注册的）
        p_ble_uarts_c->evt_handler(p_ble_uarts_c, &ble_uarts_c_evt);
        NRF_LOG_DEBUG("Client sending data.");
    }
}
//初始化串口透传客户端
uint32_t ble_uarts_c_init(ble_uarts_c_t * p_ble_uarts_c, ble_uarts_c_init_t * p_ble_uarts_c_init)
{
    uint32_t      err_code;
    ble_uuid_t    uart_uuid;
    ble_uuid128_t uarts_base_uuid = UARTS_BASE_UUID;
    //检查函数参数是否合法
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c_init);
	  VERIFY_PARAM_NOT_NULL(p_ble_uarts_c_init->p_gatt_queue);
    //将自定义的UUID基数写入到协议栈
    err_code = sd_ble_uuid_vs_add(&uarts_base_uuid, &p_ble_uarts_c->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    uart_uuid.type = p_ble_uarts_c->uuid_type;
    uart_uuid.uuid = BLE_UUID_UARTS_SERVICE;
    //初始化串口透传客户端实例：将连接句柄、特征值句柄初始化为无效，保存事件回调函数，GATT Queue指向定义的GATT Queue实例
    p_ble_uarts_c->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ble_uarts_c->evt_handler             = p_ble_uarts_c_init->evt_handler;
	  p_ble_uarts_c->error_handler           = p_ble_uarts_c->error_handler;
    p_ble_uarts_c->handles.uarts_tx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_uarts_c->handles.uarts_rx_handle = BLE_GATT_HANDLE_INVALID;
	  p_ble_uarts_c->p_gatt_queue            = p_ble_uarts_c_init->p_gatt_queue;
    //将串口透传服务的UUID注册给DB发现模块
    return ble_db_discovery_evt_register(&uart_uuid);
}

void ble_uarts_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_uarts_c_t * p_ble_uarts_c = (ble_uarts_c_t *)p_context;
    //检查参数是否正确
    if ((p_ble_uarts_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
    //检查连接句柄是否有效
    if ( (p_ble_uarts_c->conn_handle != BLE_CONN_HANDLE_INVALID)
       &&(p_ble_uarts_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }
    //判断事件类型
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX://通知或指示事件
            on_hvx(p_ble_uarts_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED://连接断开事件
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_uarts_c->conn_handle
                    && p_ble_uarts_c->evt_handler != NULL)
            {
                ble_uarts_c_evt_t uarts_c_evt;
                //设置事件类型
                uarts_c_evt.evt_type = BLE_UARTS_C_EVT_DISCONNECTED;
							  //串口透传实例中的连接句柄设置为无效
                p_ble_uarts_c->conn_handle = BLE_CONN_HANDLE_INVALID;
							  //调用串口透传客户端事件处理函数（该函数是初始化串口透传客户端时注册的）
                p_ble_uarts_c->evt_handler(p_ble_uarts_c, &uarts_c_evt);
            }
            break;

        default:
            break;
    }
}

//使能Notify
static uint32_t cccd_configure(ble_uarts_c_t * p_ble_uarts_c, bool notification_enable)
{
		nrf_ble_gq_req_t cccd_req;
		//取得写入的数据（使能/关闭Notify）
    uint8_t          cccd[BLE_CCCD_VALUE_LEN];
    uint16_t         cccd_val = notification_enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    memset(&cccd_req, 0, sizeof(nrf_ble_gq_req_t));
    cccd[0] = LSB_16(cccd_val);
    cccd[1] = MSB_16(cccd_val);
    //初始化写入参数
    cccd_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    cccd_req.error_handler.cb            = gatt_error_handler;
    cccd_req.error_handler.p_ctx         = p_ble_uarts_c;
    cccd_req.params.gattc_write.handle   = p_ble_uarts_c->handles.uarts_tx_cccd_handle;//属性句柄设置为客户端特征配置描述符(CCCD)的句柄
    cccd_req.params.gattc_write.len      = BLE_CCCD_VALUE_LEN;//写入的数据长度
    cccd_req.params.gattc_write.offset   = 0;//偏移量设置为0
    cccd_req.params.gattc_write.p_value  = cccd;//指向写入的数据
    cccd_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_REQ;//写入请求，有响应
    cccd_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;//执行准备好的写
    //执行写操作
    return nrf_ble_gq_item_add(p_ble_uarts_c->p_gatt_queue, &cccd_req, p_ble_uarts_c->conn_handle);
}

//写CCCD（客户端特征配置描述符），即使能/关闭tx特征的通知
uint32_t ble_uarts_c_tx_notif_enable(ble_uarts_c_t * p_ble_uarts_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);//检查函数参数是否合法
    //若连接句柄无效或CCCD句柄无效，返回错误代码：当前状态无效，不允许执行操作
    if ( (p_ble_uarts_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_uarts_c->handles.uarts_tx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
		//写CCCD
    return cccd_configure(p_ble_uarts_c, true);
}
//BLE发送数据
uint32_t ble_uarts_c_string_send(ble_uarts_c_t * p_ble_uarts_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts_c);

    nrf_ble_gq_req_t write_req;

    memset(&write_req, 0, sizeof(nrf_ble_gq_req_t));
    //检查数据长度是否正确
    if (length > BLE_UARTS_MAX_DATA_LEN)
    {
        NRF_LOG_WARNING("Content too long.");
        return NRF_ERROR_INVALID_PARAM;
    }
		//检查连接句柄是否有效
    if (p_ble_uarts_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_WARNING("Connection handle invalid.");
        return NRF_ERROR_INVALID_STATE;
    }
    //初始化写入参数
    write_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    write_req.error_handler.cb            = gatt_error_handler;
    write_req.error_handler.p_ctx         = p_ble_uarts_c;
    write_req.params.gattc_write.handle   = p_ble_uarts_c->handles.uarts_rx_handle;//属性句柄设置为串口透传rx特征句柄
    write_req.params.gattc_write.len      = length;//写入的数据长度
    write_req.params.gattc_write.offset   = 0;//偏移量设置为0
    write_req.params.gattc_write.p_value  = p_string;//指向写入的数据
    write_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_CMD;//写入命令，无响应
    write_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;//执行准备好的写
    //执行写操作
    return nrf_ble_gq_item_add(p_ble_uarts_c->p_gatt_queue, &write_req, p_ble_uarts_c->conn_handle);
}

//分配句柄
uint32_t ble_uarts_c_handles_assign(ble_uarts_c_t               * p_ble_uarts,
                                    uint16_t                    conn_handle,
                                    ble_uarts_c_handles_t const * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_uarts);
    //保存连接句柄到串口透传客户端实例
    p_ble_uarts->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {   //保存CCCD句柄到串口透传客户端实例
        p_ble_uarts->handles.uarts_tx_cccd_handle = p_peer_handles->uarts_tx_cccd_handle;
		    //保存TX特征值到串口透传客户端实例
        p_ble_uarts->handles.uarts_tx_handle      = p_peer_handles->uarts_tx_handle;
		    //保存RX特征值到串口透传客户端实例
        p_ble_uarts->handles.uarts_rx_handle      = p_peer_handles->uarts_rx_handle;
    }
    return nrf_ble_gq_conn_handle_register(p_ble_uarts->p_gatt_queue, conn_handle);
}



