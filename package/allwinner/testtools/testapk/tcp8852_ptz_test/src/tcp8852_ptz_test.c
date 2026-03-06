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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <misc/uapi_tcp8852.h>

#define DEV_NAME "/dev/tcp8852"

// 全局变量存储用户选择的通道和运动控制
static uint8_t selected_channel = CH12; // 默认使用CH12通道
static uint8_t selected_action = FOREWARD; // 默认使用正转

static void Usage(void)
{
	printf("Usage: tcp8852_ptz_test [option]\n");
	printf("    -h help.\n");
	printf("    -i get TCP8852 ID.\n");
	printf("    -a set TCP8852 init and auto mode.\n");
	printf("    -d set TCP8852 init and direct out mode.\n");
	printf("    -b set TCP8852 init and DC-Brush mode.\n");
	printf("    -s set TCP8852 init and 1-2 phase mode.\n");
	printf("    -r set TCP8852 init and 2-2 phase mode.\n");
	printf("    -l set TCP8852 init and IRCUT DC Motor.\n");
	printf("    -p set TCP8852 init and AbsolutePosition mode.\n");
	printf("    -o set TCP8852 init and RelativePosition mode.\n");
	printf("    -c <channel> select channel (1, 2, 12, 3, 4, 34).\n");
	printf("    -m <action> set motor action (0: OPEN_CIRCUIT, 1: FOREWARD, 2: INVERSION, 3: BRAKE).\n");
}

