/**
* @file     tuya_iot_adapter.h
* @brief    iot adapter api for low power ipc
* @version  1.0
* @date     2022-01-10
* 
* copyright Copyright (c) tuya.inc 2022
*/
#ifndef __TUYA_IOT_ADAPTER_H__
#define __TUYA_IOT_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"
#include "mqc_app.h"

typedef OPERATE_RET (*IOT_MQTT_MSG_SEND_CB)(IN CONST CHAR_T *topic, IN INT_T protocol, IN CONST CHAR_T *data, IN CONST UINT_T len);

typedef OPERATE_RET (*IOT_MQTT_BOOK_ADDITIONAL_TOPIC_CB)(IN CHAR_T *topic);

typedef OPERATE_RET (*IOT_CFG_SET_CB)(IN CHAR_T *key, IN CHAR_T *value, IN INT_T valueLen);

typedef OPERATE_RET (*IOT_CFG_GET_CB)(IN CHAR_T *key, OUT CHAR_T *value, IN INT_T valueLen);

typedef struct {
    IOT_MQTT_MSG_SEND_CB mqtt_msg_send_cb;
    IOT_MQTT_BOOK_ADDITIONAL_TOPIC_CB book_additional_topic_cb;
    DEV_OBJ_DP_CMD_CB dev_obj_dp_cb;
}TUYA_IOT_MQTT_REG_T;

typedef struct {
    IOT_CFG_GET_CB cfg_set_cb;
    IOT_CFG_GET_CB cfg_get_cb;
}TUYA_IOT_CFG_REG_T;

typedef struct {
    TUYA_IOT_MQTT_REG_T mqtt;
    TUYA_IOT_CFG_REG_T cfg;
}TUYA_IOT_ADAPTER_REG_T;

OPERATE_RET tuya_iot_adapter_register(IN CONST TUYA_IOT_ADAPTER_REG_T *iot_adapter);

VOID tuya_iot_adapter_unregister(VOID);

OPERATE_RET tuya_iot_adapter_init_params(IN CONST CHAR_T *fs_storge_path, IN CONST TY_INIT_PARAMS_S *p_param, IN CONST BOOL_T iot_enable);

OPERATE_RET tuya_iot_adapter_set_config(IN CONST CHAR_T *devId, IN CONST CHAR_T *key,
                                                    IN CONST CHAR_T *local_key, IN CONST CHAR_T *region,
                                                    IN CONST CHAR_T *httpsSelfUrl, IN CONST CHAR_T *env);

OPERATE_RET tuya_iot_adapter_set_time(IN TIME_T time_utc, IN INT_T time_zone, IN SUM_ZONE_S* zone, IN UINT_T cnt);

OPERATE_RET tuya_iot_adapter_mqc_app_register_cb(UINT_T mq_pro, iot_mqc_protocol_handler_cb handler);

OPERATE_RET tuya_iot_adapter_mqt_msg_parse(IN INT_T protocol, IN CONST CHAR_T *data, IN CONST UINT_T len);

OPERATE_RET tuya_iot_adapter_report_dp_json_async(IN CONST CHAR_T *dev_id, IN CONST TY_OBJ_DP_S *dp_data, IN CONST UINT_T cnt);
OPERATE_RET tuya_iot_adapter_report_dp_json_async_force(IN CONST CHAR_T *dev_id, IN CONST TY_OBJ_DP_S *dp_data, IN CONST UINT_T cnt);

#define tuya_iot_adapter_report_dp_raw_sync(dev_id, dpid, data, len, timeout) \
    tuya_iot_adapter_report_dp_raw_sync_extend(dev_id, dpid, data, len, timeout, TRUE)
OPERATE_RET tuya_iot_adapter_report_dp_raw_sync_extend(IN CONST CHAR_T *dev_id, IN CONST BYTE_T dpid,\
                                                                        IN CONST BYTE_T *data, IN CONST UINT_T len,\
                                                                        IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans);

#define tuya_iot_adapter_report_dp_raw_sync_with_time(dev_id, dpid, data, len, timeout, time_str) \
    tuya_iot_adapter_report_dp_raw_sync_extend_with_time(dev_id, dpid, data, len, timeout, TRUE, time_str)
OPERATE_RET tuya_iot_adapter_report_dp_raw_sync_extend_with_time(IN CONST CHAR_T *dev_id, IN CONST BYTE_T dpid,\
                                                                        IN CONST BYTE_T *data, IN CONST UINT_T len,\
                                                                        IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans,\
                                                                        IN CONST CHAR_T *time_str);

#define tuya_iot_adapter_report_dp_stat_sync(dev_id, dp_data, cnt, timeout) \
    tuya_iot_adapter_report_dp_stat_sync_extend(dev_id, dp_data, cnt, timeout, TRUE)
OPERATE_RET tuya_iot_adapter_report_dp_stat_sync_extend(IN CONST CHAR_T *dev_id, IN CONST TY_OBJ_DP_S *dp_data,\
                                                       IN CONST UINT_T cnt, IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans);

OPERATE_RET tuya_iot_adapter_book_additional_topic(IN CHAR_T *topic);

OPERATE_RET tuya_iot_adapter_mqc_prot_data_rept_seq(IN CONST UINT_T pro, IN CONST CHAR_T *data, IN CONST BYTE_T qos,\
                                                            IN CONST UINT_T to_lmt, IN CONST CB_MQTT_PUB_INFORM cb, IN VOID *prv_data);

INT_T tuya_iot_adapter_get_mqtt_socket_fd(VOID);

OPERATE_RET tuya_iot_adapter_send_custom_mqtt_msg(IN CONST UINT_T protocol, IN CONST BYTE_T *p_data);

OPERATE_RET tuya_iot_adapter_send_mqtt_msg_with_topic(IN CONST CHAR_T *topic, IN CONST UINT_T protocol, IN CONST BYTE_T *p_data);

BOOL_T tuya_iot_adapter_get_mqc_conn_stat(VOID);

#define tuya_iot_adapter_upgrade_gw(fw, get_file_cb, upgrd_nofity_cb, pri_data) \
    tuya_iot_adapter_upgrade_gw_notify(fw, get_file_cb, upgrd_nofity_cb, pri_data, TRUE, 0)
OPERATE_RET tuya_iot_adapter_upgrade_gw_notify(IN CONST FW_UG_S *fw,
                                       IN CONST GET_FILE_DATA_CB get_file_cb,\
                                       IN CONST UPGRADE_NOTIFY_CB upgrd_nofity_cb,\
                                       IN CONST PVOID_T pri_data,\
                                       BOOL_T notify, UINT_T download_buf_size);

OPERATE_RET tuya_iot_adapter_upgd_progress_rept(IN CONST UINT_T percent, IN CONST CHAR_T *devid, IN CONST DEV_TYPE_T tp);

#ifdef __cplusplus
}
#endif

#endif
