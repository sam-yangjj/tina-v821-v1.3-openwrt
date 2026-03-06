/**
 * @file tuya_imm_service_log.h
 * @author dante (houcl@tuya.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __TUYA_IMM_SERVICE_LOG_H__
#define __TUYA_IMM_SERVICE_LOG_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPRT_IMM_LOG_BUFFER_READY 1
#define OPRT_IMM_LOG_BUFFER_FULL 2

#define IMM_LOG_DOMAIN_LEN 64
#define AES_KEY_LEN 16
#define REZERVER_LEN 100 //align16 + ***

typedef VOID * IMM_SERVICE_LOG_HDL;

typedef enum
{
    SERVICE_LOG_TRACE,
    SERVICE_LOG_DEBUG,
    SERVICE_LOG_INFO,
    SERVICE_LOG_WARN,
    SERVICE_LOG_ERROR,
    SERVICE_LOG_FATAL,
    SERVICE_LOG_MAX
}IMM_SERVICE_LOG_LV_T;

typedef struct
{
    IMM_SERVICE_LOG_LV_T        log_level;
    INT_T                       interval;
    CHAR_T                      domain[IMM_LOG_DOMAIN_LEN];
    USHORT_T                    port;
    BYTE_T                      key[AES_KEY_LEN+1];
}IMM_LOG_SERVER_INFO_T;

typedef OPERATE_RET (*LOG_GATHER_CB)();
/**
 * @brief init
 * 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_imm_service_log_init();


/**
 * @brief tuya_imm_service_log_module_set_cb
 * 
 * @param cb 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_imm_service_log_module_set_cb(LOG_GATHER_CB cb);

/**
 * @brief tuya_imm_service_log_module_clean_cb
 * 
 * @param cb 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_imm_service_log_module_clean_cb(LOG_GATHER_CB cb);

/**
 * @brief tuya_imm_service_log_put
 * 
 * @param log 
 * @param log_len 
 * @return OPERATE_RET 
 */
OPERATE_RET tuya_imm_service_log_put(IMM_SERVICE_LOG_LV_T level, CHAR_T *log, INT_T log_len);


INT_T tuya_imm_service_log_get_level();
#ifdef __cplusplus
}
#endif

#endif