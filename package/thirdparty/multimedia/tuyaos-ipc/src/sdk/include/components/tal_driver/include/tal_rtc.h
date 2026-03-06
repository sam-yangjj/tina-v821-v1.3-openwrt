/**
 * @file tal_rtc.h
 * @brief Common process - adapter the rtc api
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2031 Tuya Inc. All Rights Reserved.
 *
 */
 
#ifndef __TAL_RTC_H__
#define __TAL_RTC_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief rtc init
 * 
 * @param[in] none
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_rtc_init(VOID_T);

/**
 * @brief rtc deinit
 * @param[in] none
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_rtc_deinit(VOID_T);

/**
 * @brief rtc time set
 * 
 * @param[in] time_sec: rtc time seconds
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_rtc_time_set(TIME_T time_sec);

/**
 * @brief rtc time get
 * 
 * @param[in] time_sec:rtc time seconds
 *
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tal_rtc_time_get(TIME_T *time_sec);


#define TAL_RTC_DISABLE_CMD         1
#define TAL_RTC_SET_FLAG            (1 << 0)
#define TAL_RTC_GET_FLAG            (1 << 1)

OPERATE_RET tal_rtc_time_control(UINT8_T cmd, VOID_T *arg);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __TAL_RTC_H__

