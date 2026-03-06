#ifndef _TUYA_IPC_MEDIA_STREAM_EVENT_H_
#define _TUYA_IPC_MEDIA_STREAM_EVENT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"


/**************************enum define***************************************/

// 媒体流相关事件
typedef enum
{
    // 枚举 0~29 留给与硬件相关的事件
    MEDIA_STREAM_NULL = 0,
    MEDIA_STREAM_SPEAKER_START = 1, 
    MEDIA_STREAM_SPEAKER_STOP = 2,
    MEDIA_STREAM_DISPLAY_START = 3,
    MEDIA_STREAM_DISPLAY_STOP = 4,
   
    MEDIA_STREAM_LIVE_VIDEO_START = 30,
    MEDIA_STREAM_LIVE_VIDEO_STOP = 31,
    MEDIA_STREAM_LIVE_AUDIO_START = 32,
    MEDIA_STREAM_LIVE_AUDIO_STOP = 33,
    MEDIA_STREAM_LIVE_VIDEO_CLARITY_SET = 34,
    MEDIA_STREAM_LIVE_VIDEO_CLARITY_QUERY = 35,         /* query clarity informations*/
    MEDIA_STREAM_LIVE_LOAD_ADJUST = 36,
    MEDIA_STREAM_PLAYBACK_LOAD_ADJUST = 37,
    MEDIA_STREAM_PLAYBACK_QUERY_MONTH_SIMPLIFY = 38,    /* query storage info of month  */
    MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS = 39,            /* query storage info of day */
    MEDIA_STREAM_PLAYBACK_START_TS = 40,                /* start playback */
    MEDIA_STREAM_PLAYBACK_PAUSE = 41,                   /* pause playback */
    MEDIA_STREAM_PLAYBACK_RESUME = 42,                  /* resume playback */
    MEDIA_STREAM_PLAYBACK_MUTE = 43,                    /* mute playback */
    MEDIA_STREAM_PLAYBACK_UNMUTE = 44,                  /* unmute playback */
    MEDIA_STREAM_PLAYBACK_STOP = 45,                    /* stop playback */ 
    MEDIA_STREAM_PLAYBACK_SET_SPEED = 46,               /*set playback speed*/
    MEDIA_STREAM_ABILITY_QUERY = 47,                    /* query the alibity of audion video strraming */
    MEDIA_STREAM_DOWNLOAD_START = 48,                   /* start to download */
    MEDIA_STREAM_DOWNLOAD_STOP = 49,                    /* abondoned */
    MEDIA_STREAM_DOWNLOAD_PAUSE = 50,
    MEDIA_STREAM_DOWNLOAD_RESUME = 51,
    MEDIA_STREAM_DOWNLOAD_CANCLE = 52,

    MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS_WITH_ENCRYPT = 53,            /* query storage info of day */
    MEDIA_STREAM_DOWNLOAD_START_WITH_ENCRYPT = 54,

    MEDIA_STREAM_PLAYBACK_QUERY_DAY_V3 = 55,


    /*与互联互通相关*/
    MEDIA_STREAM_LIVE_VIDEO_SEND_START = 60,            //对端请求被拉视频流
    MEDIA_STREAM_LIVE_VIDEO_SEND_STOP = 61,             //对端请求停止被拉视频流
    MEDIA_STREAM_LIVE_AUDIO_SEND_START = 62,            //对端请求被拉音频流
    MEDIA_STREAM_LIVE_AUDIO_SEND_STOP = 63,             //对端请求停止被拉音频流
    MEDIA_STREAM_LIVE_VIDEO_SEND_PAUSE = 64,            //对端暂停视频发送
    MEDIA_STREAM_LIVE_VIDEO_SEND_RESUME = 65,           //对端恢复视频发送
        
    MEDIA_STREAM_STREAMING_VIDEO_START = 100,
    MEDIA_STREAM_STREAMING_VIDEO_STOP = 101,

    MEDIA_STREAM_DOWNLOAD_IMAGE = 201,                  /* download image */
    MEDIA_STREAM_PLAYBACK_DELETE = 202,                 /* delete video */
    MEDIA_STREAM_ALBUM_QUERY = 203,
    MEDIA_STREAM_ALBUM_DOWNLOAD_START = 204,
    MEDIA_STREAM_ALBUM_DOWNLOAD_CANCEL = 205,
    MEDIA_STREAM_ALBUM_DELETE = 206,
    MEDIA_STREAM_ALBUM_PLAY_CTRL = 207,
    MEDIA_STREAM_DOWNLOAD_IMAGE_V2 = 208,               // download image 支持加密

    //xvr相关
    MEDIA_STREAM_VIDEO_START_GW = 300,         /**< 直播开始视频，参数为C2C_TRANS_CTRL_VIDEO_START*/
    MEDIA_STREAM_VIDEO_STOP_GW,         /**< 直播结束视频，参数为C2C_TRANS_CTRL_VIDEO_STOP*/
    MEDIA_STREAM_AUDIO_START_GW,         /**< 直播开始音频，参数为C2C_TRANS_CTRL_AUDIO_START*/
    MEDIA_STREAM_AUDIO_STOP_GW,         /**< 直播结束音频，参数为C2C_TRANS_CTRL_AUDIO_STOP*/
    MEDIA_STREAM_VIDEO_CLARITY_SET_GW, /**< 设置视频直播清晰度 ，参数为*/
    MEDIA_STREAM_VIDEO_CLARITY_QUERY_GW, /**< 查询视频直播清晰度 ，参数为*/
    MEDIA_STREAM_LOAD_ADJUST_GW, /**< 直播负载变更 ，参数为*/
    MEDIA_STREAM_PLAYBACK_LOAD_ADJUST_GW, /**< 开始回放 ，参数为*/
    MEDIA_STREAM_PLAYBACK_QUERY_MONTH_SIMPLIFY_GW, /* 按月查询本地视频信息，参数为  */
    MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS_GW, /* 按天查询本地视频信息，参数为  */

    MEDIA_STREAM_PLAYBACK_START_TS_GW, /* 开始回放视频，参数为  */
    MEDIA_STREAM_PLAYBACK_PAUSE_GW, /**< 暂停回放视频，参数为  */
    MEDIA_STREAM_PLAYBACK_RESUME_GW, /**< 继续回放视频，参数为  */
    MEDIA_STREAM_PLAYBACK_MUTE_GW, /**< 静音，参数为  */
    MEDIA_STREAM_PLAYBACK_UNMUTE_GW, /**< 取消静音，参数为  */
    MEDIA_STREAM_PLAYBACK_STOP_GW, /**< 停止回放视频，参数为  */

    MEDIA_STREAM_PLAYBACK_SPEED_GW, /**< 设置回放倍速，参数为  */
    MEDIA_STREAM_DOWNLOAD_START_GW, /**< 下载开始*/
    MEDIA_STREAM_DOWNLOAD_PAUSE_GW, /**< 下载暂停  */
    MEDIA_STREAM_DOWNLOAD_RESUME_GW,/**< 下载恢复*/
    MEDIA_STREAM_DOWNLOAD_CANCLE_GW,/**< 下载停止*/

    MEDIA_STREAM_SPEAKER_START_GW, /**< 开始对讲，无参数 */
    MEDIA_STREAM_SPEAKER_STOP_GW,  /**< 停止对讲，无参数 */
    MEDIA_STREAM_ABILITY_QUERY_GW,/**< 能力查询 C2C_MEDIA_STREAM_QUERY_FIXED_ABI_REQ*/
    MEDIA_STREAM_CONN_START_GW,    /**< 开启连接 */
    MEDIA_STREAM_PLAYBACK_DELETE_GW  , /* delete video */

    //for page mode play back enum
    MEDIA_STREAM_PLAYBACK_QUERY_DAY_TS_PAGE_MODE, /* query storage info of day with page id*/
    MEDIA_STREAM_PLAYBACK_QUERY_EVENT_DAY_TS_PAGE_MODE, /* query storage evnet info of day with page id */
    MEDIA_STREAM_PLAYBACK_DELETE_EVENT_BYFRAGMENT, /* delete storage evnet by time fragment */

}MEDIA_STREAM_EVENT_E;


