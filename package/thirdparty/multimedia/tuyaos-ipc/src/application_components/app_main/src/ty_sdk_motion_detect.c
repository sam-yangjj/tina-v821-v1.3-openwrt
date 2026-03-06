/*********************************************************************************
*Copyright(C),2015-2020,
*TUYA
*www.tuya.comm
*FileName:    tuya_ipc_motion_detect_demo
**********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "tal_time_service.h"
#include "utilities/uni_log.h"
#include "tuya_ipc_cloud_storage.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_event.h"

#include "ty_sdk_common.h"
#include "tal_system.h"
#include "tal_thread.h"

//According to different chip platforms, users need to implement whether there is motion alarm in the current frame.
static int fake_md_status = 0;  //1:motion_notify 2:human_notify
static int fake_sound_status = 0;  //1:abnormal sound
static pthread_mutex_t motion_status_mutex = PTHREAD_MUTEX_INITIALIZER;
static int motion_alarm_is_triggerd = FALSE;
static int sound_alarm_is_triggerd = FALSE;

VOID IPC_APP_set_motion_status(int status)
{
    if (fake_md_status == status) {
        return;
    }
    pthread_mutex_lock(&motion_status_mutex);
    fake_md_status = status;
    pthread_mutex_unlock(&motion_status_mutex);
}

static int get_motion_status()
{
    return fake_md_status;
}

VOID IPC_APP_set_sound_status(int status)
{
    fake_sound_status = status;
}

static int get_sound_status()
{
    return fake_sound_status;
}

#define MAX_SNAPSHOT_BUFFER_SIZE            (100*1024)  //in BYTE

typedef struct {
    RULE_RESULT_STATE_E state;
    TIME_T trigger_time;
    unsigned int msg_time;
    char result_list[256]; //contains detection results of images such as birds, squirrels, etc
    char msg[640]; //contains URL information
} CLOUD_AI_RESULT_T;

typedef struct {
    RULE_RESULT_STATE_E state;
    TIME_T trigger_time;
    unsigned int msg_time;
    char msg[256]; //contains URL information
} CLOUD_VIDEO_MSG_T;

typedef struct {
    BOOL_T valid;
    TIME_T trigger_time;
    int sn;
    CLOUD_AI_RESULT_T ai_result;
    CLOUD_VIDEO_MSG_T video_msg;
} TRIGEER_EVENT_T;

#define MAX_TRIGGER_EVENT_NUMBER           8
#define MAX_WAIT_RESULT_TIME               10

static TRIGEER_EVENT_T s_trigger_event[MAX_TRIGGER_EVENT_NUMBER];

static int __find_trigger_event_by_sn(int sn)
{
    int i = 0;;
    for(i = 0 ; i < MAX_TRIGGER_EVENT_NUMBER; i++) {
        if (s_trigger_event[i].valid && sn == s_trigger_event[i].sn) {
            break;
        }
    }
    if(MAX_TRIGGER_EVENT_NUMBER == i) {
        return -1;
    } else {
        return i;
    }
}

static void __save_sn(int sn)
{
    int i;
    for(i = 0 ; i < MAX_TRIGGER_EVENT_NUMBER; i++) {
        if (0 == s_trigger_event[i].valid) {
            s_trigger_event[i].sn = sn;
            s_trigger_event[i].trigger_time = tal_time_get_posix();
            s_trigger_event[i].valid = 1;
            break;
        }
    }
    if(MAX_TRIGGER_EVENT_NUMBER == i) {
        PR_ERR("array is full\n");
    }
}

static VOID __process_cloud_msg()
{
    int i = 0, snapshot_size, md_enable = 0, ret = 0;
    char *snapshot_buf = NULL;
    TRIGEER_EVENT_T *event;
    for(i = 0; i < MAX_TRIGGER_EVENT_NUMBER; i++){
        event = &s_trigger_event[i];
        if(event->valid){ //event triggered
            if(RULE_NOT_SUPPORT == event->ai_result.state){
                if(RULE_NOT_SUPPORT == event->video_msg.state){
                    PR_ERR("not support ai detect and video msg");
                    memset(event, 0, sizeof(TRIGEER_EVENT_T));
                }else if(RULE_WAIT_RESULT == event->video_msg.state){
                    if(tal_time_get_posix() > event->trigger_time + MAX_WAIT_RESULT_TIME){
                        PR_ERR("not support ai detect and wait video msg timeout");
                        memset(event, 0, sizeof(TRIGEER_EVENT_T));
                    }
                }else if(RULE_GOT_RESULT == event->video_msg.state){
                    PR_DEBUG("cloud result only contain video msg");
                    snapshot_size = 150 * 1024;
                    snapshot_buf = (char *)malloc(snapshot_size);
                    ret = IPC_APP_get_snapshot(snapshot_buf, &snapshot_size);
                    if(ret == 0) {
                        md_enable = IPC_APP_get_alarm_function_onoff();
                        tuya_ipc_notify_alarm(snapshot_buf, snapshot_size, NOTIFICATION_NAME_MOTION, md_enable, event->video_msg.msg);
                    }
                    free(snapshot_buf);
                    memset(event, 0, sizeof(TRIGEER_EVENT_T));
                }
            } else if(RULE_WAIT_RESULT == event->ai_result.state){
                if(tal_time_get_posix() > event->trigger_time + MAX_WAIT_RESULT_TIME){
                    if(RULE_GOT_RESULT == event->video_msg.state){
                        PR_DEBUG("cloud result only contain video msg");
                        snapshot_size = 150 * 1024;
                        snapshot_buf = (char *)malloc(snapshot_size);
                        ret = IPC_APP_get_snapshot(snapshot_buf, &snapshot_size);
                        if(ret == 0) {
                            md_enable = IPC_APP_get_alarm_function_onoff();
                            tuya_ipc_notify_alarm(snapshot_buf, snapshot_size, NOTIFICATION_NAME_MOTION, md_enable, event->video_msg.msg);
                        }
                        free(snapshot_buf);
                        memset(event, 0, sizeof(TRIGEER_EVENT_T));
                    }else{
                        PR_DEBUG("wait ai result and video msg timeout");
                        memset(event, 0, sizeof(TRIGEER_EVENT_T));
                    }
                }
            } else if(RULE_GOT_RESULT == event->ai_result.state){
                if(RULE_NOT_SUPPORT == event->video_msg.state){
                    PR_DEBUG("get ai result but not support video msg");
                    // Detect AI targets (such as ai bird) and then report intelligent messages
                    md_enable = IPC_APP_get_alarm_function_onoff();
                    if (event->ai_result.result_list && strstr(event->ai_result.result_list, "ai_bird")) { //have AI target:ai_bird
                        tuya_ipc_rule_report_ai_msg(event->ai_result.msg, md_enable);
                    } else if (event->ai_result.result_list && 0 == strncmp(event->ai_result.result_list, "[]", 2)){ // No AI targets detected
                        tuya_ipc_rule_report_event(NOTIFICATION_NAME_MOTION, event->ai_result.msg, md_enable);
                    }
                    memset(event, 0, sizeof(TRIGEER_EVENT_T));
                }else if(RULE_WAIT_RESULT == event->video_msg.state){
                    if(tal_time_get_posix() >event->trigger_time + MAX_WAIT_RESULT_TIME){
                        PR_DEBUG("get ai result but wait video msg timeout");
                        // Detect AI targets (such as ai bird) and then report intelligent messages
                        md_enable = IPC_APP_get_alarm_function_onoff();
                        if (event->ai_result.result_list && strstr(event->ai_result.result_list, "ai_bird")) { //have AI target:ai_bird
                            tuya_ipc_rule_report_ai_msg(event->ai_result.msg, md_enable);
                        } else if (event->ai_result.result_list && 0 == strncmp(event->ai_result.result_list, "[]", 2)){ // No AI targets detected
                            tuya_ipc_rule_report_event(NOTIFICATION_NAME_MOTION, event->ai_result.msg, md_enable);
                        }
                        memset(event, 0, sizeof(TRIGEER_EVENT_T));
                    }
                }else if(RULE_GOT_RESULT == event->video_msg.state){
                    PR_DEBUG("get ai result and video msg");
                    // Detect AI targets (such as ai bird) and then report intelligent messages
                    md_enable = IPC_APP_get_alarm_function_onoff();
                    if (event->ai_result.result_list && strstr(event->ai_result.result_list, "ai_bird")) { //have AI target:ai_bird
                        tuya_ipc_rule_report_ai_msg_with_video_msg(event->ai_result.msg, event->video_msg.msg, md_enable);
                    } else if (event->ai_result.result_list && 0 == strncmp(event->ai_result.result_list, "[]", 2)){ // No AI targets detected
                        tuya_ipc_rule_report_event_with_video_msg(NOTIFICATION_NAME_MOTION, event->ai_result.msg, event->video_msg.msg, md_enable);
                    }
                    memset(event, 0, sizeof(TRIGEER_EVENT_T));
                }
            }
        }
    }
}

#define JPEG_BUFFER_SIZE  150*1024

VOID *thread_md_proc(VOID *arg)
{
    int motion_flag = 0;
    int snap_size = JPEG_BUFFER_SIZE;
    char *snap_addr = NULL;
    int md_enable = 0;
    int ret = 0;
    TIME_T current_time;
    TIME_T last_md_time;
    unsigned int sn;

    snap_addr = (char *)malloc(JPEG_BUFFER_SIZE);
    if (!snap_addr) {
        PR_ERR("snap_addr malloc fail!");
        return NULL;
    }

    while (1) {
        tal_system_sleep(100);
        tuya_ipc_get_utc_time(&current_time);
#if (ENABLE_CLOUD_RULE == 1)
        __process_cloud_msg();
#endif
        motion_flag = get_motion_status();
        if (motion_flag) {
            last_md_time = current_time;
            if (!motion_alarm_is_triggerd) {
                motion_alarm_is_triggerd = TRUE;
                PR_INFO("motion_alarm_is_triggerd TRUE,motion_flag:%d", motion_flag);
                //start Local SD Card Event Storage and Cloud Storage Events
                if (E_STORAGE_START != tuya_ipc_ss_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_SD_STORAGE);
                }
                if (true != tuya_ipc_cloud_storage_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_CLOUD_STORAGE);
                }

#if (ENABLE_CLOUD_RULE == 1)
                /*NOTE:
                ONE：The prerequisite for using this feature is to have enabled cloud AI detection value-added services or video message value-added services
                TWO：Cloud results obtained through TUYA_APP_ai_result_cb and TUYA_APP_video_msg_cb
                */
                tuya_ipc_rule_trigger_event(NOTIFICATION_NAME_MOTION, &sn);
                __save_sn(sn);
