/**
 * @file tuya_ipc_sdk_init.h
 * @brief This is tuya ipc sdk init file
 * @version 1.0
 * @date 2021-11-17
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_IPC_SDK_INIT_H__
#define __TUYA_IPC_SDK_INIT_H__

#include <stdio.h>
#include <time.h>
#include "tuya_iot_config.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_cloud_com_defs.h"
#include "cJSON.h"
#include "tuya_ipc_upgrade.h"
#if defined(ENABLE_IPC_GW_BOTH) && (ENABLE_IPC_GW_BOTH==1)
#include "tuya_gw_init_api.h"
#endif
#include "tal_time_service.h"

#ifdef __cplusplus
extern "C" {
#endif


#define IPC_STORAGE_PATH_LEN 64
#define IPC_PRODUCT_KEY_LEN 16
#define IPC_UUID_LEN 25
#define IPC_AUTH_KEY_LEN 32
#define IPC_SW_VER_LEN 20
#define IPC_SERIAL_NUM_LEN 32
#define IPC_P2P_ID_LEN 25
#define IPC_FIRMWARE_KEY_LEN 16
#define IPC_DB_KEY_LEN 16
#define IPC_SECURITY_VER_LEN 16

typedef enum {
    PARING_MODE_WIFI_AP = 1,
    PARING_MODE_DEBUG,
    PARING_MODE_WIRED,   
    PARING_MODE_CONCURRENT
} TUYA_IPC_PARING_MODE_E;

typedef enum {
    NORMAL_POWER_DEV =0,
    LOW_POWER_DEV,
    DEV_TYPE_MAX
} TUYA_IPC_DEV_TYPE_E;

typedef  UINT_T TUYA_IPC_LINK_TYPE_E;
#define TUYA_IPC_LINK_WIFI 1
#define TUYA_IPC_LINK_WIRE 2
#define TUYA_IPC_LINK_BT   4

/* tuya sdk definition of ip-mode type */
typedef BYTE_T TUYA_IPC_IP_MODE_TYPE;       //rename from IP_MODE_TYPE
#define TUYA_IPC_IPV4_ONLY  0
#define TUYA_IPC_IPV4_FIRST 1
#define TUYA_IPC_IPV6_ONLY  2
#define TUYA_IPC_IPV6_FIRST 3

typedef BYTE_T TUYA_IPC_MOBILE_CARD_TYPE;
#define TUYA_IPC_MOBILE_PHYSICAL_CARD 1
#define TUYA_IPC_MOBILE_VIRTUAL_CARD 2
#define TUYA_IPC_MOBILE_PHYSICAL_AND_VIRTUAL_CARD (TUYA_IPC_MOBILE_PHYSICAL_CARD | TUYA_IPC_MOBILE_VIRTUAL_CARD)

typedef BYTE_T TUYA_IPC_MOBILE_CARD_VENDOR_CODE;

typedef struct {
    TUYA_IPC_MOBILE_CARD_TYPE card_type;                ///< 4G mobile network card type
    TUYA_IPC_MOBILE_CARD_VENDOR_CODE vendor_code;       ///< 4G mobile network card vendor code
} TUYA_IPC_MOBILE_NETWORK_CARD_T;

typedef enum {
	TUYA_IPC_DIRECT_CONNECT_QRCODE = 0, ///< direct connect mode, compared to wifi EZ mode, get ssid/psw via QR etc.
	TUYA_IPC_DIRECT_CONNECT_MAX,
} TUYA_IPC_DIRECT_CONNECT_TYPE_E;

typedef enum {
    TUYA_IPC_STATUS_GROUP_DEVICE = 1,
    TUYA_IPC_STATUS_GROUP_WIFI,
    TUYA_IPC_STATUS_GROUP_WIRE,
    TUYA_IPC_STATUS_GROUP_CLOUD,
    
    TUYA_IPC_STATUS_GROUP_MAX,
} TUYA_IPC_STATUS_GROUP_E;

