/**
 * @file sys_log.h
 * @brief Common process -sys log 
 * @version 0.1
 * @date 2021-08-06
 *
 * @copyright Copyright 2021-2022 Tuya Inc. All Rights Reserved.
 *
 */


#ifndef __SYS_LOG_H__
#define __SYS_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    TUYA_LOG_MODE_NONE     = 0x00000000,
    TUYA_LOG_MODE_CONSOLE  = 0x0000000F,
    TUYA_LOG_MODE_SYSLOG   = 0x000000F0, 
    TUYA_LOG_MODE_FILE     = 0x00000F00,
    TUYA_LOG_MODE_JOURNALD = 0x0000F000,
    TUYA_LOG_MODE_CALLBACK = 0x000F0000,
    TUYA_LOG_MODE_ALL      = 0xFFFFFFFF,
}TUYA_LOG_MODE_E;

typedef enum {
    TUYA_LOG_LEVEL_NONE   = 0,
    TUYA_LOG_LEVEL_ERROR  = 1,
    TUYA_LOG_LEVEL_WARNING =2,
    TUYA_LOG_LEVEL_INFO   = 3,
    TUYA_LOG_LEVEL_DEBUG  = 4,
    TUYA_LOG_LEVEL_ALL    = 5,
}TUYA_LOG_LEVEL_E;

/**
 * @brief sys log init
 * @param[in]ident: log string flag,general is app name   
 * @param[in]mode:log mode,see also TUYA_LOG_MODE_E
 * @param[in]level:log level,see also TUYA_LOG_LEVEL_E
 * @param[in]log file:log file,log save 
 * @parami[in]log_file: path to the file that will contain the log messages if log mode TUYA_LOG_MODE_FILE is selected
 * @return 0,success;-1,fail
 */
extern int sys_log_init(TUYA_LOG_MODE_E mode,TUYA_LOG_LEVEL_E level,const char *log_file);

/*
 *@brief callback function
 */
typedef int (*TUYA_LOG_CALLBACK)(TUYA_LOG_LEVEL_E type,const char *msg);

/**
 * @brief specify a callback function that will catch all log messages
 * in addition to other logs output already defined in y_init_logs
 * @param[in]cb: apointer to a user-defined callback function
 * @return 0,success;-1,fail
 */
extern int sys_log_set_callback(TUYA_LOG_CALLBACK cb);

/**
 * @brief sys log a message using current parameters
 * @param[in]level:log level,see also TUYA_LOG_LEVEL_E
 * @param[in]msg:message the message to log, using printf format
 * @param[in]tag:message tag 
 * @return 0,success;-1,fail
 */
extern int  sys_log_msg(TUYA_LOG_LEVEL_E level,const char *tag,const char *msg, ...);

/**
 * @brief sys log deinit
 * @param[in]void:message the message to log, using printf format
 * @return 0,success;-1,fail
 */
extern int sys_log_deinit(void);

#define DEBUG_STR     "%d,%s: "
#define DEBUG_ARGS    __LINE__,__FUNCTION__

#define LOGD(tag,msg,arg ...) \
     sys_log_msg(TUYA_LOG_LEVEL_DEBUG,tag,DEBUG_STR msg,DEBUG_ARGS,##arg);

#define LOGI(tag,msg,arg ...) \
     sys_log_msg(TUYA_LOG_LEVEL_INFO,tag,DEBUG_STR msg,DEBUG_ARGS,##arg);

#define LOGW(tag,msg,arg ...) \
     sys_log_msg(TUYA_LOG_LEVEL_WARNING,tag,DEBUG_STR msg,DEBUG_ARGS,##arg);

#define LOGE(tag,msg,arg ...) \
     sys_log_msg(TUYA_LOG_LEVEL_ERROR,tag,DEBUG_STR msg,DEBUG_ARGS,##arg);

#define LOG(level,tag,msg,arg ...) \
     sys_log_msg(level,tag,DEBUG_STR msg,DEBUG_ARGS,##arg);

#ifdef __cplusplus
}
#endif

#endif