#else

                // md_enable is TRUE, upload message to message center
                md_enable = IPC_APP_get_alarm_function_onoff();
                if (md_enable == true) {
                    //延迟抓拍
                    usleep(100 * 1000);
                    snap_size = JPEG_BUFFER_SIZE;
                    memset(snap_addr, 0, snap_size);
                    PR_INFO("motion_flag:%d", motion_flag);
                    IPC_APP_get_snapshot(snap_addr,&snap_size);
                    if (snap_size > 0) {
                        if (motion_flag == 1) {
                            tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_MOTION, md_enable, NULL);
                        } else if (motion_flag == 2) {
                            tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_HUMAN, md_enable, NULL);
                        }
                    }
// #ifdef IPC_DUAL
//                     usleep(200*1000);
//                     snap_size = MAX_SNAPSHOT_BUFFER_SIZE;
//                     memset(snap_addr, 0, sizeof(snap_addr));
//                     IPC_APP_get_snapshot_dual_gun(snap_addr,&snap_size);
//                     if(snap_size > 0)
//                     {
//                         if (motion_flag == 1)
//                         {
//                             tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_MOTION, md_enable, NULL);
//                         }
//                         else if (motion_flag == 2)
//                         {
//                             tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_HUMAN, md_enable, NULL);
//                         }
//                     }
// #endif //end IPC_DUAL
                }
