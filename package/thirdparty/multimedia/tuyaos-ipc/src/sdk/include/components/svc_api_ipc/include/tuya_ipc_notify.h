/**
 * @file tuya_ipc_notify.h
 * @brief This is tuya ipc notify file
 * @version 1.0
 * @date 2021-11-17
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_IPC_NOTIFY_H__
#define __TUYA_IPC_NOTIFY_H__

#include <time.h>
#include <stdio.h>
#include "tuya_iot_config.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_cloud_com_defs.h"


#ifdef __cplusplus
extern "C" {
#endif


#define IPC_AES_ENCRYPT_KEY_LEN 16
#define IPC_NOTIFY_INFO_LEN 16

#define NOTIFICATION_MOTION_DETECTION 115


/**
 * notification type of the event, currently only JPEG and PNG are supported
 * NOTIFICATION_CONTENT_TYPE_E
 */
typedef enum {
    NOTIFICATION_CONTENT_MP4 = 0,   ///< mp4 for video, NOT supported now
    NOTIFICATION_CONTENT_JPEG,      ///< jpeg for snapshot
    NOTIFICATION_CONTENT_PNG,       ///< png for snapshot
    NOTIFICATION_CONTENT_AUDIO,     ///< audio for video msg
    NOTIFICATION_CONTENT_MAX,
} NOTIFICATION_CONTENT_TYPE_E;

typedef enum {
    NOTIFICATION_NAME_MOTION = 0,               ///< 移动侦测
    NOTIFICATION_NAME_DOORBELL = 1,             ///< 门铃
    NOTIFICATION_NAME_DEV_LINK = 2,             ///< 设备联动
    NOTIFICATION_NAME_PASSBY = 3,               ///< 有人经过
    NOTIFICATION_NAME_LINGER = 4,               ///< 有人徘徊
    NOTIFICATION_NAME_MESSAGE = 5,              ///< 门铃消息留言
    NOTIFICATION_NAME_CALL_ACCEPT = 6,          ///< 门铃已接听
    NOTIFICATION_NAME_CALL_NOT_ACCEPT = 7,      ///< 门铃未接听
    NOTIFICATION_NAME_CALL_REFUSE = 8,          ///< 门铃拒接
    NOTIFICATION_NAME_HUMAN = 9,                ///< 人形检测
    NOTIFICATION_NAME_PCD = 10,                 ///< 宠物检测
    NOTIFICATION_NAME_CAR = 11,                 ///< 车辆检测
    NOTIFICATION_NAME_BABY_CRY = 12,            ///< 婴儿哭声
    NOTIFICATION_NAME_ABNORMAL_SOUND = 13,      ///< 异响
    NOTIFICATION_NAME_FACE = 14,                ///< 检测到人脸
    NOTIFICATION_NAME_ANTIBREAK = 15,           ///< 强拆报警
    NOTIFICATION_NAME_RECORD_ONLY = 16,         ///< 自定义消息
    NOTIFICATION_NAME_IO_ALARM =17,             ///< 摄像头接入io设备告警
    NOTIFICATION_NAME_LOW_BATTERY = 18,         ///< 低电量告警
    NOTIFICATION_NAME_NOUSE = 19,               ///< 门铃呼叫转154
    NOTIFICATION_NAME_COVER = 20,               ///< 封面更新
    NOTIFICATION_NAME_FULL_BATTERY = 21,        ///< 电量已满
    NOTIFICATION_NAME_USER_IO = 22,             ///< io告警2
    NOTIFICATION_NAME_DEV_LINK_2    = 23,       ///< 设备联动2
    NOTIFICATION_NAME_NO_HELMET     = 24,       ///< 未戴安全帽
    NOTIFICATION_NAME_NO_SUIT       = 25,       ///< 未穿工服
    NOTIFICATION_NAME_ENTER_AREA    = 26,       ///< 区域入侵
    NOTIFICATION_NAME_SLEEP         = 27,       ///< 睡岗检测
    NOTIFICATION_NAME_CHN_BLOCK     = 28,       ///< 通道阻塞
    NOTIFICATION_NAME_CALLING       = 29,       ///< 打电话检测
    NOTIFICATION_NAME_SMOKE         = 30,       ///< 吸烟检测
    NOTIFICATION_NAME_LOCAL_FACE    = 31,       ///< 本地人脸检测
    NOTIFICATION_NAME_FIRE          = 32,       ///< 检测到火警
    NOTIFICATION_NAME_AI            = 33,       ///< 检测到AI事件
    NOTIFICATION_NAME_MOTION_CHN    = 34,       ///< 移动侦测通道上报
    NOTIFICATION_NAME_PERSON_PORTRAIT = 35,     ///< 店内客流（客群画像）
    NOTIFICATION_NAME_PERSON_FLOW   = 36,       ///< 出入口客流
    NOTIFICATION_NAME_HOTSPOTS      = 37,       ///< 店内热区
    NOTIFICATION_NAME_CAPTURE       = 38,       ///< 图片抓拍
    NOTIFICATION_NAME_BEHAVIOR      = 39,       ///< 行为分析
    NOTIFICATION_NAME_NON_MOTO      = 40,       ///< 非机动车检测
    NOTIFICATION_NAME_DOORBELl_2    = 41,       ///< 门铃呼叫（多个门铃设备）
    NOTIFICATION_NAME_PACKAGE       = 42,       ///< 包裹检测
    NOTIFICATION_NAME_FALL          = 43,       ///< 检测到摔倒
    NOTIFICATION_NAME_CROSS_LINE    = 44,       ///< 区域越线检测
    NOTIFICATION_NAME_BIRDS         = 45,       ///< 鸟类侦测
    NOTIFICATION_NAME_MANUAL_TRIGGER = 46,      ///< 手动触发录像
    NOTIFICATION_NAME_FALL_2        = 47,       ///< 检测到摔倒
    NOTIFICATION_NAME_CROSS_LINE_2  = 48,       ///< 越线检测
    NOTIFICATION_NAME_TEMP_LOW      = 49,       ///< 温度过低报警
    NOTIFICATION_NAME_TEMP_HIGH     = 50,       ///< 温度过高报警
    NOTIFICATION_NAME_IPC_CALL_CANCLE = 54,     ///< 呼叫取消
    NOTIFY_TYPE_MAX = 1024
} NOTIFICATION_NAME_E;

