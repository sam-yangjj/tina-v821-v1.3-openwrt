/**
 * @file tkl_ir.h
 * @brief Common process - adapter the ir api
 * @version 0.1
 * @date 2021-08-06
 *
 * @copyright Copyright 2021-2022 Ty Inc. All Rights Reserved.
 *
 */

#ifndef __TKL_IR_H__
#define __TKL_IR_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ir id type 
 */
typedef enum {
    IR_ID_0 = 0,          // IR 0
    IR_ID_1 = 1,          // IR 1
    IR_ID_2 = 2,          // IR 2 
    IR_ID_3 = 3,          // IR 3 
    IR_ID_4 = 4,          // IR 4 
    IR_ID_5 = 5,          // IR 5 
    IR_ID_6 = 6,          // IR 6 
    IR_ID_7 = 7,          // IR 7 
    IR_ID_MAX ,           // IR MAX
} TY_IR_ID_E;

/** data for store IR space and pulse time value
 */
typedef struct {
    int *buf;              /**< space and pulse time buffer*/
    unsigned int cnts;     /**<  real data count */
    unsigned int size;      /**< buffer size */
}TY_IR_BUF_T;

/**
 * @brief ir init 
 * @param[in]id:ir id 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
extern OPERATE_RET tkl_ir_init(TY_IR_ID_E id);

/**
 * @brief ir deinit 
 * @param[in]id:ir id 
 * @return none
 */
extern void tkl_ir_deinit(TY_IR_ID_E id);

/**
 * @brief send IR code 
 * @param[in]id: ir id
 * @param[out]code:ir code buf
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
extern OPERATE_RET tkl_ir_send(TY_IR_ID_E id, TY_IR_BUF_T *code); 

/**
 * @brief receive IR code 
 * @param[in]id: ir id
 * @param[out]code:ir code buf
 * @param[in] timeout: -1 is blocked, value is micro seconds
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

extern OPERATE_RET tkl_ir_recv(TY_IR_ID_E id, TY_IR_BUF_T *code, INT_T ms);

/**
 * @brief set frequency of tx carrier
 * @param[in]id: ir id
 * @param[in]:frequency of the carrier to be set, in Hz
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
extern OPERATE_RET tkl_ir_set_tx_carrier(TY_IR_ID_E id, UINT_T frequency);


#ifdef __cplusplus
}
#endif

#endif