#endif
                /*NOTE:
                ONE：Considering the real-time performance of push and storage, the above interfaces can be executed asynchronously in different tasks.
                TWO：When event cloud storage is turned on, it will automatically stop beyond the maximum event time in SDK.
                THREE:If you need to maintain storage for too long without losing it, you can use the interface (tuya_ipc_ss_get_status and tuya_ipc_cloud_storage_get_event_status).
                    to monitor whether there are stop event videos in SDK and choose time to restart new events
                */
            } else {
                //Storage interruption caused by maximum duration of internal events, restart new events
                if (SS_WRITE_MODE_EVENT == tuya_ipc_ss_get_write_mode() && E_STORAGE_STOP == tuya_ipc_ss_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_SD_STORAGE);
                }

                if (ClOUD_STORAGE_TYPE_EVENT == tuya_ipc_cloud_storage_get_store_mode()
                    && FALSE == tuya_ipc_cloud_storage_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_CLOUD_STORAGE);
                }
            }
        } else {
            //No motion detect for more than 10 seconds, stop the event
            if (current_time - last_md_time > 10 && motion_alarm_is_triggerd) {
                motion_alarm_is_triggerd = FALSE;
                PR_INFO("motion_alarm_is_triggerd FALSE");
                if (sound_alarm_is_triggerd == FALSE && motion_alarm_is_triggerd == FALSE) {
                    PR_INFO("stop SD_STORAGE and CLOUD_STORAGE");
                    tuya_ipc_stop_storage(E_ALARM_SD_STORAGE);
                    tuya_ipc_stop_storage(E_ALARM_CLOUD_STORAGE);
                }
            }
        }
    }
    if (!snap_addr) {
        free(snap_addr);
        snap_addr = NULL;
    }
    return NULL;
}

