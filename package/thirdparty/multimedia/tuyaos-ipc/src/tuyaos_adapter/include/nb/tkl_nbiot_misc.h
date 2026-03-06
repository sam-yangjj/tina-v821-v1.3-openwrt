/**
 * @file tkl_nbiot_misc.h
 * @brief Common process - the nbiot misc driver api
 * 
 * @copyright Copyright 2018-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#ifndef __TKL_NBIOT_MISC_H__
#define __TKL_NBIOT_MISC_H__

#include "tkl_nbiot_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TKL_CHIP_MTK2625    = 0,        //适配所有涂鸦自己的差分升级流程！！！
    TKL_CHIP_EC616      = 1,
    TKL_CHIP_XY1100     = 2,
    TKL_CHIP_MAX        = 3,
} TKL_MISC_CHIP_TYPE_E;

/**
 * @brief get the chip type
 * 
 * @param[inout] out_type ：output the chip type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_chip_type(TKL_MISC_CHIP_TYPE_E *out_type);

/**
 * @brief if AT serial port is on or off
 * 
 * @param[in] null
 *
 * @return  true: on  Other: off
 */
BOOL_T tkl_nbiot_is_atcmd_serial_port_on(VOID);

/**
 * @brief set AT serial port off
 * 
 * @param[in] null
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_atcmd_serial_port_off(VOID);

/**
 * @brief set AT serial port on
 * 
 * @param[in] null
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_atcmd_serial_port_on(VOID);

/**
* @brief get the offset address of app firmware
* 
* @param[inout] out_addr ：output the offset address
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_app_offset(UINT_T *out_addr);

/**
* @brief get the app firmware space length
* 
* @param[inout] out_addr ：output the offset address
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_app_len(UINT_T *out_len);

/**
* @brief notify fota update start, some platform need acquire this info then restart system,such as NX1, others return directly
* 
* @param[in] null
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_fota_update_start(VOID);

/**
* @brief notify fota update end, some platform need acquire this info to clear update status,such as NX1, others return directly
* 
* @param[in] null
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_fota_update_end(VOID);

/**
* @brief some upgrade status needs to be obtained from the platform,such as NX1, others return directly
* 
* @param[in] null
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_fota_status_acquire(VOID);

/**
* @brief get the offset address to save firmware
* 
* @param[inout] out_addr ：output the offset address
* @param[inout] _1st_data ：the first fota data pointer
* @param[inout] _1st_data_size ：the first fota data length
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_fota_offset(UINT_T *out_addr, UCHAR_T *_1st_data, UINT32_T _1st_data_size);

/**
* @brief get the space legth to save firmware
* 
* @param[inout] out_addr ：output the offset address
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_fota_len(UINT_T *out_len);

/**
* @brief get the offset address to save update info
* 
* @param[inout] out_addr ：output the offset address
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_fota_update_info_offset(UINT_T *out_addr);

/**
* @brief get the offset address to save trigger fota indicator
* 
* @param[inout] out_addr ：output the offset address
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_get_fota_trigger_flag_offset(UINT_T *out_addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



