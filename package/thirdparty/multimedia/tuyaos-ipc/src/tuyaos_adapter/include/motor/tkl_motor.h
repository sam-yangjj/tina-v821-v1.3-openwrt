/*
 * Copyright (C) 2021 Tuya Inc.
 */

#ifndef __TKL_MOTOR_H__
#define __TKL_MOTOR_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* motor device attribute */
typedef struct {
    int pan_step_max;   /* maximum number of horizontal steps */
    int tilt_step_max;  /* maximum number of vertical steps */
    int pan_roll_step;  /* horizontal rollback position (in steps) */
    int tilt_roll_step;  /* tilt rollback position (in steps) */
    int infi_rot_en;   /* infinity rotation enable */
}TKL_MOTOR_INIT_PARAM_T;

/* motor command ID */
typedef enum {
    /* move towards the specified direction */
    TKL_MOTOR_CMD_GO_UP = 0,
    TKL_MOTOR_CMD_GO_DOWN,
    TKL_MOTOR_CMD_GO_LEFT,
    TKL_MOTOR_CMD_GO_RIGHT,
    TKL_MOTOR_CMD_GO_LEFT_UP,
    TKL_MOTOR_CMD_GO_LEFT_DOWN,
    TKL_MOTOR_CMD_GO_RIGHT_UP,
    TKL_MOTOR_CMD_GO_RIGHT_DOWN,
    /* move to the specified position */
    TKL_MOTOR_CMD_GO_POS,
    /* stop */
    TKL_MOTOR_CMD_STOP_ALL,   /* stop horizontal and vertical movement */
    TKL_MOTOR_CMD_STOP_PAN,         /* stop horizontal movement */
    TKL_MOTOR_CMD_STOP_TILT,        /* stop vertical movement */

    /* self check */
    TKL_MOTOR_CMD_SELF_CHECK_ALL, /* horizontal and vertical self-check */
    TKL_MOTOR_CMD_SELF_CHECK_PAN,       /* horizontal self-check */
    TKL_MOTOR_CMD_SELF_CHECK_TILT,      /* vertical self-check */

} TKL_MOTOR_CMD_E;


/* motor command parameter */
typedef struct {
    /* for TKL_MOTOR_CMD_GO_xxxxxx */
    int pan_step_periods;    /* horizontal one step  occupy numbers of period ( 100 us one period ) */
    int tilt_step_periods;   /* vertical one step  occupy numbers of period (100 us one period) */
    /* for TKL_MOTOR_CMD_GO_POS */
    int pan_pos;    /* horizontal position (in steps) */
    int tilt_pos;   /* vertical position (in steps) */

}TKL_MOTOR_CMD_PARAM_T;


/* current value cmd */
typedef enum {
    TKL_MOTOR_STATE_GET_PAN_POS, /*get pan positon*/
    TKL_MOTOR_STATE_GET_TILT_POS,/*get tilt position*/
    TKL_MOTOR_STATE_GET_PAN_ANGLE,/*git pan angle */
    TKL_MOTOR_STATE_GET_TILT_ANGLE, /* get tile angle*/
    TKL_MOTOR_STATE_GET_PAN_CHECK_STATE, /*get pan check state,1,at starting point(pos=0) ;0,not at starting point*/
    TKL_MOTOR_STATE_GET_TILT_CHECK_STATE, /*get tile check state,1,at starting point(pos=0);0,not at starting point */
    TKL_MOTOR_STATE_GET_PRESET_STATE,   /* current not used */
    TKL_MOTOR_STATE_GET_LEFT_OPTO_FLAG, /*get left limited switch flag */
    TKL_MOTOR_STATE_GET_RIGHT_OPTO_FLAG,/*get right limited switch flag */
    TKL_MOTOR_STATE_SET_CLEAR_OPTO_FLAG,/*cleaer limited switch flag */
}TKL_MOTOR_STATE_CMD_E;

/** Initialize a motor device.
 * @param dev device number.
 * @param init param.If `param` is NULL, default init param will be used.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_motor_init(TKL_MOTOR_INIT_PARAM_T *param);

/** Deinitialize a motor device.
 * @param dev device number.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_motor_deinit(void);

/** Perform a motor operation command.
 * @param cmd operation command.
 * @param cmd operation command parameter.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_motor_set_cmd(TKL_MOTOR_CMD_E cmd,TKL_MOTOR_CMD_PARAM_T *param);

/** Get motor device status.
 * @param stat cmd
 * @param stat  state
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_motor_get_state(TKL_MOTOR_STATE_CMD_E cmd, int *state);


#ifdef __cplusplus
}
#endif

#endif
