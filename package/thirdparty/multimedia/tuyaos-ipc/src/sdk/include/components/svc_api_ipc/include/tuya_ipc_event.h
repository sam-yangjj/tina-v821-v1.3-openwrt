/**
 * @file tuya_ipc_event.h
 * @brief This is tuya ipc event file
 * @version 1.0
 * @date 2021-11-17
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include "tuya_ipc_api.h"
#include "tuya_ipc_cloud_storage.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_TEXT_LEN 32

#define RESOURCE_PIC    1
#define RESOURCE_TEXT   2
#define RESOURCE_VIDEO  4
#define RESOURCE_AUDIO  8

#define AES_KEY_LEN 16

#define ALARM_STORAGE_BASE    (0x1)

#define MAX_ALARM_BITMAP_NUM 32
#define MAX_ALARM_TYPE_NUM (MAX_ALARM_BITMAP_NUM << 5)


typedef enum {
    E_ALARM_MOTION        = 0, //移动侦测
    E_ALARM_DOORBELL      = 1, //门铃
    E_ALARM_DEV_LINK      = 2, //设备联动
    E_ALARM_PASSBY        = 3, //有人经过
    E_ALARM_LINGER        = 4, //有人徘徊
    E_ALARM_MSG           = 5, //门铃消息留言
    E_ALARM_CONNECTED     = 6, //门铃已接听
    E_ALARM_UNCONNECTED   = 7, //门铃未接听
    E_ALARM_REFUSE        = 8, //门铃拒接
    E_ALARM_HUMAN         = 9, //人形检测
    E_ALARM_CAT           = 10, //宠物检测
    E_ALARM_CAR           = 11, //车辆检测
    E_ALARM_BABY_CRY      = 12, //婴儿哭声
    E_ALARM_BANG          = 13, //异响
    E_ALARM_FACE          = 14, //检测到人脸
    E_ALARM_ANTIBREAK     = 15, //强拆报警
    E_ALARM_RESERVE       = 16, //自定义消息
    E_ALARM_IO            = 17, //摄像头接入io设备告警
    E_ALARM_LOW_BATTERY   = 18, //低电量告警
    E_ALARM_DOORBELL_PRESS = 19, //门铃呼叫转154
    E_ALARM_COVER         = 20, //封面更新
    E_ALARM_FULL_BATTERY  = 21, //电量已满
    E_ALARM_IO_2          = 22, //io告警2
    E_ALARM_DEV_LINK_2    = 23, //设备联动2
    E_ALARM_NO_HELMET     = 24, //未戴安全帽
    E_ALARM_NO_SUIT       = 25, //未穿工服
    E_ALARM_ENTER_AREA    = 26, //区域入侵
    E_ALARM_SLEEP         = 27, //睡岗检测
    E_ALARM_CHN_BLOCK     = 28, //通道阻塞
    E_ALARM_CALLING       = 29, //打电话检测
    E_ALARM_SMOKE         = 30, //吸烟检测
    E_ALARM_LOCAL_FACE    = 31, //本地人脸检测
    E_ALARM_FIRE          = 32, //检测到火警
    E_ALARM_AI            = 33, //检测到AI事件
    E_ALARM_MOTION_CHN    = 34, //移动侦测通道上报
    E_ALARM_PERSON_PORTRAIT = 35, //店内客流（客群画像）
    E_ALARM_PERSON_FLOW   = 36, //出入口客流
    E_ALARM_HOTSPOTS      = 37, //店内热区
    E_ALARM_CAPTURE       = 38, //图片抓拍
    E_ALARM_BEHAVIOR      = 39, //行为分析
    E_ALARM_NON_MOTO      = 40, //非机动车检测
    E_ALARM_DOORBELl_2    = 41, //门铃呼叫（多个门铃设备）
    E_ALARM_PACKAGE       = 42, //包裹检测
    E_ALARM_FALL          = 43, //检测到摔倒
    E_ALARM_CROSS_LINE    = 44, //区域越线检测
    E_ALARM_BIRDS         = 45, //鸟类侦测
    E_ALARM_MANUAL_TRIGGER = 46, //手动触发录像
    E_ALARM_FALL_2        = 47, //检测到摔倒
    E_ALARM_CROSS_LINE_2  = 48, //越线检测
    E_ALARM_TEMP_LOW      = 49, //温度过低报警
    E_ALARM_TEMP_HIGH     = 50, //温度过高报警
    E_ALARM_IPC_CALL_CANCLE = 54, //呼叫取消
} TUYA_ALARM_TYPE_E;

typedef enum {
    E_ALARM_SD_STORAGE    = ALARM_STORAGE_BASE<<0,
    E_ALARM_CLOUD_STORAGE = ALARM_STORAGE_BASE<<1,
} TUYA_ALARM_STORAGE_TYPE_E;


typedef struct {
    INT_T text_type;//enum text
    CHAR_T text[MAX_TEXT_LEN];
} TUYA_ALARM_TEXT_T;

typedef struct {
    INT_T bitmap[MAX_ALARM_BITMAP_NUM];
} TUYA_ALARM_BITMAP_T;

typedef struct {
    INT_T type;                         ///< alarm type
    TUYA_ALARM_BITMAP_T wait_for;       ///< depends on other alarms
    INT_T is_notify;                    ///< notify to cloud
    TIME_T trigger_time;
    TIME_T upload_time;                 ///< 0 means immediately and -1 means wait until event end
    BOOL_T valid;                       ///< 1 means needs to handle and 0 means had handled. user should set to 1
    BOOL_T force;                       ///< force upload or not, independent with event like E_ALARM_COVER
    INT_T resource_type;                ///< refer to macro RESOURCE_XXX
    CHAR_T *extra_data;                 ///< extra data append to upload info, json format: "aaa":3. NULL if not need
    CHAR_T *pic_buf;
    INT_T pic_size;
    TUYA_ALARM_TEXT_T  *context;        ///< text alarm info, can be NULL
    INT_T context_cnt;                  ///< TUYA_ALARM_TEXT_T count, can be 0
    NOTIFICATION_UNIT_T *media;         ///< media info, can be NULL
    INT_T media_cnt;                    ///< NOTIFICATION_UNIT_T count, can be 0
    CHAR_T dev_id[DEV_ID_LEN+1];        ///< deive id
    INT_T  dev_chan_index;              ///< device channle
    CHAR_T *cloud_msg;                  ///< video address of cloud receipt
} TUYA_ALARM_T;

typedef struct {
    UINT64_T trigger_time_ms;
    CHAR_T* pic_buf;
    UINT_T pic_len;
    TUYA_ALARM_TYPE_E alarm_type;
} TUYA_ALARM_INFO_T;

/**
 * @brief start an event
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_event(IN TUYA_ALARM_TYPE_E alarms_types);

/**
 * @brief start an event an with storage
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_event_with_storage(IN TUYA_ALARM_TYPE_E alarms_types, IN INT_T storage_type);

/**
 * @brief start an event with storage. The image is used as the cover photo for events during local playback.
 * 
 * @param[in] alarm_info: TUYA_ALARM_INFO_T
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_event_with_storage_by_alarm_info(TUYA_ALARM_INFO_T* alarm_info, INT_T storage_type);

/**
 * @brief start an event and only once
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_event_once(IN TUYA_ALARM_TYPE_E alarms_types);

/**
 * @brief stop an event
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_event(IN TUYA_ALARM_TYPE_E alarms_types);

/**
 * @brief stop an event and stop storage
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_event_with_storage(IN TUYA_ALARM_TYPE_E alarms_types, IN INT_T storage_type);

/**
 * @brief stop an event and stop storage.This interface is used in conjunction with tuya_ipc_start_event_with_storage_by_alarm_info.
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_event_with_storage_by_alarm_info(TUYA_ALARM_TYPE_E alarms_types, INT_T storage_type);

/**
 * @brief stop an event and stop storage synchronous
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * @param[in] timeout: will return until after timeout*2*500 ms or success
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_event_with_storage_sync(IN TUYA_ALARM_TYPE_E alarms_types, IN INT_T storage_type, IN INT_T timeout);

/**
 * @brief stop an event and stop storage synchronous.This interface is used in conjunction with tuya_ipc_start_event_with_storage_by_alarm_info.
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * @param[in] timeout: will return until after timeout*2*500 ms or success
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_event_with_storage_sync_by_alarm_info(TUYA_ALARM_TYPE_E alarms_types, INT_T storage_type, INT_T timeout);

/**
 * @brief check alarm event begin or not
 * 
 * @param[in] alarms_types: TUYA_ALARM_TYPE_E
 * 
 * @return TRUE/FALSE
 */
