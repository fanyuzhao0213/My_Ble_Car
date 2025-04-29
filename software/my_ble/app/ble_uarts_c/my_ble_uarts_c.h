
#ifndef BLE_MY_UARTS_C_H__
#define BLE_MY_UARTS_C_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_gatt.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"
#include "ble_srv_common.h"

#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif


//���崮��͸���ͻ���ʵ������ʵ�����2������
//1��������static���ʹ���͸���ͻ��˽ṹ�������Ϊ����͸���ͻ��˽ṹ��������ڴ�
//2��ע����BLE�¼������ߣ���ʹ�ô���͸���ͻ��˳���ģ����Խ���BLEЭ��ջ���¼����Ӷ�������ble_uarts_c_on_ble_evt()�¼��ص������д����Լ�����Ȥ���¼�
#define BLE_UARTS_C_DEF(_name)                                                                      \
static ble_uarts_c_t _name;                                                                         \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_NUS_C_BLE_OBSERVER_PRIO,                                                   \
                     ble_uarts_c_on_ble_evt, &_name)

//���崮��͸���ͻ���ʵ���飬�������Ӷ���ӻ�ʱʹ��
#define BLE_NUS_C_ARRAY_DEF(_name, _cnt)                 \
static ble_uarts_c_t _name[_cnt];                        \
NRF_SDH_BLE_OBSERVERS(_name ## _obs,                     \
                      BLE_NUS_C_BLE_OBSERVER_PRIO,       \
                      ble_uarts_c_on_ble_evt, &_name, _cnt)
	
//���崮��͸������128λUUID����
#define UARTS_BASE_UUID                   {{0x40, 0xE3, 0x4A, 0x1D, 0xC2, 0x5F, 0xB0, 0x9C, 0xB7, 0x47, 0xE6, 0x43, 0x00, 0x00, 0x53, 0x86}} 

//��������������16λUUID
#define BLE_UUID_UARTS_SERVICE           0x000A    //����͸������16λUUID
#define BLE_UUID_UARTS_TX_CHARACTERISTIC 0x000B    //TX����16λUUID           
#define BLE_UUID_UARTS_RX_CHARACTERISTIC 0x000C    //RX����16λUUID

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_UARTS_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_UARTS_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif


//����ת����Ӧ�ó�����¼�����
typedef enum
{
    BLE_UARTS_C_EVT_DISCOVERY_COMPLETE,   //���������
    BLE_UARTS_C_EVT_NUS_TX_EVT,           //ָʾ�������յ��ӻ�����Ϣ
    BLE_UARTS_C_EVT_DISCONNECTED          //���ӶϿ�
} ble_uarts_c_evt_type_t;

/**@brief Handles on the connected peer device needed to interact with it. */
typedef struct
{
    uint16_t uarts_tx_handle;      /**< Handle of the NUS TX characteristic as provided by a discovery. */
    uint16_t uarts_tx_cccd_handle; /**< Handle of the CCCD of the NUS TX characteristic as provided by a discovery. */
    uint16_t uarts_rx_handle;      /**< Handle of the NUS RX characteristic as provided by a discovery. */
} ble_uarts_c_handles_t;

/**@brief Structure containing the NUS event data received from the peer. */
typedef struct
{
    ble_uarts_c_evt_type_t evt_type;
    uint16_t             conn_handle;
    uint16_t             max_data_len;
    uint8_t            * p_data;
    uint16_t             data_len;
    ble_uarts_c_handles_t  handles;     /**< Handles on which the Nordic Uart service characteristics was discovered on the peer device. This will be filled if the evt_type is @ref BLE_NUS_C_EVT_DISCOVERY_COMPLETE.*/
} ble_uarts_c_evt_t;

// Forward declaration of the ble_nus_t type.
typedef struct ble_uarts_c_s ble_uarts_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module to receive events.
 */
typedef void (* ble_uarts_c_evt_handler_t)(ble_uarts_c_t * p_ble_uarts_c, ble_uarts_c_evt_t const * p_evt);

//����͸���ͻ��˽ṹ��
struct ble_uarts_c_s
{
    uint8_t                   uuid_type;      //UUID ����
    uint16_t                  conn_handle;    //���Ӿ��
    ble_uarts_c_handles_t     handles;        //��֮������Ҫ�������ݵĶԶ��豸�ľ��
    ble_uarts_c_evt_handler_t evt_handler;    //����͸���¼�������������봮��͸����ص��¼�ʱ����
    ble_srv_error_handler_t   error_handler;  //����͸�������¼����������������ʱ����
    nrf_ble_gq_t            * p_gatt_queue;   //ָ��BLE GATT Queueʵ��
};

