#ifndef __TUYA_IPC_MEDIA_GATEWAY_H__
#define __TUYA_IPC_MEDIA_GATEWAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tal_mutex.h"
#include "tal_thread.h"
#include "tuya_ipc_media.h"

#define PUBLIC
#define PRIVATE

#define MAX_MEDIA_GATEWAY_ITEM_SIZE    8
#define MEDIA_GATEWAY_USERNAME_LEN     64
#define MEDIA_GATEWAY_PASSWORD_LEN     64
#define MEDIA_GATEWAY_MAX_URL_LEN      256
#define MEDIA_GATEWAY_MAX_SERVER_INFO  2

/**
 * @enum    MEDIA_GATEWAY_ITEM_STATUS_E
 * @brief   media gateway item status
 */
typedef enum {
    MEDIA_GATEWAY_ITEM_STATUS_IDLE = 0,
    MEDIA_GATEWAY_ITEM_STATUS_ONGOING
} MEDIA_GATEWAY_ITEM_STATUS_E;

/**
 * @struct  MEDIA_GATEWAY_SERVER_INFO_T
 * @brief   server info
 */
typedef struct {
    CHAR_T address[MEDIA_GATEWAY_MAX_URL_LEN];
    UINT_T tcp_port;
    TUYA_IP_ADDR_T ip_addr;
} MEDIA_GATEWAY_SERVER_INFO_T;

/**
 * @struct  MEDIA_GATEWAY_SESSION_T
 * @brief   session
 */
typedef struct {
    CHAR_T username[MEDIA_GATEWAY_USERNAME_LEN];
    CHAR_T password[MEDIA_GATEWAY_PASSWORD_LEN];
    INT_T  fd;
    INT_T  expire_time;
    BOOL_T connect_disable;
    TIME_T delay_close_time;
    UINT64_T link_id;
} MEDIA_GATEWAY_SESSION_T;

/**
 * @struct  MEDIA_GATEWAY_ITEM_T
 * @brief   media gateway upload item
 */
typedef struct {
    CHAR_T biz_tag[64];
    CHAR_T session_id[32];
    UINT_T biz_code;
    INT_T file_idx;
    BOOL_T is_last;
    BOOL_T is_stream;
    UINT_T stream_type;
    UINT64_T start_time;
    UINT_T event_time;
    INT_T security_level;
    UINT64_T trigger_time_ms;
    MEDIA_GATEWAY_ITEM_STATUS_E status;
} MEDIA_GATEWAY_ITEM_T;

typedef struct {
    BOOL_T started;
    BOOL_T change_session_id;
    BOOL_T auto_restart;
    CHAR_T session_id[32];
    CHAR_T new_session_id[32];
    CHAR_T biz_tag[64];
    UINT_T biz_code;
    UINT_T stream_type;
    INT_T  security_level;
} MEDIA_GATEWAY_STREAM_EVENT_T;

typedef struct {
PRIVATE
    BOOL_T        inited;
    BOOL_T        paused;
    THREAD_HANDLE thrd_hdl;
    int           thread_exit;

    CHAR_T        localKey[LOCAL_KEY_LEN+1];

    MUTEX_HANDLE  mutex;
    INT_T         status;
    MEDIA_GATEWAY_ITEM_T items[MAX_MEDIA_GATEWAY_ITEM_SIZE];
    MEDIA_GATEWAY_STREAM_EVENT_T stream_events[MAX_MEDIA_GATEWAY_ITEM_SIZE];

    TIME_T        config_next_update_time;       // 配置下一次更新的时间点

    INT_T         server_idx;                    // 当前使用的服务器地址下标
    INT_T         server_cnt;                    // 服务器地址数
    INT_T         delay_time_idx;
    MEDIA_GATEWAY_SERVER_INFO_T server_info[MEDIA_GATEWAY_MAX_SERVER_INFO];     // 服务器地址
    MEDIA_GATEWAY_SESSION_T session;             // 连接会话

    UINT8_T      *send_buffer;
    INT_T         send_buffer_len;
    UINT8_T      *media_buffer;
    INT_T         media_buffer_len;
    UINT8_T      *recv_buffer;
    INT_T         recv_buffer_len;

PUBLIC
    OPERATE_RET (*set_media_setting)(IPC_MEDIA_INFO_T *media_setting);
    OPERATE_RET (*fill_skills)(VOID);
    OPERATE_RET (*pause)(VOID);
    OPERATE_RET (*resume)(VOID);
    OPERATE_RET (*add_item)(CHAR_T *biz_tag, CHAR_T *session_id, UINT_T biz_code, INT_T file_idx, BOOL_T is_last, INT_T security_level);
    OPERATE_RET (*stream_start)(CHAR_T *biz_tag, CHAR_T *session_id, UINT_T biz_code, UINT_T stream_type, INT_T pre_record_time, BOOL_T auto_restart, UINT64_T *frame_timestamp_ms, INT_T security_level);
    OPERATE_RET (*stream_stop)(CHAR_T *session_id, UINT_T stream_type, UINT_T event_time);
    // 获取云存储上传状态，TRUE-正在上传
    BOOL_T      (*get_stream_status)(VOID);
    // 事件和云存储上传状态，TRUE-正在上传
    BOOL_T      (*get_status)(VOID);
} MEDIA_GATEWAY_T;

/**
 * @brief        get media gateway instance
 * @return       MEDIA_GATEWAY_T*
 * - NULL        init failed
 */
MEDIA_GATEWAY_T* tuya_ipc_media_gateway_get_instance(VOID);

/**
 * @brief        uninit media gateway
 * @return       VOID
 */
VOID tuya_ipc_media_gateway_uninit(VOID);

#ifdef __cplusplus
}
#endif

#endif
