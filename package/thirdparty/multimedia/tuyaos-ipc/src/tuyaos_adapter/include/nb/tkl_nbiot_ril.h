/**
 * @file tkl_nbiot_ril.h
 * @brief Common process - adapter the nbiot ril api
 * 
 * @copyright Copyright 2018-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#ifndef __TKL_NBIOT_RIL_H__
#define __TKL_NBIOT_RIL_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    INT_T sim_identified;
    INT_T sim_no_identified;
    INT_T net_attached;
    INT_T net_detached;
    INT_T net_denied;
} TKL_RIL_EVENT_ID_T;

typedef struct {
    INT_T mcc;    /**< Indicates Mobile Country Code. */
    INT_T mnc;    /**< Indicates Mobile Network Code. */
} TKL_RIL_PLMN_T;

typedef INT_T  (*TKL_RIL_EVENT_SEND_T)(INT_T event_id, PVOID_T param);

typedef struct {
    INT_T rxlev;  /**< The received signal strength level. */
    INT_T ber;   /**< The channel bit error rate. */
    INT_T rscp;  /**< The received signal code power. */
    INT_T ecno;  /**< The ratio of the received energy per PN chip to the total received power spectral density. */
    INT_T rsrq;  /**< The reference signal received quality. */
    INT_T rsrp;  /**< The reference signal received power. */
} TKL_RIL_CESQ_T;

  typedef struct {
      INT_T nc_earfcn;    /**< Indicates the EARFCN for neighbor cell. */
      INT_T nc_earfcn_offset;    /**< Indicates the EARFCN offset for neighbor cell. */
      INT_T nc_pci;    /**< Indicates the neighbor cell physical cell ID. */
      INT_T nc_rsrp;    /**< Indicates neighbor cell RSRP value in units of dBm. */
  } TKL_RIL_NEIGHBOR_CELL_INFO_T;

  /**
  * @brief ril驱动初始化
  * 
  * @param[in] null
  *
  * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
  */