typedef enum {
    DOORBELL_NORMAL,    ///< normal doorbell, push image
    DOORBELL_AC,        ///< AC doorbell, push P2P
    DOORBELL_TYPE_MAX,
} DOORBELL_TYPE_E;


typedef struct {
    char *data;
    int len;
    char secret_key[IPC_AES_ENCRYPT_KEY_LEN + 1];
    NOTIFICATION_CONTENT_TYPE_E type;
} NOTIFICATION_UNIT_T;

typedef struct {
    INT_T unit_count;
    NOTIFICATION_NAME_E name;
    CHAR_T *extra_data; ///< default:NULL
    CHAR_T type[IPC_NOTIFY_INFO_LEN];   ///< jpg:"image",video:""
    CHAR_T with[IPC_NOTIFY_INFO_LEN];   ///< default "resources"
    NOTIFICATION_UNIT_T unit[0];
} NOTIFICATION_DATA_GENERIC_T;

typedef struct {
    CHAR_T cmd[64];         ///< event
    CHAR_T sub_version[32]; ///< version
    CHAR_T event_type[64];  ///< event type
    CHAR_T event_id[64];    ///< event ID
    INT_T   notify_message_center; ///< 0 not notify, 1 notify
    INT_T body_len;
    CHAR_T *body_data;
} TUYA_IPC_AI_EVENT_CTX_T;


