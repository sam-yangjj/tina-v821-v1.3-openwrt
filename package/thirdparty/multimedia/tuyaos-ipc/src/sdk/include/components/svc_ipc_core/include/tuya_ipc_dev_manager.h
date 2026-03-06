#ifndef _TUYA_IPC_DEV_MANAGER_H_
#define _TUYA_IPC_DEV_MANAGER_H_

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STREAM_RESTRICT     1
#define FILE_RESTRICT       2
#define OTHER_RESTRICT      4 //for expansion
#define ALL_RESTRICT        (STREAM_RESTRICT | FILE_RESTRICT)

/**
 * @brief Initialize the device management module
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_dev_manager_init();

/**
 * @brief Whether the mode is restricted
 * @param[in] module STREAM_RESTRICT/FILE_RESTRICT/OTHER_RESTRICT/ALL_RESTRICT
 * @return BOOL_T TRUE:restricted/FALSE:not restricted
*/
BOOL_T tuya_ipc_dev_manager_func_restrict(INT_T module);

#ifdef __cplusplus
}
#endif

#endif /*_TUYA_IPC_DEV_MANAGER_H_*/