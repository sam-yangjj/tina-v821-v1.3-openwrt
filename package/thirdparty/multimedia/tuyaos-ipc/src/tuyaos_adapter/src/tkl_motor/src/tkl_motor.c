/*
* Copyright (C) 2021 Ty Inc.
*/

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <signal.h>
#include <pthread.h>
#include "tkl_motor.h"
#include "ty_motor_usr.h"

#define TKL_MOTOR_DEV_NAME  "/dev/sunxi_gpio_motor"

#ifndef TAG
#define TAG "tkl_motor"
#endif
#define MAX_MOTOR_STEP  512
#define MAX_BASE_PERIOD 10
#define MIN_BASE_PERIOD 1
#define MOTOR_STOP 1
static int s_motor_fd = -1;
struct ty_motor_param_init_s init_param;

OPERATE_RET tkl_motor_init(TKL_MOTOR_INIT_PARAM_T *param)
{
	int iret;
	int fd = -1;;
	TKL_MOTOR_CMD_PARAM_T cmd_param;
	fd = open(TKL_MOTOR_DEV_NAME, O_RDWR);

	if(fd < 0) {
		printf("open %s fail\n", TKL_MOTOR_DEV_NAME);
		return OPRT_COM_ERROR;
	}

	s_motor_fd = fd;
	memset(&init_param, 0, sizeof(struct ty_motor_param_init_s));
	memset(&cmd_param, 0, sizeof(TKL_MOTOR_CMD_PARAM_T));

	if (param == NULL) {
		printf("used default param\n");
		init_param._pan_step_max = MAX_MOTOR_STEP;
		init_param._tilt_step_max = MAX_MOTOR_STEP;
		cmd_param.pan_step_periods = MIN_BASE_PERIOD;
		cmd_param.tilt_step_periods = MIN_BASE_PERIOD;
		cmd_param.pan_pos = MAX_MOTOR_STEP / 2;
		cmd_param.tilt_pos = MAX_MOTOR_STEP / 2;
	} else {
		init_param._pan_step_max = param->pan_step_max;
		init_param._tilt_step_max = param->tilt_step_max;
		cmd_param.pan_step_periods = MIN_BASE_PERIOD;
		cmd_param.tilt_step_periods = MIN_BASE_PERIOD;
		cmd_param.pan_pos = param->pan_roll_step;
		cmd_param.tilt_pos = param->tilt_roll_step;
	}

	iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_SELF_CHECK_ALL, &cmd_param);

	if (iret != 0) {
		printf("fail to set init param\n");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

OPERATE_RET tkl_motor_deinit(void)
{
if (s_motor_fd > 0) {
	close(s_motor_fd);
	s_motor_fd = -1;
}

return OPRT_OK;
}

static OPERATE_RET __tkl_motor_set_cmd(int fd, unsigned int ioctl_cmd, unsigned char func_cmd, TKL_MOTOR_CMD_PARAM_T *param) 
{
	int iret;
	struct sunxi_motor_cmd cmd_param;
	sunxi_motor_device_status_t status;
	int ud_step, lr_step;

	memset(&cmd_param, 0, sizeof(struct sunxi_motor_cmd));
	memset(&status, 0, sizeof(sunxi_motor_device_status_t));

	if (read(fd, &status, sizeof(sunxi_motor_device_status_t)) !=
		sizeof(sunxi_motor_device_status_t)) {
		close(fd);
	} else {
		ud_step = status.y / 8;
		lr_step = status.x / 8;
	}

	cmd_param.block = 0;
	if (param != NULL) {
		if (func_cmd & 0x0f) {
			if (GET_DIR_UD(func_cmd) == DIR_UP) {
				cmd_param.dir |= DIR_UP << DIR_UD_SHIFT;
				cmd_param.ud_step = ud_step;
			} else if (GET_DIR_UD(func_cmd) == DIR_DOWN) {
				cmd_param.dir |= DIR_DOWN << DIR_UD_SHIFT;
				cmd_param.ud_step = init_param._tilt_step_max - ud_step;
			}

			if (GET_DIR_LR(func_cmd) == DIR_RIGHT) {
				cmd_param.dir |= DIR_RIGHT << DIR_LR_SHIFT;
				cmd_param.lr_step = init_param._pan_step_max - lr_step;
			} else if (GET_DIR_LR(func_cmd) == DIR_LEFT) {
				cmd_param.dir |= DIR_LEFT << DIR_LR_SHIFT;
				cmd_param.lr_step = lr_step;
			}
		} else {
			switch (func_cmd & 0xf0) {
				case GO_POS:
					if (param->tilt_pos > init_param._tilt_step_max || param->tilt_pos < 0 ||
							param->pan_pos > init_param._pan_step_max || param->pan_pos < 0) {
						return OPRT_COM_ERROR;
					}

					if (ud_step > param->tilt_pos) {
						cmd_param.dir |= DIR_UP << DIR_UD_SHIFT;
						cmd_param.ud_step = ud_step - param->tilt_pos;
					} else if (ud_step < param->tilt_pos) {
						cmd_param.dir |= DIR_DOWN << DIR_UD_SHIFT;
						cmd_param.ud_step = param->tilt_pos - ud_step;
					}

					if (lr_step > param->pan_pos) {
						cmd_param.dir |= (DIR_LEFT << DIR_LR_SHIFT);
						cmd_param.lr_step = lr_step - param->pan_pos;
					} else if (lr_step < param->pan_pos) {
						cmd_param.dir |= (DIR_RIGHT << DIR_LR_SHIFT);
						cmd_param.lr_step = param->pan_pos - lr_step;
					}
					break;
				case PAN_SELF_CHECK:
					cmd_param.dir = (DIR_LEFT << DIR_LR_SHIFT);
					cmd_param.lr_step = init_param._pan_step_max;
					cmd_param.lr_time_per_step = MIN_BASE_PERIOD;
					cmd_param.block = 1;
					iret = ioctl(fd,ioctl_cmd, &cmd_param);
					if (iret != 0) {
						goto out;
					}
					status.x = 0;
					iret = ioctl(fd,TY_MOTOR_DRV_SET_STATUS, &status);
					if (iret != 0) {
						goto out;
					}
					cmd_param.dir = (DIR_RIGHT << DIR_LR_SHIFT);
					cmd_param.lr_step = param->pan_pos;
					break;
				case TILT_SELF_CHECK:
					cmd_param.dir = (DIR_UP << DIR_UD_SHIFT);
					cmd_param.ud_step = init_param._tilt_step_max;
					cmd_param.ud_time_per_step = MIN_BASE_PERIOD;
					cmd_param.block = 1;
					iret = ioctl(fd,ioctl_cmd, &cmd_param);
					if (iret != 0) {
						goto out;
					}
					status.y = 0;
					iret = ioctl(fd,TY_MOTOR_DRV_SET_STATUS, &status);
					if (iret != 0) {
						goto out;
					}
					cmd_param.dir = (DIR_DOWN << DIR_UD_SHIFT);
					cmd_param.ud_step = param->tilt_pos;
					break;
			}
		}
		cmd_param.ud_time_per_step = param->tilt_step_periods;
		cmd_param.lr_time_per_step = param->pan_step_periods;
	}

	iret = ioctl(fd,ioctl_cmd, &cmd_param);

out:
	if (iret != 0) {
		printf("ioctl fail\n");
		iret = OPRT_COM_ERROR;
	} else {
		iret = OPRT_OK;
	}

	return iret;
}

OPERATE_RET tkl_motor_set_cmd(TKL_MOTOR_CMD_E cmd, TKL_MOTOR_CMD_PARAM_T *param)
{
	int fd = s_motor_fd;
	int iret = OPRT_COM_ERROR;

	if (fd <= 0) {
		printf("motor is not init\n");
		return OPRT_COM_ERROR;
	}

	if (cmd != TKL_MOTOR_CMD_STOP_ALL &&  param == NULL) {
		printf("invalid param\n");
		return OPRT_INVALID_PARM;
	}

	switch(cmd) {

		case TKL_MOTOR_CMD_GO_UP:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_UD(DIR_UP), param);
			break;
		case TKL_MOTOR_CMD_GO_DOWN:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_UD(DIR_DOWN), param);
			break;
		case TKL_MOTOR_CMD_GO_LEFT:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_LEFT), param);
			break;
		case TKL_MOTOR_CMD_GO_RIGHT:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_RIGHT), param);
			break;
		case TKL_MOTOR_CMD_GO_LEFT_UP:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_LEFT) | SET_DIR_UD(DIR_UP), param);
			break;
		case TKL_MOTOR_CMD_GO_LEFT_DOWN:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_LEFT) | SET_DIR_UD(DIR_DOWN), param);
			break;
		case TKL_MOTOR_CMD_GO_RIGHT_UP:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_RIGHT) | SET_DIR_UD(DIR_UP), param);
			break;
		case TKL_MOTOR_CMD_GO_RIGHT_DOWN:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, SET_DIR_LR(DIR_RIGHT) | SET_DIR_UD(DIR_DOWN), param);
			break;
		case TKL_MOTOR_CMD_GO_POS:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, GO_POS, param);
			break;
		case TKL_MOTOR_CMD_STOP_ALL:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_STOP_ALL, GO_POS, NULL);
			break;
		case TKL_MOTOR_CMD_STOP_PAN:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_STOP_PAN, PAN_STOP, NULL);
			break;
		case TKL_MOTOR_CMD_STOP_TILT:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_STOP_TILT, TILT_STOP, NULL);
			break;
		case TKL_MOTOR_CMD_SELF_CHECK_ALL:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, PAN_SELF_CHECK,param);

			if (iret == OPRT_OK) {
				iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, TILT_SELF_CHECK,param);
			}

			break;
		case TKL_MOTOR_CMD_SELF_CHECK_PAN:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, PAN_SELF_CHECK,param);
			break;
		case TKL_MOTOR_CMD_SELF_CHECK_TILT:
			iret = __tkl_motor_set_cmd(fd, TY_MOTOR_IOCTL_SET, TILT_SELF_CHECK,param);
			break;
		default:
			printf("cmd: %d not supported\n", cmd);
			break;
	}

	return iret;

}

