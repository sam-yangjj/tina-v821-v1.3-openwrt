/*
 * Copyright (C) 2021 Ty Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include "tkl_motor.h"

#define PAN_MAX_MOTOR_STEP  1450 //机器转动一圈所需的步数，需根据实际产品测量出。
#define TILT_MAX_MOTOR_STEP  140

static int sg_flag = 0;

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n", prog);
    printf(
"  -d <device number>  default 0\n");
    printf("e.g. %s -d 0\n", prog);
}

static int parse_args(int argc, char **argv)
{
    int option_index;
    int c;
    const char short_options[] = "h";
    const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'h':
            print_usage(argv[0]);
	    break;
        default:
            break;
        }
    }

    return 0;
}

void signal_handle(int sig)
{
    printf("get signal: %d\n", sig);

    switch(sig) {
    case SIGINT:
    case SIGKILL:
    case SIGTERM:
        sg_flag = 1;
        break;

    default:
        break;
    }

    return;
}

static void motor_func_example(void)
{
    int state;              /* state variable */
    int pan_pos;            /* horizontal position (in steps) */
    int tilt_pos;           /* vertical position (in steps) */
    int func_num = 0;       /* test function choose */
    int pan_step_period;    /* horizontal step periods (in 100 us) */
    int tilt_step_period;   /* vertical step periodss (in 100 us) */
    OPERATE_RET iret;
    TKL_MOTOR_CMD_E cmd;
    TKL_MOTOR_CMD_PARAM_T cmd_param;

    TKL_MOTOR_INIT_PARAM_T param;
    param.pan_step_max = PAN_MAX_MOTOR_STEP;
    param.tilt_step_max = TILT_MAX_MOTOR_STEP;
    param.pan_roll_step = PAN_MAX_MOTOR_STEP / 2;
    param.tilt_roll_step = TILT_MAX_MOTOR_STEP / 2;

    iret = tkl_motor_init(&param);

    if(iret != OPRT_OK) {
        printf("error:tkl_motor_init\n");
        return;
    }

    memset(&cmd_param,0,sizeof(TKL_MOTOR_CMD_PARAM_T));

    cmd_param.pan_step_periods = 1;
    cmd_param.tilt_step_periods = 1;

    while (!sg_flag) {
        printf("----------\n");
        printf("0  go up\n");
        printf("1  go down\n");
        printf("2  go left\n");
        printf("3  go right\n");
        printf("4  go left_up\n");
        printf("5  go left_down\n");
        printf("6  go right_up\n");
        printf("7  go right_down\n");
        printf("8  go specific position\n");
        printf("----------\n");

        printf("9  all stop\n");
        printf("10 pan stop\n");
        printf("11 tilt stop\n");
        printf("----------\n");

        printf("12 all self check\n");
        printf("13 pan self check\n");
        printf("14 tilt self check\n");
        printf("----------\n");

        printf("15 set pan-tilt speed\n");
        printf("16 get pan-tilt position\n");
        printf("17 get pan-tilt check state\n");
        printf("18 get left-right limit swith flag\n");
        printf("19 clear left right limit swith flag\n");
        printf("----------\n");

        printf("20 exit\n");
        printf("----------\n");
        printf("please enter the function number: \n");

        iret = scanf("%d", &func_num);
        if (iret != 1) {
            printf("scanf error %d\n",iret);
        }
        getchar();

        switch(func_num) {
        case 0:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_UP,&cmd_param);
            break;
        case 1:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_DOWN,&cmd_param);
            break;
        case 2:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT,&cmd_param);
            break;
        case 3:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT,&cmd_param);
            break;
        case 4:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT_UP,&cmd_param);
            break;
        case 5:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_LEFT_DOWN,&cmd_param);
            break;
        case 6:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT_UP,&cmd_param);
            break;
        case 7:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_RIGHT_DOWN,&cmd_param);
            break;

        case 8:
            printf("please input the pan position:");
            iret = scanf("%d", &pan_pos);
            getchar();
            if (iret != 1) {
                printf("scanf error %d\n", iret);
                break;
            }

            printf("please input the tilt position:");
            iret = scanf("%d", &tilt_pos);
            getchar();
            if (iret != 1) {
                printf("scanf error %d\n", iret);
                break;
            }

            cmd_param.pan_pos = pan_pos;
            cmd_param.tilt_pos = tilt_pos;

            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_GO_POS,&cmd_param);

            memset(&cmd_param,0,sizeof(TKL_MOTOR_CMD_PARAM_T));
            cmd_param.pan_step_periods = 1;
            cmd_param.tilt_step_periods = 1;

            break;

        case 9:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_STOP_ALL,&cmd_param);
            break;
		case 10:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_STOP_PAN,&cmd_param);
            break;
        case 11:
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_STOP_TILT,&cmd_param);
            break;

        case 12:
            cmd_param.pan_pos = PAN_MAX_MOTOR_STEP / 2;
            cmd_param.tilt_pos = TILT_MAX_MOTOR_STEP / 2;
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_SELF_CHECK_ALL,&cmd_param);
            break;
        case 13:
            cmd_param.pan_pos = PAN_MAX_MOTOR_STEP / 2;
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_SELF_CHECK_PAN,&cmd_param);
            break;
        case 14:
            cmd_param.tilt_pos = TILT_MAX_MOTOR_STEP / 2;
            iret = tkl_motor_set_cmd(TKL_MOTOR_CMD_SELF_CHECK_TILT,&cmd_param);
            break;
        case 15:
            printf("please input the pan_step_period:");
            iret = scanf("%d", &pan_step_period);
            getchar();
            if (iret != 1) {
                printf("scanf pan speed erro %d\n",iret);
                break;
            }

            printf("please input the tilt_step_period:");
            iret = scanf("%d", &tilt_step_period);
            getchar();
            if (iret != 1) {
                printf("scanf tilt speed erro %d\n",iret);
                break;
            }
            cmd_param.pan_step_periods = pan_step_period;
            cmd_param.tilt_step_periods = tilt_step_period;
            printf("new pan_step_period: %d\n", pan_step_period);
            printf("new tilt_step_period: %d\n", tilt_step_period);
            break;
        case 16:
            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_PAN_POS,&state);

            if(iret == OPRT_OK) {
                printf("pan pos:%d\n",state);
			} else {
                printf("pan pos get fail\n");
			}

            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_TILT_POS,&state);

            if(iret == OPRT_OK) {
                printf("tilt pos:%d\n",state);
			} else {
                printf("tilt pos fail\n");
			}
			break;
        case 17:
            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_PAN_CHECK_STATE,&state);

            if(iret == OPRT_OK) {
                printf("pan state(%d) %s\n",state,state == 1?"at start postion":"on the way");
			} else {
                printf("pan state get fail\n");
			}

            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_TILT_CHECK_STATE,&state);

            if(iret == OPRT_OK) {
                printf("tilt state(%d) %s\n",state,state == 1?"at start postion":"on the way");
			} else {
                printf("tilt state geit fail\n");
			}

            break;

		case 18:
            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_LEFT_OPTO_FLAG,&state);

            if(iret == OPRT_OK) {
                printf("left opto flag %d\n",state);
			} else {
                printf("left opto flag get fail\n");
			}

            iret = tkl_motor_get_state(TKL_MOTOR_STATE_GET_RIGHT_OPTO_FLAG,&state);

            if(iret == OPRT_OK) {
                printf("right opto flag %d\n",state);
			} else {
                printf("right opto flag get fail\n");
			}

            break;
		case 19:
            iret = tkl_motor_get_state(TKL_MOTOR_STATE_SET_CLEAR_OPTO_FLAG,NULL);

            if(iret == OPRT_OK) {
                printf("clear opto flag ok\n");
			} else {
                printf("clear opto flag fail\n");
			}

			break;
		case 20:
        default:
            sg_flag = 1;
            break;
        }

    }

    tkl_motor_deinit();

    return;
}

int main(int argc, char **argv)
{
    int value;
    int ret;

    printf("%s %s %s\n", argv[0], __DATE__, __TIME__);

    parse_args(argc, argv);

    signal(SIGINT, signal_handle);

    motor_func_example();

    return 0;
}