typedef enum
{
    TRANS_EVENT_SUCCESS = 0,                        /* 返回成功 */
    TRANS_EVENT_SPEAKER_ISUSED       = 10,          /* speker 已被使用，不同的TRANSFER_SOURCE_TYPE_E */
    TRANS_EVENT_SPEAKER_REPSTART     = 11,          /* speker 重复开启，同一个TRANSFER_SOURCE_TYPE_E */
    TRANS_EVENT_SPEAKER_STOPFAILED   = 12,          /* speker stop 失败*/
    TRANS_EVENT_SPEAKER_INVALID      = 99
}TRANSFER_EVENT_RETURN_E;


typedef enum
{
    TRANSFER_SOURCE_TYPE_P2P    = 1,
    TRANSFER_SOURCE_TYPE_WEBRTC = 2,
    TRANSFER_SOURCE_TYPE_STREAMER = 3,
} TRANSFER_SOURCE_TYPE_E;


/**
 * \brief P2P online status
 * \enum TRANSFER_ONLINE_E
 */
typedef enum
{
    TY_DEVICE_OFFLINE,
    TY_DEVICE_ONLINE,
}TRANSFER_ONLINE_E;

    
typedef enum{
    TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_VIDEO = 0x1,      // if support video
    TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_SPEAKER = 0x2,    // if support speaker
    TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_MIC = 0x4,        // is support MIC
}TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE;


// request, response
typedef struct tagC2CCmdQueryFixedAbility{
    UINT_T channel;
    UINT_T ability_mask;//ability is assigned by bit
}C2C_TRANS_QUERY_FIXED_ABI_REQ, C2C_TRANS_QUERY_FIXED_ABI_RESP;

typedef enum
{
    TY_VIDEO_CLARITY_STANDARD = 0,
    TY_VIDEO_CLARITY_HIGH,
    TY_VIDEO_CLARITY_THIRD,
    TY_VIDEO_CLARITY_FOURTH,
    TY_VIDEO_CLARITY_MAX
}TRANSFER_VIDEO_CLARITY_TYPE_E;

    

/**************************struct define***************************************/
typedef INT_T (*MEDIA_STREAM_EVENT_CB)(IN CONST INT_T device, IN CONST INT_T channel, IN CONST MEDIA_STREAM_EVENT_E event, IN PVOID_T args);

