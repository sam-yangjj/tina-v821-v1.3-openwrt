/*
 * Copyright (C) 2021 Tuya Inc.
 */

#ifndef __TKL_INPUT_EVENT_H__
#define __TKL_INPUT_EVENT_H__

#include <linux/input.h>
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVICES 128 

typedef struct input_event TUYA_INPUT_EVENT_T;

/**
 * @brief Initialize input event device.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
extern OPERATE_RET tkl_input_event_init(VOID_T);

/**
 * @brief Receive input events
 * @param[out]in_evs: input event array
 * @param[out]ev_cnts: the number of input events received
 * @param[in] timeout
 *    -1 : block indefinitely
 *     0  : non-block
 *     >0 : timeout in milliseconds
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

extern OPERATE_RET tkl_input_event_recv(TUYA_INPUT_EVENT_T *in_evs, PINT_T ev_cnts, INT_T ms);

/**
 * @brief De-Initialize input event device.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

extern VOID_T tkl_input_event_deinit(VOID_T);


#ifdef __cplusplus
}
#endif

#endif
