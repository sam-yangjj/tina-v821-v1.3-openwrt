/*
 * tuya_ipc_common_demo.h
 *Copyright(C),2017-2022, TUYA company www.tuya.comm
 *
 *FILE description:
  *
 *      Author: dante
 */

#ifndef __TY_SDK_COMMON_H_
#define __TY_SDK_COMMON_H_
#include <stdio.h>
#include "tuya_cloud_types.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_ipc_media_stream_event.h"
#include "tuya_ipc_media_stream.h"
#include "tuya_ipc_video_msg.h"
#include "tuya_ipc_event.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_album.h"
#include "tuya_ipc_rule.h"
#include "tuya_ipc_ai_station.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VOID (*TUAY_QRCODE_ACTIVE_CB)(CHAR_T* shorturl);

/**
* the fifth version api
*/
/*** separator begin  ****/
typedef struct
{
	CHAR_T product_key[IPC_PRODUCT_KEY_LEN + 1]; /* one product key(PID) for one product  */
	CHAR_T uuid[IPC_UUID_LEN + 1]; /* one uuid-authkey pair for one device, to register on TUYA coud */
	CHAR_T auth_key[IPC_AUTH_KEY_LEN + 1];
	CHAR_T dev_sw_version[IPC_SW_VER_LEN + 1]; /* software version with format x1.x2.x3.x4.x5.x6, five dots maximum, zero dots minimum */
	CHAR_T cfg_storage_path[IPC_STORAGE_PATH_LEN + 1];
    TUYA_RST_INFORM_CB gw_reset_cb;/* reset callback fucntion, triggered when user unbind device from a account */
    TUYA_RESTART_INFORM_CB gw_restart_cb;/* restart callback function */
    TUYA_IPC_DEV_TYPE_E   dev_type;/*dev type ,eg low power device*/
}TUYA_IPC_SDK_BASE_S;

typedef struct
{
	TUYA_IPC_PARING_MODE_E connect_mode;
	TUYA_IPC_STATUS_CHANGED_CB ipc_status_change_cb;
	TUYA_IPC_LINK_TYPE_E link_type;
	TUYA_IPC_IP_MODE_TYPE ip_mode_type;
}TUYA_IPC_SDK_NET_S;


typedef struct
{
	DEV_DP_QUERY_CB dp_query;
	DEV_RAW_DP_CMD_CB  raw_dp_cmd_proc;
	DEV_OBJ_DP_CMD_CB common_dp_cmd_proc;
}TUYA_IPC_SDK_DP_S;
typedef struct
{
    BOOL_T enable ;
    CHAR_T upgrade_file[128];
    TUYA_IPC_SDK_DEV_UPGRADE_INFORM_CB upgrade_cb;
}TUYA_IPC_SDK_DEMO_UPGRADE_S;
typedef struct
{
    BOOL_T enable;
}TUYA_IPC_SDK_QUICK_START_S;
typedef struct
{
    INT_T log_level;/*set log level default: error level*/
    CHAR_T qrcode_token[16]; //connect_method !=2 ,qrcode_token invalid
}TUYA_IPC_SDK_DEBUG_S;

typedef struct
{
	IPC_MEDIA_INFO_T media_info;
  IPC_MEDIA_INFO_T media_info2;//another sensor
}TUYA_IPC_SDK_MEDIA_STREAM_S;

typedef struct
{
	BOOL_T enable; /*enable == false ,other var is invalid*/
	UINT_T max_stream_client; /* stream clinet max connect number*/
    TRANS_DEFAULT_QUALITY_E live_mode;  /* for multi-streaming ipc, the default quality for live preview */
	MEDIA_STREAM_EVENT_CB media_event_cb; /*media event callback*/
	MEDIA_RECV_AUDIO_CB rev_audio_cb;/*app to dev audio callback*/
    MEDIA_RECV_VIDEO_CB rev_video_cb;/*app to dev audio callback*/
    MEDIA_RECV_FILE_CB rev_file_cb;/*app to dev file callback*/
    MEDIA_GET_SNAPSHOT_CB get_snapshot_cb;
}TUYA_IPC_SDK_MEDIA_ADAPTER_S;