typedef struct
{
    TRANSFER_VIDEO_CLARITY_TYPE_E clarity;
    VOID *pReserved;
}C2C_TRANS_LIVE_CLARITY_PARAM_S;

typedef struct tagC2C_TRANS_CTRL_LIVE_VIDEO{
    UINT_T channel;
    UINT_T type;      //拉流类型
}C2C_TRANS_CTRL_VIDEO_START,C2C_TRANS_CTRL_VIDEO_STOP;

typedef struct tagC2C_TRANS_CTRL_LIVE_AUDIO{
    UINT_T channel;   
}C2C_TRANS_CTRL_AUDIO_START,C2C_TRANS_CTRL_AUDIO_STOP;

typedef struct
{
    UINT_T start_timestamp; /* start timestamp in second of playback */
    UINT_T end_timestamp;   /* end timestamp in second of playback */
} PLAYBACK_TIME_S;

typedef struct tagPLAY_BACK_ALARM_FRAGMENT{
    UINT16_T video_type;      ///< 0:常规录像 1:AOV录像
    UINT16_T type;            ///< event type
	PLAYBACK_TIME_S time_sect;
}PLAY_BACK_ALARM_FRAGMENT;

typedef struct{
    UINT_T file_count;                            // file count of the day
    PLAY_BACK_ALARM_FRAGMENT file_arr[0];                  // play back file array
}PLAY_BACK_ALARM_INFO_ARR;


#pragma pack (4) 
typedef struct tagPLAY_BACK_FILE_INFOS_WITH_ENCRYPT {
    UINT16_T video_type;      ///< 0:常规录像 1:AOV录像
    UINT16_T type;            ///< event type
    CHAR_T uuid[32];
    PLAYBACK_TIME_S time_sect;
    UINT_T encrypt;
    BYTE_T key_hash[16];
} PLAY_BACK_FILE_INFOS_WITH_ENCRYPT;
#pragma pack () 

typedef struct {
    UINT_T file_count;             // file count of the day
    PLAY_BACK_FILE_INFOS_WITH_ENCRYPT file_arr[0]; // play back file array
} PLAY_BACK_ALARM_INFO_WITH_ENCRYPT_ARR;


typedef struct{
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;  
}C2C_TRANS_QUERY_PB_DAY_INNER_REQ;

typedef struct{
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;  
    PLAY_BACK_ALARM_INFO_ARR * alarm_arr;
    UINT_T ipcChan;
}C2C_TRANS_QUERY_PB_DAY_RESP;

typedef struct{
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;  
    UINT_T ipcChan;//??? todo position
    INT_T allow_encrypt;
    PLAY_BACK_ALARM_INFO_WITH_ENCRYPT_ARR * alarm_arr;
}C2C_TRANS_QUERY_PB_DAY_WITH_ENCRYPT_RESP;

typedef struct{
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;

    INT_T encrypt_allow;       		   // 0:不允许加密 1:允许加密
}C2C_TRANS_QUERY_PB_DAY_INNER_REQ_V3_T;

#pragma pack (4) 
typedef struct {
    UINT_T video_type;              ///< 0:常规录像 1:AOV录像
  	UINT_T event_type;              ///< 暂时无用，APP 要求兼容
    CHAR_T uuid[32];
    PLAYBACK_TIME_S time_sect;      ///< 录像时段
    INT_T encrypt;                  ///< 是否加密
    BYTE_T key_hash[16];

    UINT_T event_type_count;        ///< 事件数量
    UINT_T event_type_arr[0];       ///< 事件类型数组，大小参照 event_type_count
                                    ///< 事件类型数值定义: https://backendng-cn.tuya-inc.com:7799/iotos/dpManage
} PLAY_BACK_EVENT_INFO_V3_T;
#pragma pack () 

typedef struct {
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;
 
    UINT_T file_count;             // file count of the day
    PLAY_BACK_EVENT_INFO_V3_T file_arr[0]; // play back file array
} C2C_TRANS_PLAY_BACK_EVENT_INFOS_V3_ARR_T;

typedef struct{
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;
    UINT_T ipcChan;
    INT_T allow_encrypt;

    UINT_T file_count;                      // file count of the day
    UINT_T file_arr_sz;                     // file_arr size, Because the lengths of file_arr elements are variable, 
                                            // the entire length of the array needs to be informed
    CHAR_T* file_arr;                       // PLAY_BACK_EVENT_INFO_V3_T , the lengths of PLAY_BACK_EVENT_INFO_V3_T is variable
}C2C_TRANS_QUERY_PB_DAY_V3_RESP_T;


// 回放数据删除 按天request 
typedef struct tagC2C_TRANS_CTRL_PB_DELDATA_BYDAY_REQ{
    UINT_T channel;
    UINT_T year;                                          // 要删除的年份
    UINT_T month;                                         // 要删除的月份
    UINT_T day;                                           // 要删除的天数
}C2C_TRANS_CTRL_PB_DELDATA_BYDAY_REQ;

