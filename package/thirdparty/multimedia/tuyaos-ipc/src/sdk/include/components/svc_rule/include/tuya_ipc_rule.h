#ifndef __TUYA_IPC_VOICE_H__
#define __TUYA_IPC_VOICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/**
 * @enum    RULE_STORE_MODE_E
 * @brief   rule store mode
 */
typedef enum {
    RULE_STORE_MODE_CONTINUE = 0,     // continuous cloud storage
    RULE_STORE_MODE_EVENT,            // event cloud storage
    RULE_STORE_MODE_INVALID           // invalid order
} RULE_STORE_MODE_E;

/**
 * @enum    RULE_RESULT_STATE_E
 * @brief   rule result state
 */
typedef enum {
    RULE_WAIT_RESULT= 0x00,            // waiting for cloud receipt
    RULE_GOT_RESULT,                   // obtain cloud based results
    RULE_NOT_SUPPORT                   // not supported this service
} RULE_RESULT_STATE_E;

/**
 * @enum RULE_STREAMING_TYPE_E
 * @brief streaming type
 */
typedef enum
{
    RULE_STREAMING_TYPE_2_0 = 0,    // wukong2.0推流通道
    RULE_STREAMING_TYPE_1_0 = 1,    // wukong1.0推流通道
} RULE_STREAMING_TYPE_E;

/**
 * @brief         callback of ai result
 * @param[out]    result_list ai result list from cloud, json string, such as ["ai_package","xxx","xxx"]
 * @param[out]    result_msg      ai result message from cloud
 * @param[out]    msg_time        message time
 * @param[out]    sn              event identification. used to distinguish different events
 * @param[out]    state           result state. Specifically refer to TRANSFER_RESULT_STAT_E
 * @return        VOID
 */
typedef VOID (*RULE_AI_RESULT_CB)(OUT CHAR_T *result_list, OUT CHAR_T *result_msg, OUT UINT_T msg_time, OUT UINT_T sn, OUT RULE_RESULT_STATE_E state);

/**
 * @brief         callback of video result
 * @param[out]    video_msg       video message from cloud
 * @param[out]    msg_time        message time
 * @param[out]    sn              event identification. used to distinguish different events
 * @param[out]    state           result state. Specifically refer to TRANSFER_RESULT_STAT_E
 * @return        VOID
 */
typedef VOID (*RULE_VIDEO_MSG_CB)(OUT CHAR_T *video_msg, OUT UINT_T msg_time, OUT UINT_T sn, OUT RULE_RESULT_STATE_E state);

/**
 * @brief        callback of transferring cmd id to cmd string
 * @param[in]    id event id
 * @param[inout] cmd_buf cmd string buffer
 * @param[in]    cmd_buf_len length of cmd_buf
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
typedef OPERATE_RET (*RULE_TRANSFER_CMD_CB)(INT_T id, CHAR_T *cmd_buf, INT_T cmd_buf_len);

/**
 * @brief        rule module init
 * @param[in]    result_cb callback of cloud result
 * @param[in]    video_msg_cb  callback of video ms
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_init(IN RULE_AI_RESULT_CB ai_result_cb, IN RULE_VIDEO_MSG_CB video_msg_cb);

/**
 * @brief        rule module uninit
 * @return       VOID
 */
VOID tuya_ipc_rule_uninit(VOID);

/**
 * @brief        trigger event for rule module
 * @param[in]    event refer to NOTIFICATION_NAME_E
 * @param[out]   sn  event identification. used to distinguish different events
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_trigger_event(IN INT_T event, OUT UINT_T *sn);

/**
 * @brief        report ai message
 * @param[in]    ai_result     ai result
 * @param[in]    is_notify  notify to message center if TRUE
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_report_ai_msg(IN CHAR_T *ai_result, IN BOOL_T is_notify);

/**
 * @brief        report ai message and video message
 * @param[in]    ai_result  ai result
 * @param[in]    video_msg  video message
 * @param[in]    is_notify  notify to message center if TRUE
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_report_ai_msg_with_video_msg(IN CHAR_T *ai_result, IN CHAR_T *video_msg, IN BOOL_T is_notify);

/**
 * @brief        report expel squirrel message
 * @param[in]    ai_result      ai result
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_report_expel_squirrel(IN CHAR_T *ai_result);

/**
 * @brief        report event if ai result is not detected
 * @param[in]    event      refer to NOTIFICATION_NAME_E
 * @param[in]    ai_result  ai result
 * @param[in]    is_notify  notify to message center if TRUE
 */