typedef enum {
    //* TUYA_IPC_STATUS_GROUP_DEVICE */
    TUYA_IPC_STATUS_REGISTER = 1,
    TUYA_IPC_STATUS_REGISTER_FAILED,
    TUYA_IPC_STATUS_ACTIVATE_STAGE_FAIL_URL,
    TUYA_IPC_STATUS_ACTIVATE_STAGE_FAIL_ACTIVE,
    TUYA_IPC_STATUS_ACTIVATE_STAGE_FAIL_RESULT,
    TUYA_IPC_STATUS_ACTIVATE_STAGE_FAIL_UNKNOWN,
    TUYA_IPC_STATUS_ACTIVED,
    TUYA_IPC_STATUS_RESET,
    TUYA_IPC_STATUS_NETCFG_START,
    TUYA_IPC_STATUS_NETCFG_STOP,
    TUYA_IPC_STATUS_UPGRADE_START,
    
    //* TUYA_IPC_STATUS_GROUP_WIFI */
    TUYA_IPC_STATUS_WIFI_STA_UNCONN = 20,
    TUYA_IPC_STATUS_WIFI_STA_CONN,
    TUYA_IPC_STATUS_WIFI_STA_PRE_CONN,
    TUYA_IPC_STATUS_WIFI_STA_AP_NOT_FOUND,
    TUYA_IPC_STATUS_WIFI_STA_AP_CANT_CONN_AP,
    TUYA_IPC_STATUS_WIFI_STA_AP_ERR_PASSWD,

    //* TUYA_IPC_STATUS_GROUP_WIRE */
    TUYA_IPC_STATUS_WIRE_UNCONN = 30,
    TUYA_IPC_STATUS_WIRE_CONN,
    
    //* TUYA_IPC_STATUS_GROUP_CLOUD */
    TUYA_IPC_STATUS_ONLINE = 40,
    TUYA_IPC_STATUS_OFFLINE,
} TUYA_IPC_STATUS_E;

typedef VOID (*TUYA_RESTART_INFORM_CB)(VOID);
typedef VOID (*TUYA_RST_INFORM_CB)(GW_RESET_TYPE_E from);
typedef VOID(*TUYA_QRCODE_ACTIV_CB)(CHAR_T * shorturl);

/**
 * @brief callback of SDK status change
 * 
 * @param[in] changed_group: which status group changed
 * 
 * @param[in] status: use param changed_group to index current changed status, other group's status can also find in status array
 * 
 * @return VOID
 */
typedef VOID (*TUYA_IPC_STATUS_CHANGED_CB) (IN TUYA_IPC_STATUS_GROUP_E changed_group, IN CONST TUYA_IPC_STATUS_E status[TUYA_IPC_STATUS_GROUP_MAX]);


typedef struct {
    CHAR_T storage_path[IPC_STORAGE_PATH_LEN + 1];      ///< path to store TUYA database files, readable and writable
    CHAR_T online_log_path[IPC_STORAGE_PATH_LEN + 1];   ///< path to store TUYA online log, readable and writable
    
    CHAR_T product_key[IPC_PRODUCT_KEY_LEN + 1];    ///< one product key(PID) for one product
    CHAR_T uuid[IPC_UUID_LEN + 1];          ///< one uuid-authkey pair for one device, to register on TUYA cloud
    CHAR_T auth_key[IPC_AUTH_KEY_LEN + 1];
    CHAR_T p2p_id[IPC_P2P_ID_LEN + 1];      ///< this param is not used now
    CHAR_T firmware_key[IPC_FIRMWARE_KEY_LEN + 1];

    CHAR_T dev_sw_version[IPC_SW_VER_LEN + 1];      ///< software version with format x1.x2.x3.x4.x5.x6, five dots maximum, zero dots minimum
    CHAR_T dev_serial_num[IPC_SERIAL_NUM_LEN + 1];  ///< this param is not used in IPC now

    
    DEV_OBJ_DP_CMD_CB dev_obj_dp_cb;    ///< dp cmd callback function, which hanles cmd from TUYA cloud and APP
    DEV_RAW_DP_CMD_CB dev_raw_dp_cb;    ///< raw dp cmd callback function, which hanles cmd from TUYA cloud and APP
    DEV_DP_QUERY_CB dev_dp_query_cb;    ///< dp query callback function

    TUYA_IPC_STATUS_CHANGED_CB status_changed_cb;
    TUYA_IPC_SDK_UPGRADE_T  upgrade_cb_info;    ///< OTA callback function, triggered by uprading from APP and TUYA cloud
    TUYA_RST_INFORM_CB gw_rst_cb;   ///< reset callback fucntion, triggered when user unbind device from a account
    TUYA_RESTART_INFORM_CB gw_restart_cb;   ///< restart callback function
    TUYA_QRCODE_ACTIV_CB qrcode_active_cb;  ///<qrcode active get short  url callback
    BOOL_T mem_save_mode;   ///< woking in low memory mode
    TUYA_IPC_DEV_TYPE_E dev_type;   ///< device type ,eg low power device
    TUYA_IPC_LINK_TYPE_E link_type; ///< link type, bitmap, eg TUYA_IPC_LINK_WIFI,TUYA_IPC_LINK_WIRE,TUYA_IPC_LINK_BT
    TUYA_IPC_IP_MODE_TYPE ip_mode_type; ///< ip mode type, for ipv4 & ipv6 set
    TUYA_IPC_MOBILE_NETWORK_CARD_T mobile_network_card; ///< 4G mobile network card
    BOOL_T lan_only_mode; ///< working in lan only mode
    CHAR_T db_key[IPC_DB_KEY_LEN + 1]; ///< database encrypt key
    UINT_T db_backup_interval; ///<backup database interval, unit in seconds. If set to 0, it defaults to 120 seconds
    CHAR_T security_version[IPC_SECURITY_VER_LEN + 1]; ///<set security version
    VOID *p_reserved;
} TUYA_IPC_ENV_VAR_T;