//����͸���ͻ��˳�ʼ���ṹ��
typedef struct
{
    ble_uarts_c_evt_handler_t evt_handler;    //����͸���ͻ����¼�������
	  ble_srv_error_handler_t   error_handler;  //��������
    nrf_ble_gq_t            * p_gatt_queue;   //ָ��GATT Queueʵ��
} ble_uarts_c_init_t;


/**@brief     Function for initializing the Nordic UART client module.
 *
 * @details   This function registers with the Database Discovery module
 *            for the NUS. Doing so will make the Database Discovery
 *            module look for the presence of a NUS instance at the peer when a
 *            discovery is started.
 *
 * @param[in] p_ble_nus_c      Pointer to the NUS client structure.
 * @param[in] p_ble_nus_c_init Pointer to the NUS initialization structure containing the
 *                             initialization information.
 *
 * @retval    NRF_SUCCESS If the module was initialized successfully. Otherwise, an error
 *                        code is returned. This function
 *                        propagates the error code returned by the Database Discovery module API
 *                        @ref ble_db_discovery_evt_register.
 */
uint32_t ble_uarts_c_init(ble_uarts_c_t * p_ble_uarts_c, ble_uarts_c_init_t * p_ble_uarts_c_init);


/**@brief Function for handling events from the database discovery module.
 *
 * @details This function will handle an event from the database discovery module, and determine
 *          if it relates to the discovery of NUS at the peer. If so, it will
 *          call the application's event handler indicating that NUS has been
 *          discovered at the peer. It also populates the event with the service related
 *          information before providing it to the application.
 *
 * @param[in] p_ble_nus_c Pointer to the NUS client structure.
 * @param[in] p_evt       Pointer to the event received from the database discovery module.
 */
 void ble_uarts_c_on_db_disc_evt(ble_uarts_c_t * p_ble_uarts_c, ble_db_discovery_evt_t * p_evt);


/**@brief     Function for handling BLE events from the SoftDevice.
 *
 * @details   This function handles the BLE events received from the SoftDevice. If a BLE
 *            event is relevant to the NUS module, it is used to update
 *            internal variables and, if necessary, send events to the application.
 *
 * @param[in] p_ble_evt     Pointer to the BLE event.
 * @param[in] p_context     Pointer to the NUS client structure.
 */
void ble_uarts_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief   Function for requesting the peer to start sending notification of TX characteristic.
 *
 * @details This function enables notifications of the NUS TX characteristic at the peer
 *          by writing to the CCCD of the NUS TX characteristic.
 *
 * @param   p_ble_nus_c Pointer to the NUS client structure.
 *
 * @retval  NRF_SUCCESS If the SoftDevice has been requested to write to the CCCD of the peer.
 *                      Otherwise, an error code is returned. This function propagates the error
 *                      code returned by the SoftDevice API @ref sd_ble_gattc_write.
 */
uint32_t ble_uarts_c_tx_notif_enable(ble_uarts_c_t * p_ble_uarts_c);


/**@brief Function for sending a string to the server.
 *
 * @details This function writes the RX characteristic of the server.
 *
 * @param[in] p_ble_nus_c Pointer to the NUS client structure.
 * @param[in] p_string    String to be sent.
 * @param[in] length      Length of the string.
 *
 * @retval NRF_SUCCESS If the string was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_uarts_c_string_send(ble_uarts_c_t * p_ble_uarts_c, uint8_t * p_string, uint16_t length);


/**@brief Function for assigning handles to a this instance of nus_c.
 *
 * @details Call this function when a link has been established with a peer to
 *          associate this link to this instance of the module. This makes it
 *          possible to handle several link and associate each link to a particular
 *          instance of this module. The connection handle and attribute handles will be
 *          provided from the discovery event @ref BLE_NUS_C_EVT_DISCOVERY_COMPLETE.
 *
 * @param[in] p_ble_nus_c    Pointer to the NUS client structure instance to associate with these
 *                           handles.
 * @param[in] conn_handle    Connection handle to associated with the given NUS Instance.
 * @param[in] p_peer_handles Attribute handles on the NUS server that you want this NUS client to
 *                           interact with.
 *
 * @retval    NRF_SUCCESS    If the operation was successful.
 * @retval    NRF_ERROR_NULL If a p_nus was a NULL pointer.
 */
uint32_t ble_uarts_c_handles_assign(ble_uarts_c_t *               p_ble_uarts_c,
                                    uint16_t                      conn_handle,
                                    ble_uarts_c_handles_t const * p_peer_handles);


#ifdef __cplusplus
}
#endif


#endif // BLE_MY_UARTS_C_H__


