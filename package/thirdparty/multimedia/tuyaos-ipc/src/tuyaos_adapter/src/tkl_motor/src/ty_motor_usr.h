#ifndef __TY_MOTOR_USR_H__
#define __TY_MOTOR_USR_H__

#include <sys/ioctl.h>

struct ty_motor_param_init_s {
    int _pan_roll_step;
    int _pan_step_max;
    int _tilt_step_max;
    int _infi_rot_en;
};

/* current value cmd */
typedef enum {
    GET_PAN_POS,
    GET_TILT_POS,
    GET_PAN_ANGLE,
    GET_TILT_ANGLE,
    GET_PAN_CHECK_STATE,
    GET_TILT_CHECK_STATE,
    GET_PRESET_STATE,
    GET_LEFT_OPTO_FLAG,
    GET_RIGHT_OPTO_FLAG,
    SET_CLEAR_OPTO_FLAG,
} PTZ_STATE_CMD;

typedef struct sunxi_motor_cmd {
    int dir;				/* Direction of rotation for the motor command */
    int ud_step;			/* Number of steps to move the motor in up and down dir */
    int ud_time_per_step;	/* Time duration for each microstep in up and down dir */
    int lr_step;			/* Number of steps to move the motor in left and right dir */
    int lr_time_per_step;	/* Time duration for each microstep in left and right dir */
    int block;				/* block flag for motor commands */
} sunxi_motor_cmd_t;

typedef struct sunxi_motor_device_status {
    int x;	    /* X-coordinate step of the motor device */
    int y;	    /* Y-coordinate step of the motor device */
    int dir;    /* Direction of movement of the motor device */
    int status; /* Motor device status */
} sunxi_motor_device_status_t;

#define DIR_UP			0x01
#define DIR_DOWN  		0x02
#define DIR_UD_SHIFT	0x00
#define DIR_RIGHT		0x01
#define DIR_LEFT		0x02
#define DIR_LR_SHIFT	0x02

#define SET_DIR_UD(x)	(x << DIR_UD_SHIFT)
#define SET_DIR_LR(x) 	(x << DIR_LR_SHIFT)
#define GET_DIR_UD(x) 	((x >> DIR_UD_SHIFT) & 0x03)
#define GET_DIR_LR(x) 	((x >> DIR_LR_SHIFT) & 0x03)
#define GO_POS			1 << 0x04
#define PAN_STOP      	2 << 0x04
#define PAN_SELF_CHECK  3 << 0x04
#define TILT_STOP       4 << 0x04
#define TILT_SELF_CHECK 5 << 0x04

/* ioctl define */
#define TY_MOTOR_DRV_MAGICNUM		'm'
#define TY_MOTOR_IOCTL_STOP_ALL     _IO(TY_MOTOR_DRV_MAGICNUM, 0)
#define TY_MOTOR_IOCTL_STOP_TILT	_IO(TY_MOTOR_DRV_MAGICNUM, 1)
#define TY_MOTOR_IOCTL_STOP_PAN		_IO(TY_MOTOR_DRV_MAGICNUM, 2)
#define TY_MOTOR_IOCTL_SET			_IOW(TY_MOTOR_DRV_MAGICNUM, 6, sunxi_motor_cmd_t)
#define TY_MOTOR_DRV_GET_STATUS                                                   \
    _IOR(TY_MOTOR_DRV_MAGICNUM, 8, sunxi_motor_device_status_t)
#define TY_MOTOR_DRV_SET_STATUS                                                   \
    _IOR(TY_MOTOR_DRV_MAGICNUM, 9, sunxi_motor_device_status_t)

#endif