typedef OPERATE_RET (*TUYA_IPC_SDK_MQTT_SET_BOOK_TOPIC_CB)(IN CHAR_T *topic);
typedef OPERATE_RET (*TUYA_IPC_SDK_MQTT_MSG_SEND_CB)(IN CONST CHAR_T *topic, IN INT_T protocol, IN CONST CHAR_T *data, IN CONST UINT_T len);
typedef OPERATE_RET (*TUYA_IPC_SDK_CFG_GET_CB)(IN CHAR_T *key, OUT CHAR_T *value, IN INT_T value_len);
typedef OPERATE_RET (*TUYA_IPC_SDK_CFG_SET_CB)(IN CHAR_T *key, IN CHAR_T *value, IN INT_T value_len);
typedef struct {
    TUYA_IPC_SDK_MQTT_SET_BOOK_TOPIC_CB set_book_topic_cb;
    TUYA_IPC_SDK_MQTT_MSG_SEND_CB mqtt_msg_send_cb;
    TUYA_IPC_SDK_CFG_GET_CB cfg_get_cb;
    TUYA_IPC_SDK_CFG_SET_CB cfg_set_cb;
}TUYA_IPC_EXT_HANDLE_VAR_T;

typedef struct {
    CHAR_T storage_path[IPC_STORAGE_PATH_LEN + 1];/* path to store TUYA database files, readable and writable */
}TUYA_XVR_ENV_CTX_T;

#if defined(ENABLE_WIFI_FFS) && (ENABLE_WIFI_FFS == 1)
typedef struct {
   CHAR_T *ffsPid;      ///< ffs amazon ubikey
   UINT_T ffsPidLen;    ///< ffs pid length
   CHAR_T *ffsPin;      ///< ffs pin
   UINT_T ffsPinLen;    ///< ffs pin length
   CHAR_T *ffsDpss;     ///< ffs public key
   UINT_T ffsDpssLen;   ///< ffs public key length
   CHAR_T *ffsDsn;      ///< ffs dsn
   UINT_T ffsDsnLen;    ///< ffs device serial no lenth
   CHAR_T *ffsPrivateKey;   ///< ffs  private key
   UINT_T ffsPrivateKeyLen; ///< ffs private key length
   CHAR_T *ffsCertificate;  ///< ffs certificate
   UINT_T ffsCertificateLen; ///< ffs certificate length
} TUYA_IPC_FFS_INFO_T;
#endif


