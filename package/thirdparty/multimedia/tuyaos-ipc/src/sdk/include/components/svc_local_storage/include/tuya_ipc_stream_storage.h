/**
 * @file tuya_ipc_stream_storage.h
 * @brief This is tuya ipc stream storage file
 * @version 1.0
 * @date 2021-11-17
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_IPC_STREAM_STORAGE_H__
#define __TUYA_IPC_STREAM_STORAGE_H__

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"
#include "tuya_ipc_media.h"
#include "tuya_ipc_event.h"
#include "tuya_ipc_notify.h"
#include "tuya_list.h"
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IPC_CHANNEL_NUM
#define SS_IPC_CHAN_NUM IPC_CHANNEL_NUM
#else
#define SS_IPC_CHAN_NUM 1
#endif

#define SS_BASE_PATH_LEN                 128
#define DISK_CAPACITY_LOW_THRESH_KB      20*1024     ///< threshold to trigger a low disk space waning to APP
#define DISK_CLEAN_THRESH_KB             512*1024    ///< threshold to trigger auto deleting the oldest data
#define CAPACITY_REPORT_CLEAN_THRESH_KB  512*1024    ///< threshold to trigger a disk-space-change notification to APP

#define ABS_PATH_FILE_LEN 128   ///< absolute path name max length
#define SS_ALBUM_MAX_NUM (4)    ///< album number
#define SS_BASE_FILE_NAME_LEN  (48)

#define TUYA_IPC_SKILL_BASIC                (1<<0)
#define TUYA_IPC_SKILL_DOWNLOAD             (1<<1)
#define TUYA_IPC_SKILL_SPEED_PLAY_0Point5   (1<<8)
#define TUYA_IPC_SKILL_SPEED_PLAY_2         (1<<9)
#define TUYA_IPC_SKILL_SPEED_PLAY_4         (1<<10)
#define TUYA_IPC_SKILL_SPEED_PLAY_8         (1<<11)
#define TUYA_IPC_SKILL_SPEED_PLAY_16        (1<<12)
#define TUYA_IPC_SKILL_SPEED_PLAY_32        (1<<13)
#define TUYA_IPC_SKILL_SPEED_PLAY_0Point25  (1<<15)
#define TUYA_IPC_SKILL_DELETE_BY_DAY        (1<<16)
#define TUYA_IPC_SKILL_SUPPORT_PB_V21       (1<<27)

typedef enum {
    SD_STATUS_UNKNOWN = 0,
    SD_STATUS_NORMAL,
    SD_STATUS_ABNORMAL,
    SD_STATUS_LACK_SPACE,
    SD_STATUS_FORMATING,
    SD_STATUS_NOT_EXIST,
    SD_STATUS_POPUP_REMINDER,       // After the SD card is inserted, pop-up configuration: local storage switch + video mode setting
    SD_STATUS_NOT_MOUNT = 7,        // No card waiting
    SD_STATUS_MAX = 0xFF
} SD_STATUS_E;

typedef enum {
    E_DELETE_NONE,
    E_DELETE_YEAR,
    E_DELETE_MONTH,
    E_DELETE_DAY,
    E_DELETE_EVENT,
    E_DELETE_INVALID,
    E_DELETE_FS_ERROR
} DELETE_LEVEL_E;

typedef enum {
    E_STORAGE_STOP,
    E_STORAGE_START,
    ///< below are internal status, should NOT be used for developer
    E_STORAGE_ONGOING,
    E_STORAGE_READY_TO_STOP,
    E_STORAGE_RESTART,
    E_STORAGE_UNWRITEABLE,
    E_STORAGE_INVALID
} STORAGE_STATUS_E;

typedef enum {
    STORAGE_AOV_INVALID = 0,        // not init
    STORAGE_AOV_STOP,
    STORAGE_AOV_START,
    STORAGE_AOV_ONGOING,
    STORAGE_AOV_QUIT,
    STORAGE_AOV_WAITING             // wait for frame
} STORAGE_AOV_STATUS_E;

typedef enum {
    SS_WRITE_MODE_NONE, ///< do NOT write av data
    SS_WRITE_MODE_EVENT,///< write av data only event happens
    SS_WRITE_MODE_ALL,  ///< write av data all time
    SS_WRITE_MODE_MAX
} STREAM_STORAGE_WRITE_MODE_E;

typedef enum {
    E_STG_UNINIT,
    E_STG_INITING,
    E_STG_INITED,
    E_STG_MAX
} STG_INIT_STATUS_E;

typedef enum {
    SS_PB_FINISH = 0, ///< end of event, no more data file
    SS_PB_NEWFILE,    ///< new file found, keep sending playback data
} SS_PB_EVENT_E;

typedef enum{
    SS_PB_05TIMES = 0,  ///< 0.5 speed
    SS_PB_10TIMES = 1,  ///< normal speed
    SS_PB_15TIMES = 2,
    SS_PB_20TIMES = 3,  ///< 2.0 speed
    SS_PB_25TIMES = 4,  ///< 2.5 speed
    SS_PB_30TIMES = 5,  ///< 3.0 speed
    SS_PB_35TIMES = 6,  ///< 3.5 speed    
    SS_PB_40TIMES = 7,  ///< 4.0 speed
    SS_PB_80TIMES = 8,  ///< 8 speed
    SS_PB_160TIMES = 9,  ///< 16 speed
    SS_PB_320TIMES = 10,    ///< 32 speed
    SS_PB_025TIMES = 20,    ///< 0.25 speed
    SS_PB_SPEED_MAX
} SS_PB_SPEED_E;

typedef enum {
    SS_PB_MUTE,
    SS_PB_UN_MUTE,
    SS_PB_PAUSE,
    SS_PB_RESUME,
} SS_PB_STATUS_E;

typedef enum {
    SS_DL_IDLE,
    SS_DL_START,
    SS_DL_STOP, ///< abandoned
    SS_DL_PAUSE,
    SS_DL_RESUME,
    SS_DL_CANCLE,
} SS_DOWNLOAD_STATUS_E;


typedef struct {
    CHAR_T base_path[SS_BASE_PATH_LEN]; ///< base path of storage, /mnt/sdcard/ e.g.
    TUYA_CODEC_ID_E video_codec;       ///< codec type of video stream
    TUYA_CODEC_ID_E audio_codec;       ///< codec type of audio stream
    VOID *p_reserved;
} STREAM_STORAGE_INFO_T;

typedef struct {
    INT_T cnt;
    CHAR_T album_name[SS_ALBUM_MAX_NUM][SS_BASE_FILE_NAME_LEN];
} TUYA_IPC_ALBUM_INFO_T;

typedef struct {
    BOOL_T user_defined_name;                                         // FALSE:default name(base_path/DCIM/CHAN0); TRUE:DCIM&CHAN can be user-defined
    CHAR_T first_level_directory_name[SS_BASE_FILE_NAME_LEN];         // first level directory(e.g. DCIM)
    CHAR_T channel_directory_prefix_name[SS_BASE_FILE_NAME_LEN];      // channel directory prefix name(e.g. CHAN)
} TUYA_IPC_STORAGE_WORKING_PATH_NAME_T;

typedef struct {
    BOOL_T aov_stor_enable;                       ///< 1: aov enable
    IPC_STREAM_E enable_channel;                  ///< channel of enabled AOV
    UINT_T aov_video_frame_duration_ms;           ///< aov frame duration(1/fps)(ms)
    UINT_T aov_video_fps;                         ///< aov fps
    UINT_T aov_video_bitrate;                     ///< aov bitrate
    UINT_T aov_video_gop;                         ///< aov gop
    UINT_T aov_video_width;                       ///< aov width
    UINT_T aov_video_height;                      ///< aov height
    TUYA_CODEC_ID_E aov_video_codec;              ///< aov codec
} TUYA_IPC_AOV_STORAGE_INFO_T;


typedef OPERATE_RET (*SS_SD_STATUS_CHANGED_CB)(int status);

typedef struct {
    CHAR_T base_path[SS_BASE_PATH_LEN]; ///< base path for storage device
    UINT_T max_event_per_day;   ///< max event number per day, this limitation should be based on Soc ability,
                                ///< to query and return playback information within acceptable time.
                                ///< tuya sdk will reject query message if there are more event than this limit
    SS_SD_STATUS_CHANGED_CB  sd_status_changed_cb;
    INT_T stor_format;  //0 default(support encrypt_mode and security_level)  1 mp4(not support encrypt_mode and security_level)
    INT_T encrypt_mode;///< 1: enable encrypt
    INT_T security_level;///< encrypt level
    UINT_T  skills;             ///< 0 means default, refers to TUYA_IPC_SKILL_BASIC | TUYA_IPC_SKILL_SPEED_PLAY_2
    TUYA_IPC_ALBUM_INFO_T album_info;
    INT_T max_event_duration;///< event file during, 0 means default
    UINT_T write_card_period_s;    ///< period of video writing card; 0: default(6s) 
    TUYA_IPC_STORAGE_WORKING_PATH_NAME_T path_name; ///< path name
    TUYA_IPC_AOV_STORAGE_INFO_T aov_info;       ///< aov recording parameters 
} TUYA_IPC_STORAGE_VAR_T;

typedef struct {
    INT_T video_type;       ///< 0:normal 1:AOV
    INT_T type;             ///< TUYA_ALARM_TYPE_E(not used)
    UINT_T start_timestamp; ///< start time of the the video/audio file, in second
    UINT_T end_timestamp;   ///< end time of the the video/audio file, in second
} SS_FILE_TIME_TS_T;

typedef struct {
    UINT_T file_count; ///< file count within one day
    SS_FILE_TIME_TS_T file_arr[0]; ///< file list
} SS_QUERY_DAY_TS_ARR_T;

#pragma pack(4)
typedef struct {
    INT_T encrypt;
    INT_T security_level;
    CHAR_T uuid[32];
    BYTE_T iv[16];
    BYTE_T key_hash[16];
} SS_ENCRYPT_INFO_T;

typedef struct {
    INT_T video_type;      ///< 0:normal 1:AOV
    INT_T type;            ///< TUYA_ALARM_TYPE_E(not used)
    UINT_T start_timestamp; 
    UINT_T end_timestamp;
    SS_ENCRYPT_INFO_T enrypt_info;
} SS_DAY_TS_ENRTY_T;

typedef struct {
    UINT_T file_count; 
    SS_DAY_TS_ENRTY_T file_arr[0];
} SS_DAY_TS_RESULT_T;

typedef struct {
    LIST_HEAD entry;

    UINT_T video_type;      ///< 0:normal 1:AOV
    UINT_T start_timestamp; 
    UINT_T end_timestamp;
    SS_ENCRYPT_INFO_T enrypt_info;

    UINT_T event_cnt;          ///< event_type_arr 数组大小
    UINT_T event_type_arr[0];  ///< 事件类型,可变数组
} SS_DAY_TS_ENRTY_WITH_EVENT_TYPE_T;

typedef struct {
    LIST_HEAD entry;                            // 链表内node类型为: SS_DAY_TS_ENRTY_WITH_EVENT_TYPE_T
    UINT_T list_cnt;                            // list node 数量
    UINT_T event_type_total_cnt;                // 所有录像片段中的事件类型数量总和
} SS_DAY_TS_RESULT_LIST_WITH_EVENT_TYPE_T;
#pragma pack()

typedef struct {
    TUYA_CODEC_ID_E video_codec;
    UINT_T frame_rate;
    UINT_T video_width;
    UINT_T video_height;
    UINT_T gop_size;
} SS_IPC_VIDEO_INFO_T;

typedef struct {
    TUYA_CODEC_ID_E audio_codec;
    TUYA_AUDIO_SAMPLE_E audio_sample;
    TUYA_AUDIO_DATABITS_E audio_databits;
    TUYA_AUDIO_CHANNEL_E audio_channel;
} SS_IPC_AUDIO_INFO_T;

typedef struct{
    MEDIA_FRAME_TYPE_E frame_type;
    BYTE_T    *p_buf;
    UINT_T    size;
    UINT64_T  pts;
    UINT64_T  timestamp;
    union{
  		SS_IPC_VIDEO_INFO_T video;
  		SS_IPC_AUDIO_INFO_T audio;
	}media;
  	SS_ENCRYPT_INFO_T encrypt_info;
}SS_MEDIA_FRAME_WITH_ENCRYPT_T;

typedef struct {
    UINT_T start_timestamp; ///< start time of the the video/audio file, in second
    UINT_T end_timestamp;   ///< end time of the the video/audio file, in second
} SS_FILE_INFO_T;

typedef struct {
    UINT_T dl_start_time;           ///< download start time
    UINT_T dl_end_time;             ///< download end time
    UINT_T file_count;
    SS_FILE_INFO_T *p_file_info_arr;   ///< pointer of array of files to download
} SS_DOWNLOAD_FILES_TS_T;

typedef struct {
    UINT_T result;                                        // 0成功 1失败
    SS_FILE_INFO_T time_sect;                                  // 开始下载时间点
} SS_FILE_TRANS_CTRL_PB_DELDATA_FRAGMENT_T;

typedef struct {
    UINT_T pb_idx;                          // p2p 索引
    SS_FILE_INFO_T time_sect;
    INT_T encrypt_allow;                    // 图片是否允许加密
} SS_IMAGE_QUERY_INFO_T;

typedef struct {
    INT_T pic_name_len;                     // 图片文件名长度
    CHAR_T* pic_name_buf;                   // 图片文件名buf, 由外部申请
    INT_T encrypt;                          // 图片数据是否加密
    INT_T p_buf_len;                        // 图片数据长度
    CHAR_T* p_buf;                          // 图片buf, 内部申请的空间，在p2p内部发送后释放
} SS_IMAGE_QUERY_OUT_T;

typedef struct {
    UINT64_T event_timestamp_ms;
    CHAR_T* pic_buf;
    UINT_T pic_len;
    INT_T event_count;                     // 新增事件数量
    TUYA_ALARM_TYPE_E* event_type_arr;     // 事件类型数组，支持同一时刻多个事件类型
} SS_EVENT_DETAIL_INFO_T;

typedef OPERATE_RET (*STREAM_STORAGE_GET_MEDIA_CB)(IN CONST INT_T camera_id,IN OUT IPC_MEDIA_INFO_T *param);

typedef VOID (*SS_PB_EVENT_CB)(IN UINT_T pb_idx, IN SS_PB_EVENT_E pb_event, IN PVOID_T args);

typedef VOID (*SS_PB_GET_MEDIA_CB)(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_T *p_frame);

typedef VOID (*SS_PB_GET_MEDIA_WITH_ENCRYPT_CB)(IN UINT_T pb_idx, IN UINT_T reqId, IN CONST SS_MEDIA_FRAME_WITH_ENCRYPT_T *p_frame);


///< APIs should be implemented by developer
extern SD_STATUS_E tuya_ipc_sd_get_status(VOID);
extern VOID tuya_ipc_sd_get_capacity(UINT_T *p_total, UINT_T *p_used, UINT_T *p_free);
extern VOID tuya_ipc_sd_get_capacity_by_mount_path(CONST CHAR_T* mount_path, UINT_T *p_total, UINT_T *p_used, UINT_T *p_free);
extern VOID tuya_ipc_sd_remount(VOID);
extern CHAR_T *tuya_ipc_get_sd_mount_path(VOID);
extern STREAM_STORAGE_WRITE_MODE_E tuya_ipc_sd_get_mode_config();


/**
 * @brief initialize tuya ipc stream storage
 * 
 * @param[in] p_storage_var: pointer
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_init(IN TUYA_IPC_STORAGE_VAR_T *p_storage_var);

/**
 * @brief uninit tuya stream storage, free used resource, memory e.g.
 * 
 * @param VOID 
 * 
 * @return VOID 
 */
