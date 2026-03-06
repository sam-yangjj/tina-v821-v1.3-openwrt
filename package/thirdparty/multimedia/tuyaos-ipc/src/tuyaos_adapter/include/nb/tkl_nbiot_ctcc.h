/**
 * @file tkl_nbiot_ctcc.h
 * @brief Common process - adapter the nbiot ctcc api
 * 
 * @copyright Copyright 2018-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#ifndef __TKL_NBIOT_CTCC_H__
#define __TKL_NBIOT_CTCC_H__

#include "tkl_nbiot_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ctcc lwm2m register
 * 
 * @param[in] reg_params: register configuration
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_ctcc_lwm2m_register(NBIOT_LWM2M_REGISTER_T* reg_params);

/**
 * @brief ctcc lwm2m deregister
 * 
 * @param[in] null
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_ctcc_lwm2m_deregister(VOID);

/**
 * @brief ctcc data send
 * 
 * @param[in] data:data pointer to be send
 * @param[in] data_len:data length to be send
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_ctcc_data_send(UINT8_T* data, UINT_T data_len);

/**
 * @brief ctcc life time update
 * 
 * @param[in] lifetime: desired lifetime
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_ctcc_lifetime_update(UINT_T lifetime);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