typedef struct tagC2C_TRANS_CTRL_PB_DOWNLOAD_IMAGE_S{
    UINT_T channel;
    PLAYBACK_TIME_S time_sect;                                  // 开始下载时间点
    CHAR_T reserved[32];
    INT_T result;                      // 结果，可以扩展错误码TY_C2C_CMD_IO_CTRL_STATUS_CODE
    INT_T image_fileLength ;                                      //  文件长度 后面紧跟着h文件内容过来
    BYTE_T *pBuffer;                                    // 文件内容
}C2C_TRANS_CTRL_PB_DOWNLOAD_IMAGE_PARAM_S;

typedef struct {
    UINT_T channel;
    PLAYBACK_TIME_S time_sect;          // 开始下载时间点
    INT_T encrypt_allow;                  // 是否允许加密
    CHAR_T pic_name[32];                  // 图片文件名(包含扩展名)
    INT_T encrypt;                        // 图片数据是否加密(查找图片返回后填写)
    INT_T image_file_len;                 // 文件长度
    BYTE_T *p_buffer;            // 文件内容
}C2C_TRANS_CTRL_PB_DOWNLOAD_IMAGE_V2_T;

// query playback data by month
typedef struct tagC2CCmdQueryPlaybackInfoByMonth{
	UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;   //list days that have playback data. Use each bit for one day. For example day=26496=0110 0111 1000 0000 means day 7/8/9/19/13/14 have playback data.
    UINT_T ipcChan;
}C2C_TRANS_QUERY_PB_MONTH_REQ, C2C_TRANS_QUERY_PB_MONTH_RESP;

typedef struct tagC2CCmdQueryPlaybackInfoByMonthInner{
	UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;   //list days that have playback data. Use each bit for one day. For example day=26496=0110 0111 1000 0000 means day 7/8/9/19/13/14 have playback data.
}C2C_TRANS_QUERY_PB_MONTH_INNER_REQ, C2C_TRANS_QUERY_PB_MONTH_INNER_RESP;



typedef struct tagC2C_TRANS_CTRL_PB_START{
    UINT_T channel;
    PLAYBACK_TIME_S time_sect;   
    UINT_T playTime;  /* the actual playback time, in second */
    TRANSFER_SOURCE_TYPE_E type;
    UINT_T reqId; /*  request ID, need by send frame api */
    INT_T allow_encrypt;
}C2C_TRANS_CTRL_PB_START;

typedef struct tagC2C_TRANS_CTRL_PB_STOP{
    UINT_T channel;
}C2C_TRANS_CTRL_PB_STOP;


typedef struct tagC2C_TRANS_CTRL_PB_PAUSE{
    UINT_T channel;
}C2C_TRANS_CTRL_PB_PAUSE;

typedef struct tagC2C_TRANS_CTRL_PB_RESUME{
    UINT_T channel;
    UINT_T reqId;
}C2C_TRANS_CTRL_PB_RESUME;



typedef struct tagC2C_TRANS_CTRL_PB_MUTE{
    UINT_T channel;
}C2C_TRANS_CTRL_PB_MUTE;

typedef struct tagC2C_TRANS_CTRL_PB_UNMUTE{
    UINT_T channel;
    UINT_T reqId;
}C2C_TRANS_CTRL_PB_UNMUTE;



typedef struct tagC2C_TRANS_CTRL_PB_SET_SPEED{
    UINT_T channel;
    UINT_T speed;
    UINT_T reqId;
}C2C_TRANS_CTRL_PB_SET_SPEED;

typedef struct tagC2C_TRANS_CTRL_PB_DELDATA_BYFRAGMENT_REQ {
    UINT_T channel;
    CHAR_T reserved[64];          // 保留
    INT_T sec_count;              // 需要删除的片段总数
    PLAYBACK_TIME_S sec[0];
}C2C_TRANS_CTRL_PB_DELDATA_BYFRAGMENT_REQ_T;

/**
 * \brief network load change callback struct
 * \note NOT supported now
 */
typedef struct
{
    INT_T client_index;
    INT_T curr_load_level; /**< 0:best 5:worst */
    INT_T new_load_level; /**< 0:best 5:worst */

    VOID *pReserved;
}C2C_TRANS_PB_LOAD_PARAM_S;

typedef struct
{
    INT_T client_index;
    INT_T curr_load_level; /**< 0:best 5:worst */
    INT_T new_load_level; /**< 0:best 5:worst */

    VOID *pReserved;
}C2C_TRANS_LIVE_LOAD_PARAM_S;


typedef struct tagC2C_TRANS_CTRL_DL_START{
    UINT_T channel;
    UINT_T fileNum;
    UINT_T downloadStartTime;
    UINT_T downloadEndTime;
    PLAYBACK_TIME_S *pFileInfo;   
}C2C_TRANS_CTRL_DL_START;


typedef struct tagC2C_TRANS_CTRL_DL_ENCRYPT_START {
    UINT_T channel;
    UINT_T fileNum;
    UINT_T downloadStartTime;
    UINT_T downloadEndTime;
    INT_T allow_encrypt; // 是否允许加密数据
    INT_T reqId;// request ID, need by send frame api
    PLAYBACK_TIME_S *pFileInfo;
} C2C_TRANS_CTRL_DL_ENCRYPT_START;

typedef struct tagC2C_TRANS_CTRL_DL_STOP{
    UINT_T channel;  
}C2C_TRANS_CTRL_DL_STOP,C2C_TRANS_CTRL_DL_PAUSE,C2C_TRANS_CTRL_DL_RESUME,C2C_TRANS_CTRL_DL_CANCLE;