OPERATE_RET tkl_nbiot_ril_init(VOID);

  /**
  * @brief 注册事件回调
  * 
  * @param[in] evt_cb ：注册的相关事件回调函数
  * @param[in] evt_id ：事件定义
  *
  * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
  */
 OPERATE_RET tkl_nbiot_ril_register_event_callback(TKL_RIL_EVENT_SEND_T evt_cb, TKL_RIL_EVENT_ID_T *evt_id);

 /**
 * @brief 查询ril层网络附着状态（建议异步处理）
 * 
 * @param[in] null
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_attach_event_inquire(VOID);

 /**
 * @brief 获取当前设备是否识别到sim卡
 *
 * @return BOOL_T true: 识别到卡 false: 未识别到卡
 */
 BOOL_T tkl_nbiot_ril_is_sim_ready(VOID);

 /**
 * @brief 获取当前设备cesq相关参数（建议阻塞处理）
 * 
 * @param[inout] info ：获取当前设备cesq相关参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_cesq(TKL_RIL_CESQ_T *info);

/**
* @brief 获取设备的T3324时间（建议阻塞处理）
* 
* @param[inout] t3324：返回获取的设备的T3324值，单位：s
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_get_t3324(UINT_T *t3324);

/**
* @brief 获取设备的T3412时间（建议阻塞处理）
* 
* @param[inout] t3412：返回获取的设备的T3412值，单位：s
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_get_t3412(UINT_T *t3412);

 /**
 * @brief 设置设备的T3412时间（建议阻塞处理）
 * 
 * @param[in] req_time ：待设置设备的T3412时间，单位：s
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_set_t3412(UINT_T req_time);

/**
* @brief 设置设备的T3324时间（建议阻塞处理）
* 
* @param[in] req_time ：待设置设备的T3324时间，单位：s
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_set_t3324(UINT_T req_time);

/**
* @brief 设置当前的网络接入APN名称（建议阻塞处理）
* 
* @param[in] apn_name ：设置当前的网络接入APN名称.
* @param[in] pdp_type : 分组数据协议的类型,"IP",IPV6","IPV4V6","Non-IP"
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_set_apn(PCHAR_T apn_name, PCHAR_T pdp_type);

 /**
 * @brief 获取当前的网络接入APN名称（建议阻塞处理）
 * 
 * @param[inout] apn 返回获取的当前的网络接入APN名称
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_apn(PCHAR_T apn);

 /**
 * @brief 获取设备接收信号强度值rssi（建议阻塞处理）
 * 
 * @param[inout] out_rssi 返回的信号强度（为带符号的整型，理论上0为最强,一般为负值）
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_rssi(INT_T *out_rssi);

 /**
 * @brief 获取当前设备的IMSI（建议阻塞处理）
 * 
 * @param[inout] imsi 获取当前设备的IMSI
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_imsi(PCHAR_T imsi);

 /**
 * @brief 获取当前设备的iccid（建议阻塞处理）
 * 
 * @param[inout] iccid 获取当前设备的iccid
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_iccid(PCHAR_T iccid);

 /**
 * @brief 获取相关的国家码及运营商信息（建议阻塞处理）
 * 
 * @param[in] plmn_info 获取到的plmn信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_plmn_info(TKL_RIL_PLMN_T *plmn_info);

/**
* @brief AT CFUN设置（建议阻塞处理）
* 
* @param[in] val ：待设置的CFUN的参数，参考AT CFUN
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_set_cfun(UCHAR_T val);

/**
* @brief 获取当前AT CFUN参数（建议阻塞处理）
* 
* @param[inout] val:返回的当前CFUN参数
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_get_cfun(INT_T *val);

/**
* @brief 获取当前rf是否校准（建议阻塞处理）
* 
* @param[inout] flag:返回的flag,1 已校准，否则未校准
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_get_rf_flag(INT_T *flag);

/**
* @brief 将IMEI写入原厂NV
* 
* @param[in] imei ：将设置的IMEI字符串指针
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_set_imei(PCHAR_T imei);

 /**
 * @brief 从原厂NV获取实际的IMEI
 * 
 * @param[inout] buf : imei string buffer 
 * @param[in] len : buffer length
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_imei(PCHAR_T buf, UCHAR_T len);

/**
* @brief 根据3GPP标准释放RRC，提前退出网络连接态
* 
* @param[in] null
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_release_rrc(VOID);

 /**
 * @brief 设置频点（建议阻塞处理）
 * 
 * @param[in] band_array_str ：待设置的频带，格式举例:'3,5,8'
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_set_band(PCHAR_T band_array_str);

/**
* @brief 获取设备当前的频点（建议阻塞处理）
* 
* @param[inout] pbuf : string buffer，输出格式举例:'3,5,8' 
* @param[in] len : buffer length
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_nbiot_ril_get_band(PCHAR_T pbuf,  UCHAR_T len);

 /**
 * @brief PSM模式配置，效果参考at+cpsms中psm开关（建议阻塞处理）
 * 
 * @param[in] enable : 1，使能；0，失能 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_psm_mode_enable(UCHAR_T enable);

 /**
 * @brief EDRX使能/失能， 效果参考AT+CEDRXS（建议阻塞处理）
 * 
 * @param[in] enable : 1，使能；0，失能 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_edrx_mode_enable(UCHAR_T enable);

 /**
 * @brief 获取当前基站的id（建议阻塞处理）
 * 
 * @param[inout] pbuf : 返回当前连接基站id 字符缓存 
 * @param[in] len : buffer length
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_serving_cellid(PCHAR_T pbuf,  UCHAR_T len);

 /**
 * @brief 获取设备附近基站信息（建议阻塞处理）
 * 
 * @param[inout] pbuf : 返回附近基站信息
 * @param[inout] out_num : 附近基站总数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_nbiot_ril_get_neightbor_cellid(TKL_RIL_NEIGHBOR_CELL_INFO_T **pbuf,  INT_T *out_num);

#ifdef __cplusplus
}
#endif

#endif