static OPERATE_RET __tkl_motor_get_state(int fd, PTZ_STATE_CMD cmd, int *state)
{
	int iret;

	sunxi_motor_device_status_t param;

	memset(&param, 0, sizeof(sunxi_motor_device_status_t));

	iret = ioctl(fd, TY_MOTOR_DRV_GET_STATUS, &param);

	if(iret != 0) {
		printf("ioctl fail\n"); 
	return OPRT_COM_ERROR;
	}

	int ud_step = param.y / 8;
	int lr_step = param.x / 8;
	if(state) {
		switch (cmd) {
			case GET_PAN_POS:
				*state = lr_step;
				break;
			case GET_PAN_CHECK_STATE:
				*state = lr_step ? 0 : 1;
				break;
			case GET_TILT_POS:
				*state = ud_step;
				break;
			case GET_TILT_CHECK_STATE:
				*state = ud_step ? 0 : 1;
				break;
			case TKL_MOTOR_STATE_GET_LEFT_OPTO_FLAG:
				*state = lr_step == 0 ? 1 : 0;
				break;
			case TKL_MOTOR_STATE_GET_RIGHT_OPTO_FLAG:
				*state = lr_step == init_param._pan_step_max ? 1 : 0;
				break;
		}
	}

	return OPRT_OK;

}

