/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <errno.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

/* dir mode define */
#define DIR_UP		 0x01
#define DIR_DOWN	 0x02
#define DIR_AB_SHIFT 0x00
#define DIR_RIGHT	 0x01
#define DIR_LEFT	 0x02
#define DIR_CD_SHIFT 0x02

typedef struct sunxi_motor_device_status {
	int x;	    /* X-coordinate step of the motor device */
	int y;	    /* Y-coordinate step of the motor device */
	int dir;    /* Direction of movement of the motor device */
	int status; /* Motor device status */
} sunxi_motor_device_status_t;

typedef struct sunxi_motor_cmd {
	int dir;			  /* Direction of rotation for the motor command */
	int ab_step;		  /* Number of steps to move the motor in up and down dir */
	int ab_time_per_step; /* Time duration for each microstep in up and down dir */
	int cd_step;		  /* Number of steps to move the motor in left and right dir */
	int cd_time_per_step; /* Time duration for each microstep in left and right dir */
	int block;			  /* block flag for motor commands */
} sunxi_motor_cmd_t;

/* ioctl define */
#define MOTOR_DRV_MAGICNUM     'm'
#define MOTOR_DRV_SET_STOP     _IO(MOTOR_DRV_MAGICNUM, 0)
#define MOTOR_DRV_SET_STOP_AB  _IO(MOTOR_DRV_MAGICNUM, 1)
#define MOTOR_DRV_SET_STOP_CD  _IO(MOTOR_DRV_MAGICNUM, 2)
#define MOTOR_DRV_SET_START    _IO(MOTOR_DRV_MAGICNUM, 3)
#define MOTOR_DRV_SET_BLOCK    _IOW(MOTOR_DRV_MAGICNUM, 4, int)
#define MOTOR_DRV_SET_PARA     _IOW(MOTOR_DRV_MAGICNUM, 5, sunxi_motor_para_t)
#define MOTOR_DRV_SET_CMD      _IOW(MOTOR_DRV_MAGICNUM, 6, sunxi_motor_cmd_t)
#define MOTOR_DRV_GET_CMD      _IOR(MOTOR_DRV_MAGICNUM, 7, sunxi_motor_cmd_t)
#define MOTOR_DRV_GET_STATUS                                                   \
	_IOR(MOTOR_DRV_MAGICNUM, 8, sunxi_motor_device_status_t)
#define MOTOR_DRV_SET_STATUS                                                   \
	_IOR(MOTOR_DRV_MAGICNUM, 9, sunxi_motor_device_status_t)

void show_usage(void)
{
	printf("Usage: gpio_motor <mode> <direction> <steps> <time_per_step> <block>\n");
	printf("    - mode:\n");
	printf("        0: Control via file IO\n");
	printf("        1: Control via IOCTL\n");
	printf("        2: Terminate non-blocking movement\n");
	printf("        3: Get movement parameters\n");
	printf("    - direction:\n");
	printf("        1: DIR_UP\n");
	printf("        2: DIR_DOWN\n");
	printf("        3: DIR_RIGHT\n");
	printf("        4: DIR_LEFT\n");
	printf("        5: DIR_LEFT_UP\n");
	printf("        6: DIR_RIGHT_UP\n");
	printf("        7: DIR_LEFT_DOWN\n");
	printf("        8: DIR_RIGHT_DOWN\n");
	printf("    - block:\n");
	printf("        0: Non-blocking mode\n");
	printf("        1: Blocking mode\n");
	printf("\nExample:\n");
	printf("gpio_motor 0 1 100 1 1\n");
	printf("    - Use file system mode\n");
	printf("    - Direction: up\n");
	printf("    - Move 100 steps\n");
	printf("    - Time per step: 1000us\n");
	printf("    - Blocking mode\n");
}

int main(int argc, char *argv[])
{
	int fd, mode = 0;
	sunxi_motor_device_status_t status;
	sunxi_motor_cmd_t cmd;

	if (argc != 6) {
		show_usage();
		return 1;
	}

	mode		  = atoi(argv[1]);
	cmd.ab_step	  = atoi(argv[3]);
	cmd.ab_time_per_step = atoi(argv[4]);
	cmd.cd_step	  = atoi(argv[3]);
	cmd.cd_time_per_step = atoi(argv[4]);
	cmd.block	  = atoi(argv[5]);

	switch (atoi(argv[2])) {
		case 1:
			cmd.dir	= DIR_UP << DIR_AB_SHIFT;
			break;
		case 2:
			cmd.dir	= DIR_DOWN << DIR_AB_SHIFT;
			break;
		case 3:
			cmd.dir	= DIR_RIGHT << DIR_CD_SHIFT;
			break;
		case 4:
			cmd.dir	= DIR_LEFT << DIR_CD_SHIFT;
			break;
		case 5:
			cmd.dir	= (DIR_LEFT << DIR_CD_SHIFT) | (DIR_UP << DIR_AB_SHIFT);
			break;
		case 6:
			cmd.dir	= (DIR_RIGHT << DIR_CD_SHIFT) | (DIR_UP << DIR_AB_SHIFT);
			break;
		case 7:
			cmd.dir	= (DIR_LEFT << DIR_CD_SHIFT) | (DIR_DOWN << DIR_AB_SHIFT);
			break;
		case 8:
			cmd.dir	= (DIR_RIGHT << DIR_CD_SHIFT) | (DIR_DOWN << DIR_AB_SHIFT);
			break;
		default:
			cmd.dir	= 0;
			break;
	}

	fd = open("/dev/sunxi_gpio_motor", O_RDWR);
	if (fd == -1) {
		perror("Failed to open the device node");
		return 1;
	}

	if (mode == 0) {
		if (write(fd, &cmd, sizeof(sunxi_motor_cmd_t)) < 0) {
			perror("Failed to write to the device...");
			close(fd);
			return -1;
		}
	} else if (mode == 1) {
		if (ioctl(fd, MOTOR_DRV_SET_CMD, &cmd) == -1) {
			perror("Failed to perform ioctl MOTOR_DRV_SET_DATA");
			close(fd);
			return errno;
		}
	} else if (mode == 2) {
		if (ioctl(fd, MOTOR_DRV_SET_STOP, NULL) == -1) {
			perror("Failed to perform ioctl MOTOR_DRV_GET_DATA");
			close(fd);
			return errno;
		}
	} else {
		if (read(fd, &status, sizeof(sunxi_motor_device_status_t)) !=
		    sizeof(sunxi_motor_device_status_t)) {
			perror("Failed to read status from the device node");
			close(fd);
			return 1;
		}

		printf("x: %d\n", status.x);
		printf("y: %d\n", status.y);
		printf("dir: %d\n", status.dir);
		printf("status: %d\n", status.status);
	}

	if (read(fd, &status, sizeof(sunxi_motor_device_status_t)) !=
	    sizeof(sunxi_motor_device_status_t)) {
		perror("Failed to read status from the device node");
		close(fd);
		return 1;
	}

	printf("x: %d\n", status.x);
	printf("y: %d\n", status.y);
	printf("dir: %d\n", status.dir);
	printf("status: %d\n", status.status);

	close(fd);
	return 0;
}