typedef enum
{
    TUYA_DOWNLOAD_VIDEO = 0,
    TUYA_DOWNLOAD_ALBUM,
    TUYA_DOWNLOAD_VIDEO_ALLOW_ENCRYPT,
    TUYA_DOWNLOAD_MAX,
}TUYA_DOWNLOAD_DATA_TYPE;

typedef struct {
    INT_T video_codec;
    UINT_T frame_rate;
    UINT_T video_width;
    UINT_T video_height;
} TRANSFER_IPC_VIDEO_INFO_S;

typedef struct {
    INT_T audio_codec;
    INT_T audio_sample;// TUYA_AUDIO_SAMPLE_E
    INT_T audio_databits;// TUYA_AUDIO_DATABITS_E
    INT_T audio_channel;// TUYA_AUDIO_CHANNEL_E
} TRANSFER_IPC_AUDIO_INFO_S;

typedef struct {
    INT_T encrypt;        // 是否加密
    INT_T security_level; // 安全等级
    CHAR_T uuid[32];      // 设备UUID
    BYTE_T iv[16];        // 加密向量
} TRANSFER_MEDIA_ENCRYPT_INFO_T;

typedef struct {
    INT_T frame_type;// MEDIA_FRAME_TYPE_E
    BYTE_T *p_buf;
    UINT_T size;
    UINT64_T pts;
    UINT64_T timestamp;
    union {
        TRANSFER_IPC_VIDEO_INFO_S video;
        TRANSFER_IPC_AUDIO_INFO_S audio;
    } media;

    TRANSFER_MEDIA_ENCRYPT_INFO_T encrypt_info;
} TRANSFER_MEDIA_FRAME_WIHT_ENCRYPT_T;// 回放使用

typedef struct {
    INT_T type;// MEDIA_FRAME_TYPE_E
    UINT_T size;
    UINT64_T timestamp;
    UINT64_T pts;
    union {
        TRANSFER_IPC_VIDEO_INFO_S video;
        TRANSFER_IPC_AUDIO_INFO_S audio;
    } media;
    TRANSFER_MEDIA_ENCRYPT_INFO_T encrypt_info;
} TUYA_DOWNLOAD_FRAME_HEAD_ENCRYPT_T;

typedef struct {
    INT_T type;             // 参见 MEDIA_FRAME_TYPE_E
    UINT_T size;
    UINT64_T timestamp;
    UINT64_T pts;
    union {
        TRANSFER_IPC_VIDEO_INFO_S video;
        TRANSFER_IPC_AUDIO_INFO_S audio;
    } media;
    TRANSFER_MEDIA_ENCRYPT_INFO_T encrypt_info;
    BYTE_T *p_buf;          // frame data
} TUYA_ALBUM_PLAY_FRAME_T;

/***********************************album protocol ****************************************/
#define TUYA_ALBUM_APP_FILE_NAME_MAX_LEN (48)
#define IPC_SWEEPER_ROBOT "ipc_sweeper_robot"
typedef struct {
    UINT_T channel; // 目前不需要，保留
    CHAR_T albumName[48];
    INT_T fileLen;
    void* pIndexFile;
} C2C_QUERY_ALBUM_REQ; // 查询请求头
typedef struct tagC2C_ALBUM_INDEX_ITEM {
    INT_T idx; // 设备提供并保证唯一性
    CHAR_T valid; // 0 invalid, 1 valid
    CHAR_T channel; // 0  1通道号
    CHAR_T type; // 0 保留，1 pic 2 mp4 3全景拼接图(文件夹) 4二进制文件 5流文件
    CHAR_T dir; // 0 file 1 dir
    CHAR_T filename[48]; // 123456789_1.mp4 123456789_1.jpg  xxx.xxx
    INT_T createTime; // 文件创建时间
    short duration; // 视频文件时长
    CHAR_T reserved[18];
} C2C_ALBUM_INDEX_ITEM; // 索引Item
typedef struct {
    UINT_T crc;
    INT_T version;                    // 相册功能版本(>2 支持文件在线播放) 
    CHAR_T magic[16];
    unsigned long long min_idx;
    unsigned long long max_idx;
    CHAR_T reserved[512 - 44];
    INT_T itemCount;  // include invalid items
    C2C_ALBUM_INDEX_ITEM itemArr[0];
} C2C_ALBUM_INDEX_HEAD; //查询返回:520 = 8 + 512,索引文件头+item

typedef struct {
    UINT_T channel; // 目前业务不需要，保留
    INT_T result; // 查询返回结果
    CHAR_T reserved[512 - 4]; // 保留,共512
    INT_T itemCount; // include invalid items
    C2C_ALBUM_INDEX_ITEM itemArr[0];
} C2C_CMD_IO_CTRL_ALBUM_QUERY_RESP; //查询返回:520 = 8 + 512,索引文件头+item