VOID tuya_ipc_ss_uninit(VOID);

/**
 * @brief if stream storage is inited
 * 
 * @param VOID 
 * 
 * @return TRUE/FALSE 
 */
BOOL_T tuya_ipc_ss_is_inited(VOID);

/**
 * @brief set wtote mode of stream storage
 * 
 * @param[in] write_mode 
 * 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_ipc_ss_set_write_mode(IN CONST STREAM_STORAGE_WRITE_MODE_E write_mode);

/**
 * @brief get current stream storage write mode
 * 
 * @param VOID 
 * 
 * @return STREAM_STORAGE_WRITE_MODE_E 
 */
STREAM_STORAGE_WRITE_MODE_E tuya_ipc_ss_get_write_mode(VOID);

/**
 * @brief if stream storage write mode is enabled
 * 
 * @param VOID 
 * 
 * @return TRUE/FALSE 
 */
BOOL_T tuya_ipc_ss_write_mode_is_enabled(VOID);

/**
 * @brief start event stream storage by channel
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_start_event(IN INT_T channel);

/**
 * @brief stop event stream storage by channel
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_stop_event(IN INT_T channel);

/**
 * @brief create and start event stream storage 
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * @param[in] event_info: event info
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_start_event_by_info(IN INT_T channel, IN SS_EVENT_DETAIL_INFO_T* event_info);

/**
 * @brief add event info after tuya_ipc_ss_start_event_by_info
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * @param[in] event_info: event info
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_append_event_info(IN INT_T channel, IN SS_EVENT_DETAIL_INFO_T* event_info);

/**
 * @brief stop event stream storage after tuya_ipc_ss_start_event_by_info
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_stop_event_by_info(IN INT_T channel);

/**
 * @brief set pre-record time, invoke this API only if needed, or it is 2 seconds by default.
 *        Should be invoked once after init and before start.
 * 
 * @param[in] pre_record_second: time in second
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_set_pre_record_time(IN UINT_T pre_record_second);

/**
 * @brief set max event duration, invoke this API only if needed, 
 *        or it is 600 seconds for private format, 120 seconds for mp4 by default.
 *        Should be invoked onece after init and before start.
 * 
 * @param[in] max_event_duration: time in second
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_set_max_event_duration(IN UINT_T max_event_duration);

/**
 * @brief delete the oldest event. Event is the smallest unit to delete.
 * 
 * @param VOID 
 * 
 * @return DELETE_LEVEL_E 
 */
