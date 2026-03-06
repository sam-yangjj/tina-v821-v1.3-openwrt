#ifndef __TAL_BLUETOOTH_H__
#define __TAL_BLUETOOTH_H__

#include "tal_bluetooth_def.h"

/**
 * @brief   Function for initializing the bluetooth
 * @param   [in] role:      set the ble/bt role
 *          [in] p_event:   Event handler provided by the user. 
 * @return  SUCCESS         Initialized successfully.
 *          ERROR
 * */
OPERATE_RET tal_ble_bt_init(TAL_BLE_ROLE_E role, CONST TAL_BLE_EVT_FUNC_CB ble_event);

/**
 * @brief   Function for deinitializing the bluetooth, or specify one role 
 * @param   [in] role:      set the ble/bt role
 * @return  SUCCESS         Deinitialized successfully.
 *          ERROR
 * */
OPERATE_RET tal_ble_bt_deinit(TAL_BLE_ROLE_E role);

/**
 * @brief   Set the local Bluetooth identity address.
 * @param   [in] p_addr:    pointer to local address
 * @return  SUCCESS         Set Address successfully.
 *          ERROR
 * */
OPERATE_RET tal_ble_address_set(TAL_BLE_ADDR_T CONST *p_addr);

/**
 * @brief   Get the local Bluetooth identity address.
 * @param   [out] p_addr: pointer to local address
 * @return  SUCCESS         Set Address successfully.
 *          ERROR
 * */
OPERATE_RET tal_ble_address_get(TAL_BLE_ADDR_T *p_addr);

/**
 * @brief   Get Max Link Size
 * @param   [out] max_mtu:  set mtu size.
 * @return  SUCCESS
 *          ERROR
 * */
OPERATE_RET tal_ble_bt_link_max(USHORT_T *p_maxlink);

/**
 * @brief   Start advertising
 * @param  [in] p_adv_param : pointer to advertising parameters, see TAL_BLE_ADV_PARAMS_T for details
 * @return  SUCCESS             Successfully started advertising procedure.
 *          ERR_INVALID_STATE   Not in advertising state.
 * */
OPERATE_RET tal_ble_advertising_start(TAL_BLE_ADV_PARAMS_T CONST *p_adv_param);

/**
 * @brief   Setting advertising data
 * @param   [in] p_adv : Data to be used in advertisement packets
 *          [in] p_rsp : Data to be used in scan response packets.
 * @return  SUCCESS             Successfully set advertising data.
 *          ERR_INVALID_param   Invalid parameter(s) supplied.
 * */
OPERATE_RET  tal_ble_advertising_data_set(TAL_BLE_DATA_T *p_adv, TAL_BLE_DATA_T *p_scan_rsp);

/**
 * @brief   Stop advertising
 * @param   VOID
 * @return  SUCCESS             Successfully stopped advertising procedure.
 *          ERR_INVALID_STATE   Not in advertising state.
 * */
OPERATE_RET tal_ble_advertising_stop(VOID);

/**
 * @brief   Updating advertising data
 * @param   [in] p_adv :        Data to be used in advertisement packets
 *          [in] p_rsp :        Data to be used in scan response packets.
 * @return  SUCCESS             Successfully update advertising data.
 *          ERR_INVALID_STATE   Invalid parameter(s) supplied.
 * @Note:   If being in advertising, we will do update adv+rsp data.
 *          If not being in advertising, we will only do set adv+rsp data.
 * */
OPERATE_RET tal_ble_advertising_data_update(TAL_BLE_DATA_T *p_adv, TAL_BLE_DATA_T *p_scan_rsp);

/**
 * @brief   Start scanning
 * @param :
 *          [in] scan_param:    scan parameters including interval, windows
 * and timeout
 * @return  SUCCESS             Successfully initiated scanning procedure.
 *
 * @note    Other default scanning parameters : public address, no
 * whitelist.
 *          The scan response and adv data are given through TAL_BLE_EVT_ADV_REPORT event !!!!!!
 */
OPERATE_RET tal_ble_scan_start(TAL_BLE_SCAN_PARAMS_T CONST *p_scan_param);

/**
 * @brief   Stop scanning
 * @param   VOID
 * @return  SUCCESS         Successfully stopped scanning procedure.
 *          ERROR           Not in scanning state.
 * */
OPERATE_RET tal_ble_scan_stop(VOID);

/**
 * @brief   Get the received signal strength for the last connection event.
 * @param   [in]peer:       conn_handle Connection handle.
 * @return  SUCCESS         Successfully read the RSSI.
 *          ERROR           No sample is available.
 *          The rssi reading is given through TAL_BLE_EVT_LINK_RSSI event !!!!!!
 * */
OPERATE_RET tal_ble_rssi_get(CONST TAL_BLE_PEER_INFO_T peer);

/**
 * @brief   Set the radio's transmit power.
 * @param   [in] role: 0: Advertising Tx Power; 1: Scan Tx Power; 2: Connection Power
 *          [in] tx_power:      tx power:This value will be magnified 10 times. 
 *                              If the tx_power value is -75, the real power is -7.5dB.(or 40 = 4dB)
 * @return  SUCCESS
 *          ERROR
 * */ 
OPERATE_RET tal_ble_tx_power_set(UCHAR_T role, INT_T tx_power);

/**
 * @brief   Update connection parameters.
 * @details In the central role this will initiate a Link Layer connection parameter update procedure,
 *          otherwise in the peripheral role, this will send the corresponding L2CAP request and wait for
 *          the central to perform the procedure..
 * @param :
 *          [in] conn_param: connect parameters including interval, peer info 
 * @return  SUCCESS         Successfully initiated connecting procedure.
 *
 *          The connection parameters updating is given through TAL_BLE_EVT_CONN_PARAM_UPDATE event !!!!!!
 */