VOID *thread_snd_proc(VOID *arg)
{
    int sound_flag = 0;
    int snap_size = JPEG_BUFFER_SIZE;
    char *snap_addr = NULL;
    int md_enable = 0;
    TIME_T current_time;
    TIME_T last_md_time;

    snap_addr = (char *)malloc(JPEG_BUFFER_SIZE);
    if (!snap_addr) {
        PR_ERR("snap_addr malloc fail!");
        return NULL;
    }

    while (1) {
        tal_system_sleep(100);
        tuya_ipc_get_utc_time(&current_time);
        sound_flag = get_sound_status();
        if (sound_flag) {
            last_md_time = current_time;
            if (!sound_alarm_is_triggerd) {
                sound_alarm_is_triggerd = TRUE;
                PR_INFO("sound_alarm_is_triggerd TURE");
                //start Local SD Card Event Storage and Cloud Storage Events
                if (E_STORAGE_START != tuya_ipc_ss_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_SD_STORAGE);
                }
                if (true != tuya_ipc_cloud_storage_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_CLOUD_STORAGE);
                }

                snap_size = JPEG_BUFFER_SIZE;
                memset(snap_addr, 0, snap_size);
                IPC_APP_get_snapshot(snap_addr, &snap_size);
                if (snap_size > 0) {
                    tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_ABNORMAL_SOUND, true, NULL);
                }
                // #ifdef IPC_DUAL
                //                 usleep(200*1000);
                //                 snap_size = MAX_SNAPSHOT_BUFFER_SIZE;
                //                 memset(snap_addr, 0, sizeof(snap_addr));
                //                 IPC_APP_get_snapshot_dual_gun(snap_addr,&snap_size);
                //                 if(snap_size > 0)
                //                 {
                //                     tuya_ipc_notify_alarm(snap_addr, snap_size, NOTIFICATION_NAME_ABNORMAL_SOUND, true, NULL);
                //                 }
                // #endif //end IPC_DUAL
                        /*NOTE:
                        ONE：Considering the real-time performance of push and storage, the above interfaces can be executed asynchronously in different tasks.
                        TWO：When event cloud storage is turned on, it will automatically stop beyond the maximum event time in SDK.
                        THREE:If you need to maintain storage for too long without losing it, you can use the interface (tuya_ipc_ss_get_status and tuya_ipc_cloud_storage_get_event_status).
                            to monitor whether there are stop event videos in SDK and choose time to restart new events
                        */
            } else {
                //Storage interruption caused by maximum duration of internal events, restart new events
                if (SS_WRITE_MODE_EVENT == tuya_ipc_ss_get_write_mode() && E_STORAGE_STOP == tuya_ipc_ss_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_SD_STORAGE);
                }

                if (ClOUD_STORAGE_TYPE_EVENT == tuya_ipc_cloud_storage_get_store_mode()
                    && FALSE == tuya_ipc_cloud_storage_get_status()) {
                    tuya_ipc_start_storage(E_ALARM_CLOUD_STORAGE);
                }
            }
        } else {
            //No motion detect for more than 10 seconds, stop the event
            if (current_time - last_md_time > 10 && sound_alarm_is_triggerd) {
                sound_alarm_is_triggerd = FALSE;
                PR_INFO("sound_alarm_is_triggerd FALSE");
                if (sound_alarm_is_triggerd == FALSE && motion_alarm_is_triggerd == FALSE) {
                    PR_INFO("stop SD_STORAGE and CLOUD_STORAGE");
                    tuya_ipc_stop_storage(E_ALARM_SD_STORAGE);
                    tuya_ipc_stop_storage(E_ALARM_CLOUD_STORAGE);
                }
            }
        }
    }
    if (!snap_addr) {
        free(snap_addr);
        snap_addr = NULL;
    }
    return NULL;
}

