#ifndef __TUYA_IPC_AOV_CLOUD_STORAGE_H__
#define __TUYA_IPC_AOV_CLOUD_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/**
 * @brief    initialize aov cloud storage, which will malloc all needed memory after this
 * @return   OPERATE_RET
 * - OPRT_OK init success
 * - Others  init failed
 */
OPERATE_RET tuya_ipc_aov_cloud_storage_init(VOID);

/**
 * @brief    start aov cloud storage
 * @return   OPERATE_RET
 * - OPRT_OK start aov cloud storage success
 * - Others  start aov cloud storage failed
 */
OPERATE_RET tuya_ipc_aov_cloud_storage_start(VOID);

/**
 * @brief    get aov cloud storage status
 * @return   BOOL_T
 * - TRUE    aov cloud storage is ongoing
 * - FALSE   aov cloud storage is idle
 */
BOOL_T tuya_ipc_aov_cloud_storage_get_status(VOID);

/**
 * @brief    pause aov cloud storage, used in privacy mode e.g.
 * @return   VOID
 */
VOID tuya_ipc_aov_cloud_storage_pause(VOID);

/**
 * @brief    resume aov cloud storage which is paused by pause API
 * @return   VOID
 */
VOID tuya_ipc_aov_cloud_storage_resume(VOID);

#ifdef __cplusplus
}
#endif

#endif