typedef struct tagC2C_CMD_IO_CTRL_ALBUM_fileInfo {
    CHAR_T filename[48]; //文件名，不带绝对路径
} C2C_CMD_IO_CTRL_ALBUM_fileInfo;
typedef struct tagC2C_CMD_IO_CTRL_ALBUM_DOWNLOAD_START {
    UINT_T channel; //暂无用保留
    INT_T operation; // 参见 TY_CMD_IO_CTRL_DOWNLOAD_OP
    CHAR_T albumName[48];
    INT_T thumbnail; // 0 原图 ，1 缩略图
    INT_T fileTotalCnt; //max 50
    C2C_CMD_IO_CTRL_ALBUM_fileInfo pFileInfoArr[0];
} C2C_CMD_IO_CTRL_ALBUM_DOWNLOAD_START;
typedef struct tagC2C_ALBUM_DOWNLOAD_CANCEL {
    UINT_T channel; //暂无用保留
    CHAR_T albumName[48];
} C2C_ALBUM_DOWNLOAD_CANCEL;

typedef struct tagC2C_CMD_IO_CTRL_ALBUM_DELETE {
    UINT_T channel;
    CHAR_T albumName[48];
    INT_T fileNum; // -1 全部，其他：文件个数
    CHAR_T res[64];
    C2C_CMD_IO_CTRL_ALBUM_fileInfo pFileInfoArr[0];
} C2C_CMD_IO_CTRL_ALBUM_DELETE; //删除文件

typedef struct {
    INT_T reqId;
    INT_T fileIndex; // start from 0
    INT_T fileCnt; // max 50
    CHAR_T fileName[48]; // 文件名
    INT_T packageSize; // 当前文件片段的实际数据长度
    INT_T fileSize; // 文件大小
    INT_T fileEnd; // 文件结束标志,最后一个片段10KB
} C2C_DOWNLOAD_ALBUM_HEAD; //下载数据头

typedef struct {
    UINT_T channel;
    INT_T result;     // 参见TY_C2C_CMD_IO_CTRL_STATUS_CODE_E
    INT_T operation;  // 参见TY_CMD_IO_CTRL_ALBUM_PLAY_OP_E,文件在线播放结束后为TY_CMD_IO_CTRL_ALBUM_PLAY_OVER
} C2C_CMD_IO_CTRL_ALBUM_PLAY_RESULT_RESP_T;

typedef enum {
    TY_CMD_IO_CTRL_ALBUM_PLAY_START = 0,                             // 开始播放
    TY_CMD_IO_CTRL_ALBUM_PLAY_STOP,                                  // 结束
    TY_CMD_IO_CTRL_ALBUM_PLAY_PAUSE,                                 // 暂停
    TY_CMD_IO_CTRL_ALBUM_PLAY_RESUME,                                // 恢复
    TY_CMD_IO_CTRL_ALBUM_PLAY_CANCEL,                                // 取消
    TY_CMD_IO_CTRL_ALBUM_PLAY_OVER,                                  // 播放结束,设备SDK主动发送
}TY_CMD_IO_CTRL_ALBUM_PLAY_OP_E;

typedef struct {
    UINT_T channel;       // 暂无用保留
    INT_T operation;              // 参见 TY_CMD_IO_CTRL_ALBUM_PLAY_OP_E
    INT_T thumbnail;              // 0 原图 ，1 缩略图
  	UINT_T start_time;    // 起始播放时间, 单位: s
    CHAR_T album_name[TUYA_ALBUM_APP_FILE_NAME_MAX_LEN];          // 相册名称
    CHAR_T file_name[TUYA_ALBUM_APP_FILE_NAME_MAX_LEN];           // 需要播放的文件名，不带绝对路径
} C2C_CMD_IO_CTRL_ALBUM_PLAY_CTRL_REQ_T;

typedef struct {
    UINT_T channel;       // 暂无用保留(多目相机通道)
    INT_T user_idx;
    INT_T req_id;
    INT_T operation;              // 参见 TY_CMD_IO_CTRL_ALBUM_PLAY_OP_E
    INT_T thumbnail;              // 0 原图 ，1 缩略图
  	UINT_T start_time;    // 起始播放时间, 单位: s
    CHAR_T album_name[TUYA_ALBUM_APP_FILE_NAME_MAX_LEN];         // 相册名称
    CHAR_T file_name[TUYA_ALBUM_APP_FILE_NAME_MAX_LEN];          // 需要播放的文件名，不带绝对路径
} C2C_CMD_IO_CTRL_ALBUM_PLAY_CTRL_T;

typedef enum {
    E_FILE_TYPE_2_APP_PANORAMA = 1, //全景拼接图
} FILE_TYPE_2_APP_E;
typedef struct {
    FILE_TYPE_2_APP_E fileType;
    INT_T param; // 全景拼接图时，为子图总数
} TUYA_IPC_BRIEF_FILE_INFO_4_APP;

/**
 * \fn tuya_ipc_start_send_file_to_app
 * \brief start send file to app by p2p
 * \param[in] strBriefInfo: brief file infomation
 * \return handle , >=0 valid, -1 err
 */
OPERATE_RET tuya_ipc_start_send_file_to_app(IN CONST TUYA_IPC_BRIEF_FILE_INFO_4_APP* pStrBriefInfo);

/**
 * \fn tuya_ipc_stop_send_file_to_app
 * \brief stop send file to app by p2p
 * \param[in] handle
 * \return ret
 */
OPERATE_RET tuya_ipc_stop_send_file_to_app(IN CONST INT_T handle);