OPERATE_RET tal_ble_conn_param_update(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_CONN_PARAMS_T CONST *p_conn_params);

/**
 * @brief   Start connecting one peer
 * @param : [in] peer info.     include address and address type
 *          [in] conn_param:    connect parameters including interval, peer info \
 * @return  SUCCESS             Successfully initiated connecting procedure.
 *
 *          The connection event is given through TAL_BLE_EVT_CENTRAL_CONNECT event !!!!!!
 */
OPERATE_RET tal_ble_connect_and_discovery(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_CONN_PARAMS_T CONST *p_conn_params);

/**
 * @brief   Disconnect from peer
 * @param   [in] conn_handle: the connection handle
 * @return  SUCCESS         Successfully disconnected.
 *          ERROR STATE     Not in connnection.
 *          ERROR PARAMETER
 * @note    Both of ble central and ble peripheral can call this function. 
 *              For ble peripheral, we only need to fill connect handle.
*               For ble central, we need to fill connect handle and peer addr.
 * */
OPERATE_RET tal_ble_disconnect(CONST TAL_BLE_PEER_INFO_T peer);

/**
 * @brief   [Ble Peripheral] Notify Command characteristic value
 * @param   [in] p_data: pointer to data, refer to TAL_BLE_DATA_T
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 * */
OPERATE_RET tal_ble_server_common_send(TAL_BLE_DATA_T *p_data);

/**
 * @brief   [Ble Peripheral] Set Read Command characteristic value
 * @param   [in] p_data: pointer to data, refer to TAL_BLE_DATA_T
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 * */
OPERATE_RET tal_ble_server_common_read_update(TAL_BLE_DATA_T *p_data);

/**
 * @brief   [Ble Central] write data to peer within command channel
 * @param   [in] p_data: pointer to data, refer to TAL_BLE_DATA_T
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 * */
OPERATE_RET tal_ble_client_common_send(CONST TAL_BLE_PEER_INFO_T peer, TAL_BLE_DATA_T *p_data);

/**
 * @brief   [Ble Central] read data from peer within command channel
 * @param   [in] p_data: pointer to data, refer to TAL_BLE_DATA_T
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 * */
OPERATE_RET tal_ble_client_common_read(CONST TAL_BLE_PEER_INFO_T peer);

/**
 * @brief   [Ble Peripheral] Reply to an ATT_MTU exchange request by sending an Exchange MTU Response to the client.
 * @param   [in] att_mtu: mtu size
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 *          The MTU negotiation is given through TAL_BLE_EVT_MTU_REQUEST event !!!!!!
 * */
OPERATE_RET tal_ble_server_exchange_mtu_reply(CONST TAL_BLE_PEER_INFO_T peer, USHORT_T server_mtu);

/**
 * @brief   [Ble Central] Start an ATT_MTU exchange by sending an Exchange MTU Request to the server.
 * @param   [in] att_mtu: mtu size
 * @return  SUCCESS
 *          ERROR Refer to platform error code
 *          ERROR_INVALID_CONN_HANDLE Invalid connection handle.
 *          ERROR_INVALID_STATE Invalid connection state or an ATT_MTU exchange was already requested once.
 * */
OPERATE_RET tal_ble_client_exchange_mtu_request(CONST TAL_BLE_PEER_INFO_T peer, USHORT_T client_mtu);

/**
 * @brief   [Special Command Control] Base on Bluetooth, We can do some special commands for exchanging some informations.
 * @param   [in] opcode         Operations Opcode.
 *          [in] user_data      Post Some Special Commands Data.
 *          [in] data_len       User's Data Length.
 * @note    For Operations Codes, we can do anythings after exchange from TAL Application
 *          And We define some Opcodes as below for reference.
 *          For Bluetooth NCP Module: Mask=0x01, Code ID: 0x00~0xff. Opcode = ((0x01 << 8) & Code ID)
 *          eg:     0x0100: Special Vendor Module Init
 *                  0x0101: Special Vendor Module Deinit
 *                  0x0102: Special Vendor Module Reset
 *                  0x0103: Special Vendor Module Check Exist: Return OPRT_OK or OPRT_NOT_FOUND ..
 *                  0x0104: Specail Vendor Module Version Get.
 *                  0x0105: Specail Vendor Module Version Set.
 *                  0x0106: Specail Vendor Module Version Update.
 *                  0x0107: Specail Vendor Module Scan Switch.
 *                  0x0108: Specail Vendor Module Scan Stop.
 *                  0x0109: Specail Vendor Module Auth Check.
 *                  0x010A: Specail Vendor Module Auth Erase.
 *
 *  * @return  SUCCESS
 *          ERROR
 * */
OPERATE_RET tal_ble_vendor_command_control(USHORT_T opcode, VOID_T *user_data, USHORT_T data_len);

/**
 * @brief   [Tuya Internal] Base on Bluetooth Gateway, We define this function for reporting bluetooth log string.
 * @param   [in] TAL_BLE_LOG_SEQ_FUNC   the function of log-seq interface.
 *          [out] TAL_BLE_PUBLISH_CB    the function of publish callback.
 * 
 *  * @return  SUCCESS
 *          ERROR
 * */
OPERATE_RET tal_ble_cloud_func_register(TAL_BLE_LOG_SEQ_FUNC log_seq_func, TAL_BLE_PUBLISH_CB *publish_func);
#endif