/**
 * @brief init notify module lock
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_notify_init_lock(VOID);


/**
 * @brief send a doorbell pressing message to tuya cloud and APP
 * 
 * @param[in] snap_buffer: address of current snapshot
 * @param[in] snap_size: size of snapshot, in Byte
 * @param[in] type: snapshot file type, jpeg or png
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 * 
 * @warning this API will be abandoned later. Use tuya_ipc_door_bell_press instead
 */
OPERATE_RET tuya_ipc_notify_door_bell_press(IN CONST CHAR_T *snap_buffer, IN CONST UINT_T snap_size, IN CONST NOTIFICATION_CONTENT_TYPE_E type);

/**
 * @brief send a doorbell pressing message to tuya cloud and APP
 * 
 * @param[in] doorbell_type: DOORBELL_NORMAL or DOORBELL_AC
 * @param[in] snap_buffer: address of current snapshot
 * @param[in] snap_size: size of snapshot, in Byte
 * @param[in] type: snapshot file type, jpeg or png
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_door_bell_press(IN CONST DOORBELL_TYPE_E doorbell_type, IN CONST CHAR_T *snap_buffer, IN CONST UINT_T snap_size, IN CONST NOTIFICATION_CONTENT_TYPE_E type);

/**
 * @brief send a doorbell pressing stop message to tuya cloud and APP(only for ac doorbell)
 * 
 * @param VOID
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
*/
OPERATE_RET tuya_ipc_door_bell_press_stop(VOID);
/***********************************************************
*  Function: tuya_ipc_door_bell_press_cancel
*  Description: cancel ac doorbell
*  Input: none
*  Output: none
*  Return: Return: sucess: OPRT_OK.failed :not OPRT_OK;
***********************************************************/
OPERATE_RET tuya_ipc_door_bell_press_cancel();

/**
 * @brief start storage
 * 
 * @param[in] storage_type: OR operation use type TUYA_ALARM_STORAGE_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_start_storage(IN INT_T storage_type);

/**
 * @brief stop storage
 * 
 * @param[in] storage_type: OR operation use type TUYA_ALARM_STORAGE_TYPE_E
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_stop_storage(IN INT_T storage_type);

/**
 * @brief send a editable alarm to tuya cloud and APP
 * 
 * @param[in] snap_buffer: address of current snapshot
 * @param[in] snap_size: size fo snapshot, in Byte
 * @param[in] name: editable event type, NOTIFICATION_NAME_E
 * @param[in] is_notify: send to message center if TRUE
 * @param[in] cloud_result: cloud results such as AI detection results and video address
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_notify_alarm(IN CONST CHAR_T *snap_buffer, IN CONST UINT_T snap_size, IN CONST NOTIFICATION_NAME_E name, IN BOOL_T is_notify, IN CONST CHAR_T *cloud_result);

/**
 * @brief send customized mqtt message to tuya cloud
 * 
 * @param[in] protocol: mqtt protocol number
 * @param[in] p_data: protocol data
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_send_custom_mqtt_msg(IN CONST UINT_T protocol, IN CONST CHAR_T *p_data);

/**
 * @brief report living message to tuya APP
 * 
 * @param[in] error_code: 0 success, others fail
 * @param[in] force: useless now
 * @param[in] timeout: useless now
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_report_living_msg(IN UINT_T error_code, IN UINT_T force, IN UINT_T timeout);

/**
 * @brief notify an ai event
 * 
 * @param[in] ctx: TUYA_IPC_AI_EVENT_CTX_T
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_event_notify(IN TUYA_IPC_AI_EVENT_CTX_T *ctx);

/**
 * @brief upload ai image
 * 
 * @param[in] image_buf: image buffer
 * @param[in] image_size: image size
 * @param[out] result: result in json format
 * @param[in] result_len: can not less 256,must >=256
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_image_upload(IN CHAR_T *image_buf, IN UINT_T image_size, OUT CHAR_T *result, IN UINT_T result_len);


#ifdef __cplusplus
}
#endif

#endif

