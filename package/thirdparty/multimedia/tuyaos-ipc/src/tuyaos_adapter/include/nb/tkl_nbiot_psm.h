/**
* @file tkl_nbiot_psm.h
* @brief Common process - adapter the nbiot psm api
* @version 0.1
* @date 2021-09-09
*
* @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
*
*/

#ifndef __TKL_NBIOT_PSM_H__
#define __TKL_NBIOT_PSM_H__

#include "tuya_cloud_types.h"
#include "tkl_nbiot_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef UCHAR_T TKL_LOCK_HANDLE;

typedef enum {
    TKL_NB_FIRST_BOOT_UP       = 0,     /* 1st boot up */
    TKL_NB_DEEP_SLEEP          = 1,     /* deep sleep */
    TKL_NB_DEEPER_SLEEP        = 2,     /* power off */
    TKL_NB_SYS_RESET           = 3,     /* long press shutdown or sys_reset */
    TKL_NB_WDT_HW_RESET        = 4,     /* wdt hw reset */
    TKL_NB_WDT_SW_RESET        = 5,     /* wdt sw reset */
    TKL_NB_FORCED_SHUT_DOWN    = 6,     /* forced shut down */
    TKL_NB_FORCED_RESET        = 7,     /* forced reset */
    TKL_NB_POWER_ON_INVALID    = 0xFF,   /* invalid result */
    TKL_NB_POWER_ON_RESULT_MAX
} TKL_NB_POWER_ON_RESULT_E;

typedef enum {
    TKL_PSM_SLEEP_NONE   = 0,
    TKL_PSM_LIGHT_SLEEP  = 1,
    TKL_PSM_DEEP_SLEEP   = 2,
    TKL_PSM_DEEPER_SLEEP = 3,
    TKL_PSM_SLEEP_MAX    = 4,
} TKL_NB_PSM_SLP_TYPE_E;

typedef enum
{
    TKL_WAKEUP_FROM_NONE,
    TKL_WAKEUP_FROM_POR, // power on  or reset
    TKL_WAKEUP_FROM_RTC,
    TKL_WAKEUP_FROM_KEY,
}TKL_NB_WAKEUP_SOURCE_E;

/**
 * @brief prototype of the PSM notify call back
 * @param sleep_type @ref TKL_NB_PSM_SLP_TYPE_E
 */
typedef VOID_T (*TKL_NB_PSM_NOTIFY_CB_T)(TKL_NB_PSM_SLP_TYPE_E sleep_type);

/**
 * @brief prototype of the sleep timer call back
 *
 * @param user_data data for user input
 */
typedef VOID_T (*TKL_NB_SLP_TIMER_CB_T)(PVOID_T user_data);

/**
 * @brief prototype of the ext trigger call back
 *
 * @param user_data data for user input
 */
typedef VOID_T (*TKL_NB_WAKEUP_CB)(PVOID_T user_data);

/**
 * @brief Set config about the paltform
 *
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_plat_config(VOID);

/**
 * @brief Regist the PSM sleep notify cb
 *
 * @param cb notify callback function
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_sleep_notify_reg(TKL_NB_PSM_NOTIFY_CB_T cb);

/**
 * @brief Create a psm manager lock
 *
 * @param lock_name name of the lock
 * @param handle sleeplock handle
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_create_sleeplock(CONST PCHAR_T lock_name, UCHAR_T *handle);

/**
 * @brief Acquire the sleeplock, keep awake
 *
 * @param handle sleeplock handle
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_acquire_sleeplock(UCHAR_T handle);

/**
 * @brief Release the sleeplock, allow psm
 *
 * @param handle sleeplock handle
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_release_sleeplock(UCHAR_T handle);

/**
 * @brief Force enter psm mode
 *
 * @return VOID
 */
VOID tkl_nbiot_psm_force_sleep(VOID);

/**
 * @brief Get the power on reason
 *
 * @param result powerson reason
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_get_poweron_result(TKL_NB_POWER_ON_RESULT_E *result);

/**
 * @brief Get the wake up reason
 *
 * @param src wakeup reason reason
 * @return OPERATE_RET OPRT_OK: success others: fail
 */
OPERATE_RET tkl_nbiot_psm_get_wakeup_source(TKL_NB_WAKEUP_SOURCE_E *src);

/**
 * @brief Create rtc timer
 *
 * @param time_period_sec time period of the timer
 * @param is_periodic if the timer is perodic
 * @param cb timer callback
 * @return OPERATE_RET OPRT_OK: success others: failed
 */
OPERATE_RET tkl_nbiot_psm_rtc_timer_create(UINT_T time_period_sec, BOOL_T is_periodic,TKL_NB_SLP_TIMER_CB_T cb);

/**
 * @brief Start a rtc timer
 *
 * @param null
 * @return OPERATE_RET OPRT_OK: success others: failed
 */
OPERATE_RET tkl_nbiot_psm_rtc_timer_start(VOID);

/**
 * @brief Stop rtc timer
 *
 * @param null
 * @return OPERATE_RET OPRT_OK: success others: failed
 */
OPERATE_RET tkl_nbiot_psm_rtc_timer_stop(VOID);

/**
 * @brief Del rtc timer
 *
 * @param null
 * @return OPERATE_RET OPRT_OK: success others: failed
 */
OPERATE_RET tkl_nbiot_psm_rtc_timer_delete(VOID);

/**
 * @brief Check the usb is active
 *
 * @return BOOL_T true: active false: not active
 */
BOOL_T tkl_nbiot_psm_usb_is_active(VOID);

/**
 * @brief Specify a pin for wakeup trigger
 *
 * @param pin_num  @ref TKL_GPIO_NAME_E
 * @param wakeup_cb call back funtion
 * @return OPERATE_RET OPRT_OK: success others: failed
 */
OPERATE_RET tkl_nbiot_psm_wakeup_pin_cfg(INT_T pin_num, TKL_NB_WAKEUP_CB wakeup_cb);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
