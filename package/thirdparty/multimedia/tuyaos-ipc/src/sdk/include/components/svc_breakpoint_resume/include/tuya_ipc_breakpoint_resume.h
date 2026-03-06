/*********************************************************************************
  *Copyright(C),2018, 涂鸦科技 www.tuya.comm
  *FileName:    tuya_ipc_breakpoint_resume.h
**********************************************************************************/

#ifndef __TUYA_IPC_BREAKPOINT_RESUME_H__
#define __TUYA_IPC_BREAKPOINT_RESUME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tuya_ipc_media.h"
#include "tuya_ipc_event.h"
#include "tuya_ipc_notify.h"

#define BREAKPOINT_RESUME_BASE_PATH_LEN                 128

typedef INT_T(*BREAKPOINT_RESUME_GET_OTHER_CLIENT_USER_NUM_CB)(VOID);

typedef struct __breakpoint_database_w_event_info
{
    UINT64_T event_timestamp_ms;                                            // 事件时间
    TUYA_ALARM_TYPE_E event_type;                                           // 事件类型
    UINT_T event_duration_ms;                                               // 事件持续时长(录像时长)
    UINT_T video_mode;                                                      // 录像模式 0:持续存储 1:事件存储

    UINT_T unit_amount;                                                     // 消息媒体文件数量
    NOTIFICATION_UNIT_T* p_media_unit;                                      // 消息媒体文件(在事件中心展示的图片/视频留言等)

    INT_T cloud_video_flag;                                                 // 云存录像标记
    INT_T cloud_video_index;                                                // 云存录像序号
    CHAR_T cloud_video_path[BREAKPOINT_RESUME_BASE_PATH_LEN];               // 云存录像缓存路径     
} BREAKPOINT_RESUME_EVENT_INFO_T;           // 事件信息

typedef enum {
    BREAKPOINT_RESUME_UPLOAD_SPEED_CTRL_ON = 0,                             // 控速上传开
    BREAKPOINT_RESUME_UPLOAD_SPEED_CTRL_OFF,                                // 控速上传关
} BREAKPOINT_RESUME_UPLOAD_SPEED_CTRL_E;

// Check whether the start conditions for uploading data are met
typedef OPERATE_RET (*BREAKPOINT_RESUME_CHECK_UPLOAD_CONDITIONS_CB)(VOID);

// whether network status is normal
typedef OPERATE_RET (*BREAKPOINT_RESUME_IS_NETWORK_NORMAL_CB)(VOID);

// Upload cloud storage
typedef OPERATE_RET (*BREAKPOINT_RESUME_UPLOAD_CLOUD_DATA_CB)(BREAKPOINT_RESUME_EVENT_INFO_T* info);

typedef struct {
    INT_T breakpoint_resume_enable;                         ///< 1: breakpoint resume enable
    INT_T pic_event_enable;                                 ///< 1: pic & event resume enable
    CHAR_T folder_path[BREAKPOINT_RESUME_BASE_PATH_LEN];    ///< Node of storage card is mounted
    FLOAT_T max_record_hours;                               ///< Maximum record hours for breakpoint events(0:default 12 hours)

    INT_T data_file_need_backup;                            ///< Whether to enable a backup policy
    UINT_T max_record_num;                                  ///< max number of offline records(0:default 4000 records)
    INT_T enable_intelli_resume;                            ///< According to the upload network status intelligent control of upload
    UINT_T max_time_for_upload_to_cloud;                    ///< max time for upload single video to cloud(0:default 10 seconds)

    BREAKPOINT_RESUME_UPLOAD_SPEED_CTRL_E speed_control_upload_enable;     ///< Speed control upload

    BREAKPOINT_RESUME_CHECK_UPLOAD_CONDITIONS_CB check_upload_cb;          ///< Check whether the start conditions for uploading data are met; 0:ok/other:not ok
    BREAKPOINT_RESUME_IS_NETWORK_NORMAL_CB network_normal_cb;              ///< whether network status is normal; 0:ok/other:not ok

    BREAKPOINT_RESUME_UPLOAD_CLOUD_DATA_CB cloud_data_upload_cb;           ///< Upload cloud storage
    BREAKPOINT_RESUME_GET_OTHER_CLIENT_USER_NUM_CB get_other_client_num_cb;
} TUYA_IPC_BREAKPOINT_RESUME_T;

extern VOID tuya_ipc_sd_get_capacity_by_mount_path(CONST CHAR_T* mount_path, UINT_T *p_total, UINT_T *p_used, UINT_T *p_free);

/**
 * \brief initialize tuya ipc breakpoint resume
 * \param[in] pbreakpoint_resume_var: init para
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_breakpoint_resume_init(IN TUYA_IPC_BREAKPOINT_RESUME_T *pbreakpoint_resume_var);

/**
 * \brief uinit tuya ipc breakpoint resume
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_breakpoint_resume_uninit();

BOOL_T tuya_ipc_breakpoint_resume_init_ok();

/**
 * @brief Add events to the storage datasheet
 * 
 * @param[in] event_info event info
 * @return OPERATE_RET OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_resume_add_event(BREAKPOINT_RESUME_EVENT_INFO_T* event_info);

/**
 * @brief set event upload status
 * 
 * @param[in] event_info event info
 * @param[in] upload_ok_flag TRUE:uplaod ok/FALSE: uplaod fail
 * @return OPERATE_RET OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_resume_set_event_upload_status(BREAKPOINT_RESUME_EVENT_INFO_T* event_info, INT_T upload_ok_flag);

/**
 * @brief set mq status
 * 
 * @param[in] online online status 0/1 : offline/online
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_set_mq_status(INT_T online);

/**
 * @brief suspen breakpoint resume service
 * 
 * @param
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_resume_suspend();

/**
 * @brief recovery breakpoint resume service
 * 
 * @param
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_resume_recovery();

/**
 * @brief set Upload speed control switch
 * 
 * @param[in] upload_speed_ctrl upload speed switch
 * @return OPRT_OK on success. Others on error
 */
OPERATE_RET tuya_ipc_breakpoint_resume_set_upload_control(BREAKPOINT_RESUME_UPLOAD_SPEED_CTRL_E upload_speed_ctrl);

/**
 * @brief is breakpoint_resume task suspend
 * 
 * @param
 * @return TRUE/FALSE : running/not
 */
BOOL_T tuya_ipc_breakpoint_resume_is_suspend();


OPERATE_RET tuya_ipc_breakpoint_resume_set_work_sleep_time(UINT_T sleep_time_ms);


#ifdef __cplusplus
}
#endif

#endif