typedef struct {
    CHAR_T* fileName; //最长48字节，若为null，采用SDK内部命名
    INT_T len;
    CHAR_T* buff;
} TUYA_IPC_FILE_INFO_4_APP;
/**
 * \fn tuya_ipc_send_file_to_app
 * \brief start send file to app by p2p
 * \param[in] handle: handle
 * \param[in] strfileInfo: file infomation
 * \param[in] timeOut_s: suggest 30s, 0 no_block (current not support),
 * \return ret
 */
OPERATE_RET tuya_ipc_send_file_to_app(IN CONST INT_T handle, IN CONST TUYA_IPC_FILE_INFO_4_APP* pStrfileInfo, IN CONST INT_T timeOut_s);

typedef enum {
    SWEEPER_ALBUM_FILE_TYPE_MIN = 0,
    SWEEPER_ALBUM_FILE_MAP = SWEEPER_ALBUM_FILE_TYPE_MIN,//map file
    SWEEPER_ALBUM_FILE_CLEAN_PATH = 1,
    SWEEPER_ALBUM_FILE_NAVPATH = 2,
    SWEEPER_ALBUM_FILE_TYPE_MAX = SWEEPER_ALBUM_FILE_NAVPATH,

    SWEEPER_ALBUM_STREAM_TYPE_MIN = 3,
    SWEEPER_ALBUM_STREAM_MAP = SWEEPER_ALBUM_STREAM_TYPE_MIN, // map stream , devcie should send map file to app continue
    SWEEPER_ALBUM_STREAM_CLEAN_PATH = 4,
    SWEEPER_ALBUM_STREAM_NAVPATH = 5,
    SWEEPER_ALBUM_STREAM_TYPE_MAX = SWEEPER_ALBUM_STREAM_NAVPATH,

    SWEEPER_ALBUM_FILE_ALL_TYPE_MAX = SWEEPER_ALBUM_STREAM_TYPE_MAX, //最大值 5
    SWEEPER_ALBUM_FILE_ALL_TYPE_COUNT, //个数 6
} SWEEPER_ALBUM_FILE_TYPE_E;

typedef enum {
    SWEEPER_TRANS_NULL,
    SWEEPER_TRANS_FILE, //文件传输
    SWEEPER_TRANS_STREAM, //文件流传输
} SWEEPER_TRANS_MODE_E;


// 文件传输的状态
typedef enum
{
    TY_DATA_TRANSFER_IDLE,
    TY_DATA_TRANSFER_START,
    TY_DATA_TRANSFER_PROCESS,
    TY_DATA_TRANSFER_END,
    TY_DATA_TRANSFER_ONCE,
    TY_DATA_TRANSFER_CANCEL,
    TY_DATA_TRANSFER_MAX
}TY_DATA_TRANSFER_STAT;

/***********************************album protocol end ****************************************/


/***********************************xvr  protocol start ****************************************/
typedef struct{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
    UINT_T year;                                  // 要查询的年份
    UINT_T month;                                 // 要查询的月份
    UINT_T day;                                   // 要查询的天数
}C2C_TRANS_QUERY_GW_PB_DAY_REQ;

typedef struct{
    UINT_T channel;
    UINT_T idx;//会话的索引。
    UINT_T map_chan_index;;//一个绘话中，绑定的通道。用户透传回来即可。
    CHAR_T subdid[64];
    UINT_T year;                                  // 要查询的年份
    UINT_T month;                                 // 要查询的月份
    UINT_T day;                                   // 要查询的天数
    PLAY_BACK_ALARM_INFO_ARR * alarm_arr;                    // 用户返回的查询结果
}C2C_TRANS_QUERY_GW_PB_DAY_RESP;

/**
UINT一共有32位，每1位表示对应天数是否有数据，最右边一位表示第0天。
比如 day = 26496 = B0110 0111 1000 0000
那么表示第7,8,9,10,13,14天有回放数据。
 */
// 按月查询有回放数据的天 request, response
typedef struct tagC2CCmdQueryGWPlaybackInfoByMonth{
    UINT_T channel;
    UINT_T idx;//会话的索引。
    UINT_T map_chan_index;//绘话中，绑定的通道。用户透传回来即可。
    CHAR_T subdid[64];
    UINT_T year;                                  // 要查询的年份
    UINT_T month;                                 // 要查询的月份
    UINT_T day;                                  // 有回放数据的天
}C2C_TRANS_QUERY_GW_PB_MONTH_REQ, C2C_TRANS_QUERY_GW_PB_MONTH_RESP;


// request
//回放相关操作结构体
typedef struct tagC2C_TRANS_CTRL_GW_PB_START{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
    PLAYBACK_TIME_S time_sect;
    UINT_T playTime;  /**< 实际回放开始时间戳（以秒为单位） */
}C2C_TRANS_CTRL_GW_PB_START;

