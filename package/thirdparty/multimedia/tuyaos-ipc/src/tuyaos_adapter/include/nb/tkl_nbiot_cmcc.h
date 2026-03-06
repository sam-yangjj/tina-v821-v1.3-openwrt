/**
 * @file tkl_nbiot_cmcc.h
 * @brief Common process - adapter the nbiot cmcc api
 * 
 * @copyright Copyright 2018-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#ifndef __TKL_NBIOT_CMCC_H__
#define __TKL_NBIOT_CMCC_H__

#include "tkl_nbiot_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief cmcc lwm2m register
 * 
 * @param[in] reg_params: register configuration
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_cmcc_register(NBIOT_LWM2M_REGISTER_T *reg_params);

/**
 * @brief cmcc lwm2m deregister
 * 
 * @param[in] null
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_cmcc_deregister(VOID);

/**
 * @brief cmcc data send
 * 
 * @param[in] data:data pointer to be send
 * @param[in] data_len:data length to be send
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_cmcc_data_send(UINT8_T* data, UINT_T data_len);

/**
 * @brief cmcc life time update
 * 
 * @param[in] lifetime: desired lifetime
 *
 * @return  OPRT_OS_ADAPTER_OK: success  Other: fail
 */
OPERATE_RET tkl_nbiot_cmcc_lifetime_update(UINT_T lifetime);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