OPERATE_RET tkl_motor_get_state(TKL_MOTOR_STATE_CMD_E cmd, int *state)
{
	int iret = OPRT_COM_ERROR;
	int fd = s_motor_fd;

	if (fd <= 0) {
		printf("motor is not init\n");
		return OPRT_COM_ERROR;
	}

	if(cmd != TKL_MOTOR_STATE_SET_CLEAR_OPTO_FLAG && state == NULL) {
		printf("invalid param\n");
		return OPRT_INVALID_PARM;
	}

	switch(cmd) {

		case TKL_MOTOR_STATE_GET_PAN_POS:
			iret = __tkl_motor_get_state(fd, GET_PAN_POS, state);
			break;

		case TKL_MOTOR_STATE_GET_TILT_POS:
			iret = __tkl_motor_get_state(fd, GET_TILT_POS, state);
			break;

		case TKL_MOTOR_STATE_GET_PAN_CHECK_STATE:
			iret = __tkl_motor_get_state(fd, GET_PAN_CHECK_STATE, state);
			break;

		case TKL_MOTOR_STATE_GET_TILT_CHECK_STATE:
			iret = __tkl_motor_get_state(fd, GET_TILT_CHECK_STATE, state);
			break;

		case TKL_MOTOR_STATE_GET_LEFT_OPTO_FLAG:
			iret = __tkl_motor_get_state(fd, GET_LEFT_OPTO_FLAG, state);
			break;

		case TKL_MOTOR_STATE_GET_RIGHT_OPTO_FLAG:
			iret = __tkl_motor_get_state(fd, GET_RIGHT_OPTO_FLAG, state);
			break;

		case TKL_MOTOR_STATE_SET_CLEAR_OPTO_FLAG:
			iret = __tkl_motor_get_state(fd, SET_CLEAR_OPTO_FLAG, NULL);
			break;

		default:
			printf("cmd: %d not supported\n", cmd);
			break;

	}

	return iret;
}