BOOL_T tuya_ipc_event_check_status(IN TUYA_ALARM_TYPE_E alarms_types);

/**
 * @brief check storage is working or not
 * 
 * @param[in] storage_type: OR operation with type TUYA_ALARM_STORAGE_TYPE_E
 * 
 * @return TRUE/FALSE 
 */
BOOL_T tuya_ipc_event_check_status_with_storage(IN INT_T storage_type);

/**
 * @brief notify an alarm base on event
 * 
 * @param[in] alarm: TUYA_ALARM_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_alarm_event(IN TUYA_ALARM_T *alarm);

/**
 * @brief notify an alarm not base on event
 * 
 * @param alarm: TUYA_ALARM_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_trigger_alarm_without_event(IN TUYA_ALARM_T *alarm);

/**
 * @brief notify event module device is online, SDK can abtain cofig as soon as possible
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_event_set_online(VOID);

/**
 * @brief add alarm info bitmap
 * 
 * @param[in] bitmap: TUYA_ALARM_BITMAP_T
 * @param[in] alarm_types: TUYA_ALARM_BITMAP_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_event_add_alarm_types(IN TUYA_ALARM_BITMAP_T *bitmap, IN TUYA_ALARM_TYPE_E alarm_types);

/**
 * @brief init event module
 * 
 * @param[in] max_event: number of event, like [video detect] [audio detect] [doorbell]. alarm upload depends on priority in same type
 * @param[in] alarms_of_event: alarms in every single event, usually is a TUYA_ALARM_BITMAP_T array and items number is max_event
 * @param[in] alarm_cnt_per_event: every single event support number of notify in same time
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_event_module_init(IN INT_T max_event, IN TUYA_ALARM_BITMAP_T *alarms_of_event, IN INT_T alarm_cnt_per_event);

/**
 * @brief exit event module release resource
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_event_module_exit(VOID);

/**
 * @brief get string cmd of alarm. like: 0 -> ipc_motion
 * 
 * @param id :TUYA_ALARM_TYPE_E
 * @param cmd_buf 
 * @param cmd_buf_len :max len is MAX_TEXT_LEN
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_ipc_event_alarm_id_to_cmd(INT_T id, CHAR_T *cmd_buf, INT_T cmd_buf_len);

/**
 * @brief notify an alarm only for breakpoint module
 * 
 * @param alarm: TUYA_ALARM_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_event_upload_alarm_for_breakpoint(TUYA_ALARM_T *alarm);

#ifdef __cplusplus
}
#endif

#endif