/**
 * @brief initialize tuya sdk for embeded devices
 * 
 * @param[in] p_var: TUYA_IPC_ENV_VAR_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_init_sdk(IN CONST TUYA_IPC_ENV_VAR_T *p_var);

/**
 * @brief start ipc as WIFI_DEVICE
 * 
 * @param[in] paring_mode: TUYA_IPC_PARING_MODE_E
 * @param[in] p_token: QR token
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_sdk(IN CONST TUYA_IPC_PARING_MODE_E paring_mode, IN CONST CHAR_T *p_token);

#if defined(ENABLE_IPC_GW_BOTH) && (ENABLE_IPC_GW_BOTH==1)
OPERATE_RET tuya_ipc_gw_set_gw_cbs(TY_IOT_GW_CBS_S *p_gw_cbs);

OPERATE_RET tuya_ipc_gw_start_sdk(IN CONST TUYA_IPC_PARING_MODE_E paring_mode,IN CONST char *pToken);
#endif

#if defined(ENABLE_WIFI_FFS) && (ENABLE_WIFI_FFS == 1)
/**
 * @brief write ffs certificate
 * 
 * @param[in] ffs_info: ffs certificate info
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ffs_info_write(IN TUYA_IPC_FFS_INFO_T *ffs_info);
#endif

/**
 * @brief register external handle (mqtt, cfg) for sdk
 *
 * @param[in] p_ext_var: TUYA_IPC_EXT_HANDLE_VAR_T
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_register_ext_handle(IN CONST TUYA_IPC_EXT_HANDLE_VAR_T *p_ext_var);

/**
 * @brief enable preson flow detect skill
 * 
 * @param VOID
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_skill_enable_person_flow(VOID);

/**
 * @brief upload device skills to cloud after init success
 * 
 * @return VOID 
 */
VOID tuya_ipc_upload_skills(VOID);

/**
 * @brief cascade skill enable, call before tuya_ipc_upload_skills
 * 
 * @param VOID
 * 
 * @return VOID 
 */
VOID tuya_ipc_cascade_enable(VOID);


/**
 * @brief set SDK dns cache priority
 * 
 * @param[in] priority: 0 use region dns, 1 use random dns, 2 use system dns
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_set_dns_cache_priority(IN CONST INT_T priority);

/**
 * @brief use this API to connect to specific wifi and register to tuya cloud, data can be obtain via QR code e.g.
 * 
 * @param[in] p_str: in format: {"p":"password_of_wifi","s":"ssid_of_wifi","t":"token_to_register"}
 * @param[in] source: only TUYA_IPC_DIRECT_CONNECT_QRCODE is supported now
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_direct_connect(IN CONST CHAR_T *p_str, IN CONST TUYA_IPC_DIRECT_CONNECT_TYPE_E source);
/**
 * @brief use this API for ipc sdk init in meger sdk.
 *
 * @param VOID
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_service_event_subscribe(VOID);
/**
 * @brief use this API for ipc xvr sdk init
 *
 * @param[in] xvr_context_ptr: storage path
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_xvr_sdk_init(TUYA_XVR_ENV_CTX_T * xvr_context_ptr);

/**
 * @brief essential config for ipc sdk if iot function is disabled
 *
 * @param[in] dev_id: device id
 * @param[in] key: key
 * @param[in] local_key: local key
 * @param[in] region: region
 * @param[in] https_self_url: https url
 * @param[in] env
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_set_config(IN CONST CHAR_T *dev_id, IN CONST CHAR_T *key,
                                        IN CONST CHAR_T *local_key, IN CONST CHAR_T *region,
                                        IN CONST CHAR_T *https_self_url, IN CONST CHAR_T *env);

/**
 * @brief set time of tuya SDK
 * @param[in] time_utc: utx time
 * @param[in] time_zone: time zone in seconds
 * @param[in] zone: summer zone
 * @param[in] cnt: element count in summer zone
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_set_time(IN TIME_T time_utc, IN INT_T time_zone, IN SUM_ZONE_S *zone, IN UINT_T cnt);

/**
 * @brief upload iccid to tuya cloud. Should upload when active or card changed
 * 
 * @param iccid 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_ipc_upload_iccid(CHAR_T *iccid);

/**
 * @brief upload imei to tuya cloud. Should upload after active
 * 
 * @param imei 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_ipc_upload_imei(CHAR_T *imei);

/**
 * @brief upload lan ip addr to app. Should upload after active
 * 
 * @param  
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_ipc_upload_lan_ip_addr();

/**
 * @brief check flow restrict  , must called after iot init
 * 
 * @param NULL
 * @return FALSE:flow ok  TRUE:flow is restricted 
 */
BOOL_T tuya_ipc_is_flow_restrict();


#ifdef __cplusplus
}
#endif

#endif

