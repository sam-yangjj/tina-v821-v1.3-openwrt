/**
 * @file tuya_devos_log_upd.c
 * @author fangweng.hu@tuya.com
 * @brief tuya log upload
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright 2019-2023 Tuya Inc. All Rights Reserved.
 *
 */
#include <stdio.h>
#include <string.h>
#include "tal_memory.h"
#include "uni_log.h"
#include "ty_cJSON.h"
#include "tuya_ws_db.h"

#if defined(ENABLE_LOG_UPLOAD) && (ENABLE_LOG_UPLOAD==1)
#define UPD_FILE_NAME_LEN          256

/**
 * @brief get local log name
 *
 * @param[in] max_name_len max log name length
 * @param[out] log_name local log name, include path
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
typedef OPERATE_RET(*TY_GET_LOG_FILE_NAEM_CB)(CHAR_T *log_name, UINT_T max_name_len);
/**
 * @brief called when log upload finished
 *
 */
typedef VOID(*TY_FIN_LOG_UPD_CB)();

typedef struct {
    /* get file name */
    TY_GET_LOG_FILE_NAEM_CB get_name;
    /* fin log upload */
    TY_FIN_LOG_UPD_CB fin_upd;
} TY_LOG_UPD_CB_S;

/**
 * @brief set log upload cbs
 *
 * @param[in] cb callback
 *
 */
VOID tuya_log_upd_set_cb(TY_LOG_UPD_CB_S *cb);

/**
 * @brief log upload init
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_log_upd_init();

/**
 * @brief log upload file trigger by device manually
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_log_upd_file_dev_trig();
#endif