typedef struct tagC2C_TRANS_CTRL_GW_PB_STOP{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_PB_STOP;

typedef struct tagC2C_TRANS_CTRL_GW_PB_PAUSE{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_PB_PAUSE,C2C_TRANS_CTRL_GW_PB_RESUME;

typedef struct tagC2C_TRANS_CTRL_GW_PB_MUTE{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_PB_MUTE,C2C_TRANS_CTRL_GW_PB_UNMUTE;

// 能力集查询 C2C_CMD_QUERY_FIXED_ABILITY
// request, response
typedef struct tagC2CCmdQueryGWFixedAbility{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
    UINT_T ability_mask;//能力结果按位赋值
}C2C_TRANS_QUERY_GW_FIXED_ABI_REQ, C2C_TRANS_QUERY_GW_FIXED_ABI_RESP;

/**
 * \brief 直播模式下申请修改或者查询清晰度回调参数结构体
 * \struct C2C_TRANS_LIVE_CLARITY_PARAM_S
 */
typedef struct
{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
    TRANSFER_VIDEO_CLARITY_TYPE_E clarity; /**< 视频清晰度 */
    VOID *pReserved;
}C2C_TRANS_LIVE_GW_CLARITY_PARAM_S;

//预览相关操作结构体
typedef struct tagC2C_TRANS_CTRL_GW_LIVE_VIDEO{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_VIDEO_START,C2C_TRANS_CTRL_GW_VIDEO_STOP;

typedef struct tagC2C_TRANS_CTRL_GW_LIVE_AUDIO{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_AUDIO_START,C2C_TRANS_CTRL_GW_AUDIO_STOP;

typedef struct tagC2C_TRANS_CTRL_GW_SPEAKER{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_SPEAKER_START,C2C_TRANS_CTRL_GW_SPEAKER_STOP;

typedef struct tagC2C_TRANS_CTRL_GW_DEV_CONN{
    UINT_T channel;
    UINT_T idx;
    CHAR_T subdid[64];
}C2C_TRANS_CTRL_GW_DEV_CONN;

typedef struct tagC2C_TRANS_CTRL_PB_SET_SPEED_GW{
    CHAR_T devid[64];
    UINT_T channel;
    UINT_T speed;
}C2C_TRANS_CTRL_PB_SET_SPEED_GW;


// 回放数据删除 按天request
typedef struct tagC2C_TRANS_CTRL_PB_DELDATA_BYDAY_GW_REQ{
    CHAR_T subdid[64];
    UINT_T channel;
    UINT_T year;                                          // 要删除的年份
    UINT_T month;                                         // 要删除的月份
    UINT_T day;                                           // 要删除的天数
}C2C_TRANS_CTRL_PB_DELDATA_BYDAY_GW_REQ;

typedef struct tagC2C_CMD_PROTOCOL_VERSION{
    UINT_T version;       //高位主版本号，低16位次版本号
}C2C_CMD_PROTOCOL_VERSION;
typedef struct{
    CHAR_T subid[64];
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;
    INT_T page_id;
}C2C_TRANS_QUERY_PB_DAY_V2_REQ,C2C_TRRANS_QUERY_EVENT_PB_DAY_REQ;
#pragma pack(4)
typedef struct{
    CHAR_T subid[64];
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;
    INT_T page_id;
    INT_T total_cnt;
    INT_T page_size;
    PLAY_BACK_ALARM_INFO_ARR  *alarm_arr;
    INT_T idx;//会话索引
}C2C_TRANS_QUERY_PB_DAY_V2_RESP;
typedef struct {
    UINT_T start_timestamp; /* start timestamp in second of playback */
    UINT_T end_timestamp;
    unsigned short video_type;      ///< 0:常规录像 1:AOV录像
    unsigned short type;            ///< event type
    CHAR_T pic_id[20];
}C2C_PB_EVENT_INFO_S;
typedef struct {
    INT_T version;
    INT_T event_cnt;
    C2C_PB_EVENT_INFO_S event_info_arr[0];
}C2C_PB_EVENT_INFO_ARR_S;
typedef struct{
    CHAR_T subid[64];
    UINT_T channel;
    UINT_T year;
    UINT_T month;
    UINT_T day;
    INT_T page_id;
    INT_T total_cnt;
    INT_T page_size;
    C2C_PB_EVENT_INFO_ARR_S * event_arr;
    INT_T idx;
}C2C_TRANS_QUERY_EVENT_PB_DAY_RESP;
#pragma pack()
typedef struct tagC2C_TRANS_CTRL_GW_DL_STOP{
    CHAR_T devid[64];
    UINT_T channel;
}C2C_TRANS_CTRL_GW_DL_STOP,C2C_TRANS_CTRL_GW_DL_PAUSE,C2C_TRANS_CTRL_GW_DL_RESUME,C2C_TRANS_CTRL_GW_DL_CANCLE;
typedef struct tagC2C_TRANS_CTRL_GW_DL_START{
    CHAR_T devid[64];
    UINT_T channel;
    UINT_T downloadStartTime;
    UINT_T downloadEndTime;
}C2C_TRANS_CTRL_GW_DL_START;
/***********************************xvr  protocol end ****************************************/





/**************************function define***************************************/

OPERATE_RET tuya_ipc_media_stream_register_event_cb(MEDIA_STREAM_EVENT_CB event_cb);

OPERATE_RET tuya_ipc_media_stream_event_call( INT_T device, INT_T channel, MEDIA_STREAM_EVENT_E event, PVOID_T args);


#ifdef __cplusplus
}
#endif

#endif /*_TUYA_IPC_MEDIA_STREAM_EVENT_H_*/