// 初始化自动控制模式
static int initAutoControl(int channel, int micro_step, int clock_pre_div, int clock_div, int speed_pre_div, int speed_div,
			   uint16_t speed_count)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_clock_cmd clock_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = AUTO;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置细分数
	ch_cmd.value = micro_step;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_MICROSTEP, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set micro step: %s\n", strerror(errno));
		return ret;
	}

	// 设置时钟分频
	clock_cmd.channel = channel;
	clock_cmd.pre_div = clock_pre_div;
	clock_cmd.div	  = clock_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_CLOCK, &clock_cmd);
	if (ret < 0) {
		printf("Failed to set clock div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化直接控制模式
static int initDirectControl(int channel)
{
	struct tcp8852_channel_cmd ch_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = DIRECT_OUT;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化BLDC控制模式
static int initBldcControl(int channel, int speed_pre_div, int speed_div, uint16_t speed_count)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = BLDC;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化1-2相位控制模式
static int init1_2PhaseControl(int channel, int clock_pre_div, int clock_div, int speed_pre_div, int speed_div, uint16_t speed_count,
			       uint16_t phase, uint16_t cycle)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_clock_cmd clock_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = PHASE_1_2;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置时钟分频
	clock_cmd.channel = channel;
	clock_cmd.pre_div = clock_pre_div;
	clock_cmd.div	  = clock_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_CLOCK, &clock_cmd);
	if (ret < 0) {
		printf("Failed to set clock div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	// 设置相位
	ch_cmd.value = phase;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_PHASE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set phase: %s\n", strerror(errno));
		return ret;
	}

	// 设置圈数
	ch_cmd.value = cycle;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_CYCLE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set cycle: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化2-2相位控制模式
static int init2_2PhaseControl(int channel, int clock_pre_div, int clock_div, int speed_pre_div, int speed_div, uint16_t speed_count,
			       uint16_t phase, uint16_t cycle)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_clock_cmd clock_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = PHASE_2_2;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置时钟分频
	clock_cmd.channel = channel;
	clock_cmd.pre_div = clock_pre_div;
	clock_cmd.div	  = clock_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_CLOCK, &clock_cmd);
	if (ret < 0) {
		printf("Failed to set clock div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	// 设置相位
	ch_cmd.value = phase;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_PHASE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set phase: %s\n", strerror(errno));
		return ret;
	}

	// 设置圈数
	ch_cmd.value = cycle;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_CYCLE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set cycle: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化IRCUT控制
static int initIRCUTAction(int action)
{
	struct tcp8852_ircut_cmd ir_cmd;
	int ret;

	ir_cmd.action = action;
	ir_cmd.enable = 1;
	ret	      = ioctl(0, TCP8852_SET_IR_CUT, &ir_cmd);
	if (ret < 0) {
		printf("Failed to set IRCUT: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化绝对位置控制模式
static int initAbsolutePositionControl(int channel, int micro_step, int clock_pre_div, int clock_div, int speed_pre_div, int speed_div,
				       uint16_t speed_count, uint16_t phase, uint16_t cycle)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_clock_cmd clock_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = ABSOLUTE_POSITION;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置细分数
	ch_cmd.value = micro_step;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_MICROSTEP, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set micro step: %s\n", strerror(errno));
		return ret;
	}

	// 设置时钟分频
	clock_cmd.channel = channel;
	clock_cmd.pre_div = clock_pre_div;
	clock_cmd.div	  = clock_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_CLOCK, &clock_cmd);
	if (ret < 0) {
		printf("Failed to set clock div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	// 设置相位
	ch_cmd.value = phase;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_PHASE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set phase: %s\n", strerror(errno));
		return ret;
	}

	// 设置圈数
	ch_cmd.value = cycle;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_CYCLE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set cycle: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 初始化相对位置控制模式
static int initRelativePositionControl(int channel, int micro_step, int clock_pre_div, int clock_div, int speed_pre_div, int speed_div,
				       uint16_t speed_count, uint16_t phase, uint16_t cycle)
{
	struct tcp8852_channel_cmd ch_cmd;
	struct tcp8852_clock_cmd clock_cmd;
	struct tcp8852_speed_cmd speed_cmd;
	int ret;

	// 设置通道模式
	ch_cmd.channel = channel;
	ch_cmd.value   = RELATIVE_POSITION;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_MODE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel mode: %s\n", strerror(errno));
		return ret;
	}

	// 设置细分数
	ch_cmd.value = micro_step;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_MICROSTEP, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set micro step: %s\n", strerror(errno));
		return ret;
	}

	// 设置时钟分频
	clock_cmd.channel = channel;
	clock_cmd.pre_div = clock_pre_div;
	clock_cmd.div	  = clock_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_CLOCK, &clock_cmd);
	if (ret < 0) {
		printf("Failed to set clock div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度分频
	speed_cmd.channel = channel;
	speed_cmd.pre_div = speed_pre_div;
	speed_cmd.div	  = speed_div;
	ret		  = ioctl(0, TCP8852_SET_CHANNEL_SPEED, &speed_cmd);
	if (ret < 0) {
		printf("Failed to set speed div: %s\n", strerror(errno));
		return ret;
	}

	// 设置速度计数
	ch_cmd.value = speed_count;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_SPEEDCNT, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set speed count: %s\n", strerror(errno));
		return ret;
	}

	// 设置相位
	ch_cmd.value = phase;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_PHASE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set phase: %s\n", strerror(errno));
		return ret;
	}

	// 设置圈数
	ch_cmd.value = cycle;
	ret	     = ioctl(0, TCP8852_SET_CHANNEL_CYCLE, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set cycle: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 设置通道动作
static int setChannelAction(int channel, int action)
{
	struct tcp8852_channel_cmd ch_cmd;
	int ret;

	ch_cmd.channel = channel;
	ch_cmd.value   = action;
	ret	       = ioctl(0, TCP8852_SET_CHANNEL_ACTION, &ch_cmd);
	if (ret < 0) {
		printf("Failed to set channel action: %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

// 获取芯片ID
static uint16_t getID(void)
{
	uint16_t id = 0;
	int ret;

	ret = ioctl(0, TCP8852_GET_CHIP_ID, &id);
	if (ret < 0) {
		printf("Failed to get chip ID: %s\n", strerror(errno));
	}

	return id;
}

int main(int argc, char **argv)
{
	// 打开设备文件
	int fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s: %s\n", DEV_NAME, strerror(errno));
		return -1;
	}

	// 将文件描述符复制到标准输入，以便函数使用
	dup2(fd, 0);

	int c;
	while ((c = getopt(argc, argv, "hiadbsrlpo:c:m:")) != EOF) {
		switch (c) {
		case 'i': {
			uint16_t id = getID();
			printf("ID:0x%04X\n", id);
		} break;
		case 'c': {
			// 设置通道
			int channel = atoi(optarg);
			switch (channel) {
			case 1:
				selected_channel = CH1;
				break;
			case 2:
				selected_channel = CH2;
				break;
			case 12:
				selected_channel = CH12;
				break;
			case 3:
				selected_channel = CH3;
				break;
			case 4:
				selected_channel = CH4;
				break;
			case 34:
				selected_channel = CH34;
				break;
			default:
				printf("Invalid channel: %d\n", channel);
				Usage();
				return -1;
			}
			printf("Selected channel: %d\n", channel);
		} break;
		case 'm': {
			// 设置运动控制
			int action = atoi(optarg);
			if (action >= 0 && action <= 3) {
				selected_action = action;
				printf("Set motor action: %d\n", action);
			} else {
				printf("Invalid motor action: %d\n", action);
				Usage();
				return -1;
			}
		} break;
		case 'a': {
			initAutoControl(selected_channel, MSTEP_DIVIDE_64, CLOCK_PRE_DIVIDE_64, CLOCK_DIVIDE_1, SPEED_PRE_DIVIDE_64, SPEED_DIVIDE_1,
					0x0040);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'd': {
			initDirectControl(selected_channel);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'b': {
			initBldcControl(selected_channel, SPEED_PRE_DIVIDE_64, SPEED_DIVIDE_1, 0x8000);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 's': {
			init1_2PhaseControl(selected_channel, CLOCK_PRE_DIVIDE_1, CLOCK_DIVIDE_1, SPEED_PRE_DIVIDE_32, SPEED_DIVIDE_1, 0X02FF, 0,
					0x06ff);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'r': {
			init2_2PhaseControl(selected_channel, CLOCK_PRE_DIVIDE_1, CLOCK_DIVIDE_1, SPEED_PRE_DIVIDE_32, SPEED_DIVIDE_1, 0X04FF, 0x0080,
					0x06ff);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'l': {
			initIRCUTAction(selected_action);
		} break;
		case 'p': {
			initAbsolutePositionControl(selected_channel, MSTEP_DIVIDE_128, CLOCK_PRE_DIVIDE_128, CLOCK_DIVIDE_1, SPEED_PRE_DIVIDE_32,
					SPEED_DIVIDE_1, 0x0080, 0, 0x00ff);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'o': {
			initRelativePositionControl(selected_channel, MSTEP_DIVIDE_128, CLOCK_PRE_DIVIDE_128, CLOCK_DIVIDE_1, SPEED_PRE_DIVIDE_32,
					SPEED_DIVIDE_1, 0x0080, 0, 0x0032);
			setChannelAction(selected_channel, selected_action);
		} break;
		case 'h':
			Usage();
			break;
		}
	}

	close(fd);
	return 0;
}