typedef struct
{
	BOOL_T enable; /*enable == false ,other var is invalid*/
	CHAR_T storage_path[IPC_STORAGE_PATH_LEN + 1];
	UINT_T max_event_num_per_day ;
	UINT_T  preview_record_time;
	UINT_T  skills;//参考tuya_ipc_stream_storage.h TUYA_IPC_SKIL_BASIC
	SS_SD_STATUS_CHANGED_CB sd_status_cb;
}TUYA_IPC_SDK_LOCAL_STORAGE_S;

typedef struct
{
	BOOL_T enable; /*enable == false ,other var is invalid*/
	BOOL_T en_audio_record; /* default enable */
	INT_T pre_record_time;
}TUYA_IPC_SDK_CLOUD_STORAGE_S;

typedef struct
{
	BOOL_T enable; /*enable == false ,other var is invalid*/
	MESSAGE_E type; /* video msg type: video+audio, video only, audio only*/
	INT_T msg_duration; /* the duration of video record */
}TUYA_IPC_SDK_VIDEO_MSG_S;

typedef struct
{
    BOOL_T enable;
    INT_T  max_event;
    TUYA_ALARM_BITMAP_T *alarms_of_events;
    INT_T  alarms_cnt_per_event;
}TUYA_IPC_SDK_EVENT_S;

typedef struct 
{
    BOOL_T enable;
}TUYA_IPC_SDK_CALL_S;

typedef struct
{
    BOOL_T ai_enable;
    BOOL_T ai_chat_enable;
    BOOL_T ai_detect_enable;
    RULE_STREAMING_TYPE_E streaming_type;
    TUYA_IPC_AI_CMD_CB ai_cmd_cb;
    TUYA_IPC_AI_AUDIO_PLAY_CB audio_play_cb;
    RULE_AI_RESULT_CB on_ai_result_cb; /** ai result callback function */
    RULE_VIDEO_MSG_CB on_video_msg_cb; /** video msg callback function */
}TUYA_IPC_SDK_AI_S;

/**
 * \brief IPC SDK run variable set
 * \struct TUYA_IPC_SDK_RUN_VAR_S
 */
typedef struct
{
	TUYA_IPC_SDK_BASE_S  iot_info;/*sdk base configure information*/
	TUYA_IPC_SDK_MEDIA_STREAM_S media_info;/*audio video configure information*/
	TUYA_IPC_SDK_NET_S net_info;/*net work info*/
	TUYA_IPC_SDK_DEMO_UPGRADE_S upgrade_info;/*fireware upgrade information*/
	TUYA_IPC_SDK_DP_S dp_info;/*date point information*/
	TUYA_IPC_SDK_MEDIA_ADAPTER_S media_adatper_info;/*media adapter information*/
	TUYA_IPC_SDK_LOCAL_STORAGE_S local_storage_info;/* sd card storage information*/
	TUYA_IPC_SDK_CLOUD_STORAGE_S cloud_storage_info;/*cloud storage information*/
	TUYA_IPC_SDK_VIDEO_MSG_S  video_msg_info;/*door bell function information*/
	TUYA_IPC_SDK_QUICK_START_S  quick_start_info;/*start way for p2p  */
	TUYA_IPC_SDK_EVENT_S event_info;/* event information */
	TUYA_IPC_SDK_DEBUG_S debug_info;/*debug info sets*/
	TUAY_QRCODE_ACTIVE_CB    qrcode_active_cb;
	TUYA_IPC_SDK_CALL_S	call_info;
	TUYA_IPC_SDK_AI_S ai_info;
}TUYA_IPC_SDK_RUN_VAR_S;

#ifdef __cplusplus
}
#endif
#endif