OPERATE_RET tuya_ipc_rule_report_event(IN INT_T event, IN CHAR_T *ai_result, IN BOOL_T is_notify);

/**
 * @brief        report event with video msg if ai result is not detected
 * @param[in]    event      refer to NOTIFICATION_NAME_E
 * @param[in]    ai_result  ai result
 * @param[in]    video_msg  video message
 * @param[in]    is_notify  notify to message center if TRUE
 */
OPERATE_RET tuya_ipc_rule_report_event_with_video_msg(IN INT_T event, IN CHAR_T *ai_result, IN CHAR_T *video_msg, IN BOOL_T is_notify);

/**
 * @brief        check rule config is ready
 * @return       BOOL_T
 * - TRUE        rule config is ready
 * - FALSE       rule config is not ready
 */
BOOL_T tuya_ipc_rule_config_ready(VOID);

/**
 * @brief        get store config
 * @param[out]   p_store_mode store mode
 * @param[out]   p_video_time video longest time
 * @param[out]   p_count_left count left today
 * @param[out]   p_silence_interval silence interval
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_get_store_config(OUT RULE_STORE_MODE_E *p_store_mode, OUT UINT_T *p_video_time, OUT UINT_T *p_count_left, OUT UINT_T *p_silence_interval);

/**
 * @brief        start cloud store
 * @param[in]    pre_record_time
 * @return       error code
 * - OPRT_OK     success
 * - Others      fail
 */
OPERATE_RET tuya_ipc_rule_start_cloud_store(IN INT_T pre_record_time);

/**
 * @brief        stop cloud store
 * @return       VOID
 */
VOID tuya_ipc_rule_stop_cloud_store(VOID);

/**
 * @brief        get cloud store status
 * @return       BOOL_T
 * - TRUE        cloud store is ongoing
 * - FALSE       cloud store is finished
 */
BOOL_T tuya_ipc_rule_get_cloud_store_status(VOID);

/**
 * @brief        set cloud store event time
 * @param[in]    event_time
 * @return       VOID
 */
VOID tuya_ipc_rule_set_cloud_store_event_time(IN UINT_T event_time);

/**
 * @brief        get rule status
 * @return       BOOL_T
 * - TRUE        rule is ongoing
 * - FALSE       rule is finished
 */
BOOL_T tuya_ipc_rule_get_status(VOID);

/**
 * @brief        pause rule module
 * @return       VOID
 */
VOID tuya_ipc_rule_pause(VOID);

/**
 * @brief        resume rule module
 * @return       VOID
 */
VOID tuya_ipc_rule_resume(VOID);

/**
 * @brief        set transfer cmd callback
 * @param[in]    cb callback of transferring event id to event string
 * @return       VOID
 */
VOID tuya_ipc_rule_set_transfer_cmd_cb(RULE_TRANSFER_CMD_CB cb);

/**
 * @brief        get pause state
 * @return       pause state
 */
BOOL_T tuya_ipc_rule_get_pause_state(VOID);

/**
 * @brief        get video recording state
 * @return       video recording state
 */
BOOL_T tuya_ipc_rule_video_is_recording(VOID);

/**
 * @brief        get stream duration
 * @param[in]    sn              event identification. used to distinguish different events
 * @return       stream duration. Unit is second,-1 indicates continuous recording
 */
INT_T tuya_ipc_rule_get_stream_duration(IN UINT_T sn);

/**
 * \fn OPERATE_RET tuya_ipc_rule_set_video_msg_pre_record_time
 * \brief set video msg pre record time duration
 * \return OPERATE_RET
 */
OPERATE_RET tuya_ipc_rule_set_video_msg_pre_record_time(INT_T pre_record_time);

/**
 * brief Determine if there is a rule
 * @return       BOOL_T
 * - TRUE        have rules
 * - FALSE       not have rules
 */
BOOL_T tuya_ipc_rule_already_get_rules();


/**
 * brief Stop streaming based on sn
 * @param[in]    sn  event identification. used to distinguish different events
 * @return       OPERATE_RET
 */
OPERATE_RET tuya_ipc_rule_stop_stream_by_sn(IN UINT_T sn);

/**
 * @brief users choose streaming type
 *
 * @return  OPERATE_RET
 */
OPERATE_RET tuya_ipc_rule_set_streaming_type(RULE_STREAMING_TYPE_E streaming_type);

#ifdef __cplusplus
}
#endif

#endif