OPERATE_RET TUYA_APP_Enable_Motion_Detect()
{
    THREAD_CFG_T motion_detect_thread_param;
    motion_detect_thread_param.priority = THREAD_PRIO_2;
    motion_detect_thread_param.stackDepth = 512 * 1024;
    motion_detect_thread_param.thrdname = "md_proc";
    THREAD_HANDLE motion_detect_thrd_hdl;
    tal_thread_create_and_start(&motion_detect_thrd_hdl, NULL, NULL, thread_md_proc, NULL, &motion_detect_thread_param);
    return OPRT_OK;
}

OPERATE_RET TUYA_APP_Enable_Sound_Detect()
{
    THREAD_CFG_T sound_detect_thread_param;
    sound_detect_thread_param.priority = THREAD_PRIO_2;
    sound_detect_thread_param.stackDepth = 512 * 1024;
    sound_detect_thread_param.thrdname = "snd_proc";
    THREAD_HANDLE sound_detect_thrd_hdl;
    tal_thread_create_and_start(&sound_detect_thrd_hdl, NULL, NULL, thread_snd_proc, NULL, &sound_detect_thread_param);
    return OPRT_OK;
}

/**
* @brief        The developer first calls "tuya_ipc_rule_trigger_event" to trigger an event, 
                and then the AI detection results will be notified to the upper layer through this callback
* @param[out]   result_list: ai result list from cloud, json string, such as ["ai_package","ai_car","ai_bird","ai_fire"]
* @param[out]   result_msg:  message from cloud, such as {"type":"xxx","ai_bucket":"xxx","ai_path":"xxx","ai_expire":"xxx"}
* @param[out]   msg_time:    time of cloud message
* @param[out]   type:        message type.Specifically refer to RULE_RESULT_STATE_E
* @return       VOID
*/
VOID TUYA_APP_ai_result_cb(OUT CHAR_T *result_list, OUT CHAR_T *result_msg, OUT UINT_T msg_time, OUT UINT_T sn, OUT RULE_RESULT_STATE_E state)
{
    int index = 0;
    index = __find_trigger_event_by_sn(sn);
    if(index == -1){
        PR_ERR("not find sn\n");
        return;
    }
    s_trigger_event[index].ai_result.msg_time = msg_time;
    s_trigger_event[index].ai_result.state = state;
    memset(s_trigger_event[index].ai_result.msg, 0, 640);
    memcpy(s_trigger_event[index].ai_result.msg, result_msg, 640);
    memset(s_trigger_event[index].ai_result.result_list, 0, 256);
    memcpy(s_trigger_event[index].ai_result.result_list, result_list, 256);
    return;
}

/**
* @brief        The developer first calls "tuya_ipc_rule_trigger_event" to trigger an event, 
                and then the AI detection results and video address will be notified to the upper layer through this callback
* @param[out]   video_msg:   message from cloud, such as {"type":"xxx","store_bucket":"xxx","store_path":"xxx","store_expire":"xxx"}
* @param[out]   msg_time:    time of cloud message
* @param[out]   type:        message type.Specifically refer to RULE_RESULT_STATE_E
* @return       VOID
*/
VOID TUYA_APP_video_msg_cb(OUT CHAR_T *video_msg, OUT UINT_T msg_time, OUT UINT_T sn, OUT RULE_RESULT_STATE_E state)
{
    int index = 0;
    index = __find_trigger_event_by_sn(sn);
    if(index == -1){
        PR_ERR("not find sn\n");
        return;
    }
    s_trigger_event[index].video_msg.msg_time = msg_time;
    s_trigger_event[index].video_msg.state = state;
    memset(s_trigger_event[index].video_msg.msg, 0, 256);
    memcpy(s_trigger_event[index].video_msg.msg, video_msg, 256);
    return;
}