DELETE_LEVEL_E tuya_ipc_ss_delete_oldest_event(VOID);

/**
 * @brief delete all video/audio data stored by tuya SDK
 * 
 * @param VOID 
 * 
 * @return VOID 
 */
VOID tuya_ipc_ss_delete_all_files(VOID);

/**
 * @brief get current stream storage status
 * 
 * @param VOID 
 * 
 * @return STORAGE_STATUS_E 
 */
STORAGE_STATUS_E tuya_ipc_ss_get_status(VOID);

/**
 * @brief query the accumulative day count that has stream stored, in specific year and month
 * 
 * @param[in] channel: which channel, default: 0(camera0)
 * @param[in] query_year: year
 * @param[in] query_month: month
 * @param[out] p_return_days: days count
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_pb_query_by_month(IN INT_T channel, IN USHORT_T query_year, IN USHORT_T query_month, OUT UINT_T *p_return_days);

/**
 * @brief query video/audio informarion in certain year/month/day
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] channel: which channel, default: 0(camera0)
 * @param[in] year: which year
 * @param[in] month: which montu
 * @param[in] day: which day
 * @param[out] pp_ts_arr: array of query result
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_pb_query_by_day(IN UINT_T pb_idx, IN INT_T channel, IN USHORT_T year, IN USHORT_T month, IN UCHAR_T day,
                                          OUT SS_QUERY_DAY_TS_ARR_T **pp_ts_arr);

OPERATE_RET tuya_ipc_pb_query_by_day_with_encrypt_info(IN UINT_T pb_idx, IN INT_T channel, IN BOOL_T allow_encrypt, IN USHORT_T year,IN USHORT_T month, IN UCHAR_T day,
                                      OUT SS_DAY_TS_RESULT_T **pp_ts_arr);

OPERATE_RET tuya_ipc_pb_query_by_day_with_event_type(IN UINT_T pb_idx, IN INT_T channel, IN BOOL_T allow_encrypt, IN USHORT_T year,IN USHORT_T month, IN UCHAR_T day,
                                                OUT SS_DAY_TS_RESULT_LIST_WITH_EVENT_TYPE_T *p_event_list);
/**
 * @brief free resource of API tuya_ipc_pb_query_by_day output
 * 
 * @param[in] p_query_ts_arr: pointer of array to be free
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_pb_query_free_ts_arr(IN SS_QUERY_DAY_TS_ARR_T *p_query_ts_arr);
OPERATE_RET tuya_ipc_pb_query_free_ts_arr_with_encrypt(IN SS_DAY_TS_RESULT_T *p_query_ts_arr);
OPERATE_RET tuya_ipc_pb_query_free_ts_list_with_event_type(IN SS_DAY_TS_RESULT_LIST_WITH_EVENT_TYPE_T *p_query_list);

/**
 * @brief start a new playback
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] event_cb: callback function of playback event
 * @param[in] video_cb: callback function of getting playback video data
 * @param[in] audio_cb: callback function of getting playback audio data
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_start(IN UINT_T pb_idx, IN SS_PB_EVENT_CB event_cb, IN SS_PB_GET_MEDIA_CB video_cb, IN SS_PB_GET_MEDIA_CB audio_cb);


OPERATE_RET tuya_ipc_ss_pb_start_with_encrypt(IN UINT_T pb_idx, IN INT_T reqId,IN BOOL_T allow_encrypt, IN SS_PB_EVENT_CB event_cb,
                                 IN SS_PB_GET_MEDIA_WITH_ENCRYPT_CB video_encrypt_cb, IN SS_PB_GET_MEDIA_WITH_ENCRYPT_CB audio_encrypt_cb);
/**
 * @brief set playback speed
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] pb_speed: playback speed
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_set_speed(IN UINT_T pb_idx, IN UINT_T pb_speed);
OPERATE_RET tuya_ipc_ss_pb_set_speed_with_reqId(IN UINT_T pb_idx, INT_T reqId, IN UINT_T pb_speed);

/**
 * @brief playback accelerate send one frame by time_ms faster, default 8ms
 * 
 * @param[in] time_ms: time to accelerate for send frame
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_accelerate_send_frame(IN INT_T time_ms);

/**
 * @brief set playback status
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] new_status: status to be set
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_set_status(IN UINT_T pb_idx, IN SS_PB_STATUS_E new_status);
OPERATE_RET tuya_ipc_ss_pb_set_status_with_reqId(IN UINT_T pb_idx, IN SS_PB_STATUS_E new_status, IN INT_T reqId);

/**
 * @brief stop an ongoing playback 
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_stop(IN UINT_T pb_idx);

/**
 * @brief seek file to match wanted timestamp
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] pb_file_info: file to seek
 * @param[in] play_timestamp: time to seek
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_seek(IN UINT_T pb_idx, IN SS_FILE_TIME_TS_T *pb_file_info, IN UINT_T play_timestamp);
OPERATE_RET tuya_ipc_ss_pb_seek_with_reqId(IN UINT_T pb_idx, IN INT_T reqId, IN SS_FILE_TIME_TS_T* pb_file_info, IN UINT_T play_timestamp);

/**
 * @brief stop all ongoing playback
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_pb_stop_all(VOID);

/**
 * @brief do some pre work, before download
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] pb_files_info: file to seek
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_donwload_pre(IN UINT_T pb_idx, IN SS_DOWNLOAD_FILES_TS_T *pb_files_info);
OPERATE_RET tuya_ipc_ss_donwload_pre_support_encrypt(IN UINT_T pb_idx, IN INT_T allow_encrypt, IN SS_DOWNLOAD_FILES_TS_T* pb_files_info);

/**
 * @brief set download status
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] new_status: status
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_download_set_status(IN UINT_T pb_idx, IN SS_DOWNLOAD_STATUS_E new_status);

/**
 * @brief download image from storage
 * 
 * @param[in] query_in: query info
 * @param[in] image_out: query result
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_download_image(IN UINT_T pbIdx, IN SS_FILE_INFO_T fileInfo,
    									OUT INT_T* len, OUT CHAR_T** pBuf);

OPERATE_RET tuya_ipc_download_image_v2(IN SS_IMAGE_QUERY_INFO_T* query_in, OUT SS_IMAGE_QUERY_OUT_T* image_out);								

/**
 * @brief delete video/audio/image in certain year/month/day
 * 
 * @param[in] pb_idx: playback/query index, for different client do query in the same time
 * @param[in] year: which year
 * @param[in] month: which year
 * @param[in] day: which year
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_delete_video(IN UINT_T pb_idx, IN UINT_T year, IN UINT_T month, IN UINT_T day);

OPERATE_RET tuya_ipc_ss_delete_events(IN UINT_T pb_idx, SS_FILE_INFO_T* sec, int sec_count);


/**
 * @brief set write data mode for audio include
 * 
 * @param[in] audio_enable: TRUE/FALSE
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_set_data_mode(IN CONST BOOL_T audio_enable);

OPERATE_RET tuya_ipc_ss_set_encrypt_mode(IN CONST BOOL_T isEncryptEnable);

/**
 * @brief get debug info
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_get_debug_info(VOID);

/**
 * @brief save one record file into dest dir
 * 
 * @param[in] channel: which channel, 0 for ipc
 * @param[in] file_info: record file info (fragment, also means a event file) 
 * @param[in] dest_dir: dest dir of mp4 file
 * @param[out] file_name: buffer of file name of mp4
 * @param[in] name_len: file name buffer length
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ss_save_album_mp4(IN INT_T channel, IN SS_FILE_TIME_TS_T* file_info, IN CHAR_T *dest_dir,
                                               OUT CHAR_T *file_name, IN INT_T name_len);

/**
 * @brief set interval of capture image
 * 
 * @param[in] interval: interval of capture image, seconds
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ss_set_capture_image_interval(INT_T interval);

/**
 * @brief start aov local storage
 * 
 * @param[in] chan stream channel
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_ss_start_aov_local_storage(IN IPC_STREAM_E chan);

/**
 * @brief stop aov local storage
 * 
 * @param[in] chan stream channel
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_ss_stop_aov_local_storage(IN IPC_STREAM_E chan);

/**
 * @brief get aov local storage write status
 * 
 * @param[in] chan stream channel
 * @return STORAGE_AOV_STATUS_E
 */
STORAGE_AOV_STATUS_E tuya_ipc_ss_get_aov_storage_status(IN IPC_STREAM_E chan);

/**
 * @brief get earliest fragment utc time
 * 
 * @param[out] utc_time_s utc time 
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_ss_get_earliest_fragment_time_info(OUT UINT_T  * utc_time_s);


#ifdef __cplusplus
}
#endif

#endif
