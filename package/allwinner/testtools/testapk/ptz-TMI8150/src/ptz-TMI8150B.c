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
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include "std_defs.h"

extern int sensor_write_register(unsigned char addr, unsigned int data);
extern int sensor_read_register(unsigned char addr);
//278 80 703 32 352 64 20000
#define PTZ_STEP_INTERVER 20000
#define PTZ_STEP_MAX_V 670 //300 408 1062
#define PTZ_STEP_MAX_H 230 //72  158 558
#define PTZ_X_SINGLE_POINT_STEPS_DEFAULT                                       \
	(10) //默认水平方向摇杆单点对应云台的步数
#define PTZ_Y_SINGLE_POINT_STEPS_DEFAULT                                       \
	(10) //默认水平方向摇杆单点对应云台的步数
#define PTZ_X_SINGLE_POINT_STEPS (5) //默认水平方向摇杆单点对应云台的步数
#define PTZ_Y_SINGLE_POINT_STEPS (5) //默认水平方向摇杆单点对应云台的步数
#define PTZ_SINGLE_POINT_STEPS_DEFAULT                                         \
	(100) //默认水平方向摇杆单点对应云台的步数
#define PTZ_REFRESH_LOCATION_STEPS (20) //瑙﹀彂鏇存柊浜戝彴浣嶇疆鐨勬鏁?

//by aw k.l
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

typedef enum {
	PTZ_SPEED_LEVEL_LOW = 0,
	PTZ_SPEED_LEVEL_MID,
	PTZ_SPEED_LEVEL_HIGH,

} ptz_speed_level_t;

static ptz_speed_level_t g_speed_level = PTZ_SPEED_LEVEL_MID;

#define AW_ERROR(err_info)                                                     \
	do {                                                                   \
		printf err_info;                                               \
	} while (0)

#define AW_DEBUG(dbg_info)                                                     \
	do {                                                                   \
		printf dbg_info;                                               \
	} while (0)

#define INFO_DEBUG
#ifdef INFO_DEBUG
#define aw_debug(fmt, args...)                                                 \
	do {                                                                   \
		printf("[FUN]%s [LINE]%d  " fmt, __FUNCTION__, __LINE__,       \
		       ##args);                                                \
	} while (0)
#else
#define aw_debug(fmt, args...)                                                 \
	do {                                                                   \
	} while (0)
#endif

//aw k.l

#define CONFIG_USE_TMI8150B

#ifdef CONFIG_USE_TMI8150B
#define GAP_STEP_X 1 //x机器结构缝隙所浪费的步数
#define GAP_STEP_Y 1 //y机器结构缝隙所浪费的步数
#define GAP_RUN_STEP_X 1 //x机器结构缝隙+运动惯性所浪费的步数
#define GAP_RUN_STEP_y 1 //y机器结构缝隙+运动惯所浪费的步数

#else
#define GAP_STEP_X 6 //x机器结构缝隙所浪费的步数
#define GAP_STEP_Y 1 //y机器结构缝隙所浪费的步数
#define GAP_RUN_STEP_X 6 //x机器结构缝隙+运动惯性所浪费的步数
#define GAP_RUN_STEP_y 1 //y机器结构缝隙+运动惯所浪费的步数
#endif

static struct ptz_priv_s {
	int ptz_x_stepmd; //设置x细分步数
	int ptz_y_stepmd; //设置Y细分步数

	pthread_t ptz_x_update_task;
	pthread_t ptz_y_update_task;
	int ptz_x_update_task_status;
	int ptz_y_update_task_status;
	int ptz_x_is_running;
	int ptz_y_is_running;
	int ptz_x_last_step; //已上报的x圈数记录
	int ptz_y_last_step; //已上报的y圈数记录
	int ptz_x_last_dir; //存取上一次方向，用来消除结构上消耗的步数差1 顺时针 ； -1 逆时针
	int ptz_x_gop_step;
	int ptz_y_last_dir; //存取上一次方向，用来消除结构上消耗的步数差1 顺时针 ； -1 逆时针
	int ptz_y_gop_step;
	pthread_mutex_t ptz_x_lock;
	pthread_mutex_t ptz_y_lock;

	int update_time; //读取上报步数的时间
} ptz_priv;

/*更新圈数*/
static struct xy_count_s {
	int delta_x;
	int delta_y;
} xy_count;

int (*func_cb)(void) = NULL;

int aw_update_ptz_info(void)
{
	if (func_cb == NULL) {
		if (xy_count.delta_x != 0 ||
		    xy_count.delta_y != 0) //非0情况才上传
		{
			aw_debug("update ptz info x %d, y:%d \n",
				 xy_count.delta_x, xy_count.delta_y);
		}
	} else {
		aw_debug("cb function is NULL, please set it first \n");
		return -1;
	}

	//上传完清零
	xy_count.delta_x = 0;
	xy_count.delta_y = 0;

	return 0;
}
/*
* aw_ptz_update_step 函数用于计圈数上报和清零
* dir:用于计数清零：0：顺时针清零，1：逆时针清零  ；
* motor: 0 更新x轴位置，1:更新y轴位置； 2:更新x和y轴位置
*/
static void aw_ptz_update_step(int dir, int motor)
{
	int x_num = 0;
	int y_num = 0;
	unsigned char val = 0;
	do {
		if (motor == 0 || motor == 2) {
			//如果电机停了，则不计算步数。只作清0
			if (ptz_priv.ptz_x_is_running == 0) {
				//X轴清零计数
				if (dir == 0) {
					sensor_write_register(0x0b, 0x00);
					sensor_write_register(0x0c, 0x00);
					sensor_write_register(0x08, 0x00);
					sensor_write_register(0x09, 0x00);
					sensor_write_register(0x04,
							      0x07); // 关闭x轴
					ptz_priv.ptz_x_last_step = 0;
				} else {
					sensor_write_register(0x0b, 0x00);
					sensor_write_register(0x0c, 0x00);
					sensor_write_register(0x08, 0x00);
					sensor_write_register(0x09, 0x00);
					sensor_write_register(0x04,
							      0x07); // 关闭x轴
					ptz_priv.ptz_x_last_step = 8192;
				}
				break;
			}

			/*先关闭X轴，不要动*/
			val = (unsigned char)sensor_read_register(0x05);
			val |= 0x0f;
			sensor_write_register(0x05, val); //关闭x轴

			/*读出上一次的值*/
			/*X轴*/
			val = (unsigned char)sensor_read_register(0x08);
			x_num = val;
			val = (unsigned char)sensor_read_register(0x09);
			x_num += (int)(val & 0x1f) << 8;

			//这里有可能是上一次运动还没执行，就马上就下来第二命令所以圈数寄存器还是0，没更新
			if (x_num == 0) {
				if (ptz_priv.ptz_x_gop_step >
				    0) { //上一次的状态是（反|正）， 所以需要填结构缝隙 +
					ptz_priv.ptz_x_last_dir =
						-1; //复制上上次运动的状态
					ptz_priv.ptz_x_gop_step =
						0; //本次没动，所以清除掉偏移
				} else if (ptz_priv.ptz_x_gop_step <
					   0) { //上一次的状态是（正|反）， 所以需要填结构缝隙 -
					ptz_priv.ptz_x_last_dir =
						1; //复制上上次运动的状态
					ptz_priv.ptz_x_gop_step =
						0; //本次没动，所以清除掉偏移
				}

				//X轴清零计数
				if (dir == 0) {
					sensor_write_register(0x0b, 0x00);
					sensor_write_register(0x0c, 0x00);
					sensor_write_register(0x08, 0x00);
					sensor_write_register(0x09, 0x00);
					sensor_write_register(0x04,
							      0x07); // 关闭x轴
					ptz_priv.ptz_x_last_step =
						0; //因为是重新开始控制，计数和上次上报都清零。
				} else {
					sensor_write_register(0x0b, 0x00);
					sensor_write_register(0x0c, 0x00);
					sensor_write_register(0x08, 0x00);
					sensor_write_register(0x09, 0x00);
					sensor_write_register(0x04,
							      0x07); // 关闭x轴
					ptz_priv.ptz_x_last_step =
						8192; //因为是重新开始控制，计数和上次上报都清零。
				}
				break;
			}

			//与上次运动记录做个差值
			//aw_debug("x_num:%d last:%d \n", x_num, ptz_priv.ptz_x_last_step);
			x_num -=
				ptz_priv.ptz_x_last_step; //减去上次已经上报的，剩下的再上报。

			//X轴清零计数
			if (dir == 0) {
				sensor_write_register(0x0b, 0x00);
				sensor_write_register(0x0c, 0x00);
				sensor_write_register(0x08, 0x00);
				sensor_write_register(0x09, 0x00);
				sensor_write_register(0x04, 0x07); // 关闭x轴
				ptz_priv.ptz_x_last_step =
					0; //因为是重新开始控制，计数和上次上报都清零。
			} else {
				sensor_write_register(0x0b, 0x00);
				sensor_write_register(0x0c, 0x00);
				sensor_write_register(0x08, 0x00);
				sensor_write_register(0x09, 0x00);
				sensor_write_register(0x04, 0x07); // 关闭x轴
				ptz_priv.ptz_x_last_step =
					8192; //因为是重新开始控制，计数和上次上报都清零。
			}

			//ptz_priv.ptz_x_gop_step 是结构缝隙所消耗的步数，不需要上报
#if 0
			if (ptz_priv.ptz_x_gop_step)
				aw_debug("X GOP_STEP %d \n",
					 ptz_priv.ptz_x_gop_step);
#endif
			xy_count.delta_x += (x_num + ptz_priv.ptz_x_gop_step);
			ptz_priv.ptz_x_gop_step = 0;
			aw_debug("update x:%d \n", xy_count.delta_x);
		}
	} while (0);

	do {
		if (motor == 1 || motor == 2) {
			//如果电机停了，则不计算步数。只作清0
			if (ptz_priv.ptz_y_is_running == 0) {
				if (dir == 0) {
					sensor_write_register(0x1b, 0x00);
					sensor_write_register(0x1c, 0x00);
					sensor_write_register(0x18, 0x00);
					sensor_write_register(0x19, 0x00);
					sensor_write_register(0x14,
							      0x07); // 关闭y轴
					ptz_priv.ptz_y_last_step = 0;
				} else {
					sensor_write_register(0x1b, 0x00);
					sensor_write_register(0x1c, 0x00);
					sensor_write_register(0x18, 0x00);
					sensor_write_register(0x19, 0x00);
					sensor_write_register(0x14,
							      0x07); // 关闭y轴
					ptz_priv.ptz_y_last_step = 8192;
				}
				break;
			}

			/*先关闭Y轴，不要动*/
			val = (unsigned char)sensor_read_register(0x15);
			val |= 0x0f;
			sensor_write_register(0x15, val); //关闭Y轴

			/*Y 轴*/
			val = (unsigned char)sensor_read_register(0x18);
			y_num = val;
			val = (unsigned char)sensor_read_register(0x19);
			y_num += (int)(val & 0x1f) << 8;

			//这里有可能是上一次运动还没执行，就马上就下来第二命令所以圈数寄存器还是0，没更新
			if (y_num == 0) {
				if (ptz_priv.ptz_y_gop_step >
				    0) { //上一次的状态是（反|正）， 所以需要填结构缝隙 +
					ptz_priv.ptz_y_last_dir =
						-1; //复制上上次运动的状态
					ptz_priv.ptz_y_gop_step =
						0; //本次没动，所以清除掉偏移
				} else if (ptz_priv.ptz_y_gop_step <
					   0) { //上一次的状态是（正|反）， 所以需要填结构缝隙 -
					ptz_priv.ptz_y_last_dir =
						1; //复制上上次运动的状态
					ptz_priv.ptz_y_gop_step =
						0; //本次没动，所以清除掉偏移
				}

				//Y轴清零计数
				if (dir == 0) {
					sensor_write_register(0x1b, 0x00);
					sensor_write_register(0x1c, 0x00);
					sensor_write_register(0x18, 0x00);
					sensor_write_register(0x19, 0x00);
					sensor_write_register(0x14,
							      0x07); // 关闭y轴
					ptz_priv.ptz_y_last_step = 0;
				} else {
					sensor_write_register(0x1b, 0x00);
					sensor_write_register(0x1c, 0x00);
					sensor_write_register(0x18, 0x00);
					sensor_write_register(0x19, 0x00);
					sensor_write_register(0x14,
							      0x07); // 关闭y轴
					ptz_priv.ptz_y_last_step = 8192;
				}
				break;
			}

			//与上次运动记录做个差值
			aw_debug("y_num:%d last:%d \n", y_num,
				 ptz_priv.ptz_y_last_step);
			y_num -=
				ptz_priv.ptz_y_last_step; //减去上次已经上报的，剩下的再上报。

			//Y轴清零计数
			if (dir == 0) {
				sensor_write_register(0x1b, 0x00);
				sensor_write_register(0x1c, 0x00);
				sensor_write_register(0x18, 0x00);
				sensor_write_register(0x19, 0x00);
				sensor_write_register(0x14, 0x07); // 关闭y轴
				ptz_priv.ptz_y_last_step = 0;
			} else {
				sensor_write_register(0x1b, 0x00);
				sensor_write_register(0x1c, 0x00);
				sensor_write_register(0x18, 0x00);
				sensor_write_register(0x19, 0x00);
				sensor_write_register(0x14, 0x07); // 关闭y轴
				ptz_priv.ptz_y_last_step = 8192;
			}

			//ptz_priv.ptz_y_gop_step 是结构缝隙所消耗的步数，不需要上报
#if 0
			if (ptz_priv.ptz_y_gop_step)
				aw_debug("Y GOP_STEP %d \n",
					 ptz_priv.ptz_y_gop_step);
#endif
			xy_count.delta_y += (y_num + ptz_priv.ptz_y_gop_step);
			ptz_priv.ptz_y_gop_step = 0;
			aw_debug("update y:%d \n", y_num);
		}
	} while (0);
}

static char aw_get_step_md(int stepmd)
{
#ifdef CONFIG_USE_TMI8150B
	switch (stepmd) {
	case 4:
		return 0x00;
	case 8:
		return 0x10;
	case 16:
		return 0x20;
	case 32:
		return 0x30;
	case 64:
		return 0x40;
	case 128:
		return 0x50;
	case 256:
		return 0x60;
	case 512:
		return 0x70;
	default:
		return 0x60;
	}

#else //TMI8150
	switch (stepmd) {
	case 4:
		return 0x00;
	case 8:
		return 0x20;
	case 16:
		return 0x40;
	case 32:
		return 0x60;
	case 64:
		return 0x80;
	case 128:
		return 0xa0;
	case 256:
		return 0xc0;
	default:
		return 0xc0;
	}
#endif
}

//run up
static int aw_ptz_run_up(unsigned int step_count)
{
	char val;
	aw_debug("up: %d \n", step_count);
	//统计步数
	aw_ptz_update_step(0, 1);

	//检查是否和上次运动方向相反，如果相反需要补偿结构缝隙所损耗的步数
	if (ptz_priv.ptz_y_last_dir == -1) { //与上次运动相反
		if (ptz_priv.ptz_y_is_running == 0) { //查看电机是否在运动
			aw_debug("==>will tuning up \n");
			step_count += GAP_STEP_Y; //走的步数需要+
			ptz_priv.ptz_y_gop_step = -GAP_STEP_Y; //上报步数需要-
		} else { // 这里由于电机还在运动，存在惯性，反向需要消耗更多步数
			aw_debug("=xx=>will tuning up \n");
			step_count += GAP_RUN_STEP_y; //走的步数需要+
			ptz_priv.ptz_y_gop_step =
				-GAP_RUN_STEP_y; //上报步数需要-
		}
	}
	ptz_priv.ptz_y_last_dir = 1; //更新方向

	//设置step模式细分和 方向
	val = aw_get_step_md(ptz_priv.ptz_y_stepmd);
	val |= 0x05; //设置方向
	sensor_write_register(0x15, val); //设置step模式细分和 方向

	sensor_write_register(0x1b, step_count & 0xff); //set 圈数
	sensor_write_register(0x1c, (step_count / 256));

#ifdef CONFIG_USE_TMI8150B
	//分步使能
	sensor_write_register(0x14, 0x07); //set 手动控制
	sensor_write_register(0x14, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x8b); //启动电机
#else
	sensor_write_register(0x14, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x89); //启动电机
#endif

	return 0;
}
//run down
static int aw_ptz_run_down(unsigned int step_count)
{
	char val;
	aw_debug("down: %d \n", step_count);
	//统计步数和清零
	aw_ptz_update_step(1, 1);

	//检查是否和上次运动方向相反，如果相反需要补偿结构缝隙所损耗的步数
	if (ptz_priv.ptz_y_last_dir == 1) { //与上次运动相反
		if (ptz_priv.ptz_y_is_running == 0) { //查看电机是否在运动
			aw_debug("==>will tuning dowm \n");
			step_count += GAP_STEP_Y; //走的步数需要+
			ptz_priv.ptz_y_gop_step = GAP_STEP_Y; //上报步数需要-
		} else { // 这里由于电机还在运动，存在惯性，反向需要消耗更多步数
			aw_debug("=xx=>will tuning dowm \n");
			step_count += GAP_RUN_STEP_y; //走的步数需要+
			ptz_priv.ptz_y_gop_step =
				GAP_RUN_STEP_y; //上报步数需要-
		}
	}
	ptz_priv.ptz_y_last_dir = -1; //更新方向

	int value =
		8192 - step_count; // 翻转是递减的，需要从8192减， 转一圈为512

	//设置step模式细分和 方向
	val = aw_get_step_md(ptz_priv.ptz_y_stepmd);
	val |= 0x0a; //设置方向
	sensor_write_register(0x15, val); //设置step模式细分和 方向

	sensor_write_register(0x1b, value & 0xff); //set 圈数
	sensor_write_register(0x1c, (value / 256));

#ifdef CONFIG_USE_TMI8150B
	//分步使能
	sensor_write_register(0x14, 0x07); //set 手动控制
	sensor_write_register(0x14, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x8b); //启动电机
#else
	sensor_write_register(0x14, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x89); //启动电机
#endif

	return 0;
}
//run left
static int aw_ptz_run_left(unsigned int step_count)
{
	char val;
	aw_debug("left: %d \n", step_count);
	//统计步数和清零
	aw_ptz_update_step(0, 0);

	//检查是否和上次运动方向相反，如果相反需要补偿结构缝隙所损耗的步数
	if (ptz_priv.ptz_x_last_dir == -1) { //与上次运动相反
		if (ptz_priv.ptz_x_is_running == 0) { //查看电机是否在运动
			aw_debug("==> will turn left \n");
			step_count += GAP_STEP_X; //走的步数需要+
			ptz_priv.ptz_x_gop_step = -GAP_STEP_X; //上报步数需要-
		} else { // 这里由于电机还在运动，存在惯性，反向需要消耗更多步数
			aw_debug("=xx=>will tuning left \n");
			step_count += GAP_RUN_STEP_X; //走的步数需要+
			ptz_priv.ptz_x_gop_step =
				-GAP_RUN_STEP_X; //上报步数需要-
		}
	}
	ptz_priv.ptz_x_last_dir = 1; //更新方向

	//设置step模式细分和 方向
	val = aw_get_step_md(ptz_priv.ptz_x_stepmd);
	val |= 0x05; //设置方向
	sensor_write_register(0x05, val);

	sensor_write_register(0x0b, step_count & 0xff); //set 圈数
	sensor_write_register(0x0c, (step_count / 256));

#ifdef CONFIG_USE_TMI8150B
	//分步使能
	sensor_write_register(0x04, 0x07); //set 手动控制
	sensor_write_register(0x04, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x8b); //启动电机
#else
	sensor_write_register(0x04, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x89); //启动电机
#endif

	return 0;
}
//run right
static int aw_ptz_run_right(unsigned int step_count)
{
	char val;
	aw_debug("right: %d \n", step_count);
	//统计步数和清零
	aw_ptz_update_step(1, 0);

	//检查是否和上次运动方向相反，如果相反需要补偿结构缝隙所损耗的步数
	if (ptz_priv.ptz_x_last_dir == 1) { //与上次运动相反
		if (ptz_priv.ptz_x_is_running == 0) {
			aw_debug("==> will tuning right \n");
			step_count += GAP_STEP_X; //走的步数需要+
			ptz_priv.ptz_x_gop_step =
				GAP_STEP_X; //上报步数需要-, 上报是负数
		} else { // 这里由于电机还在运动，存在惯性，反向需要消耗更多步数
			aw_debug("=xx=>will tuning right \n");
			step_count += GAP_RUN_STEP_X; //走的步数需要+
			ptz_priv.ptz_x_gop_step =
				GAP_RUN_STEP_X; //上报步数需要-, 上报是负数
		}
	}
	ptz_priv.ptz_x_last_dir = -1; //更新方向

	int value =
		8192 - step_count; // 翻转是递减的，需要从8191减， 转一圈为512

	//设置step模式细分和 方向
	val = aw_get_step_md(ptz_priv.ptz_x_stepmd);
	val |= 0x0a; //设置方向
	sensor_write_register(0x05, val); //设置step模式细分和 方向

	sensor_write_register(0x0b, value & 0xff); //set 圈数
	sensor_write_register(0x0c, (value / 256));
	aw_debug("0b:0x%x 0c:0x%x \n", sensor_read_register(0x0b),
		 sensor_read_register(0x0c));
#ifdef CONFIG_USE_TMI8150B
	//分步使能
	sensor_write_register(0x04, 0x07); //set 手动控制
	sensor_write_register(0x04, 0x87);
	sensor_write_register(0x02, 0x8b); //启动电机
#else
	sensor_write_register(0x04, 0x87); //set 手动控制
	sensor_write_register(0x02, 0x89); //启动电机
#endif

	return 0;
}

//run stop
static int aw_ptz_run_stop()
{
	int x_num = 0;
	int y_num = 0;
	unsigned char val = 0;

	val = (unsigned char)sensor_read_register(0x05);
	val |= 0x0f;
	sensor_write_register(0x05, val); //关闭x轴

	val = (unsigned char)sensor_read_register(0x15);
	val |= 0x0f;
	sensor_write_register(0x15, val); //关闭Y轴

	//触发xy位置更新；
	aw_ptz_update_step(0, 2);

	return 0;
}

static int _aw_set_ptz_speed(int speed_level)
{
#if 0
	if (0 == g_speed_level) {
		sensor_write_register(0x06, 0XE3); //设置x轴速度
		sensor_write_register(0x16, 0XE3); //设置Y速度
	} else if (1 == g_speed_level) {
		sensor_write_register(0x06, 0XC5); //设置x轴速度
		sensor_write_register(0x16, 0XC5); //设置Y速度
	} else if (2 == g_speed_level) {
		sensor_write_register(0x06, 0XC3); //设置x轴速度
		sensor_write_register(0x16, 0XC3); //设置Y速度
	} else if (speed_level == 3) {
		sensor_write_register(0x06, 0XA5); //设置x轴速度
		sensor_write_register(0x16, 0XA5); //设置Y速度
	} else if (speed_level == 4) {
		sensor_write_register(0x06, 0XA4); //设置x轴速度
		sensor_write_register(0x16, 0XA4); //设置Y速度
	} else if (speed_level == 5) {
		sensor_write_register(0x06, 0XA3); //设置x轴速度
		sensor_write_register(0x16, 0XA3); //设置Y速度
	} else if (speed_level == 6) {
		sensor_write_register(0x06, 0X85); //设置x轴速度
		sensor_write_register(0x16, 0X85); //设置Y速度
	} else if (speed_level == 7) {
		sensor_write_register(0x06, 0X84); //设置x轴速度
		sensor_write_register(0x16, 0X84); //设置Y速度
	} else if (speed_level == 8) {
		sensor_write_register(0x06, 0X83); //设置x轴速度
		sensor_write_register(0x16, 0X83); //设置Y速度
	} else if (speed_level == 9) {
		sensor_write_register(0x06, 0X65); //设置x轴速度
		sensor_write_register(0x16, 0X65); //设置Y速度
	} else if (speed_level == 10) {
		sensor_write_register(0x06, 0X64); //设置x轴速度
		sensor_write_register(0x16, 0X64); //设置Y速度
	} else if (speed_level == 11) {
		sensor_write_register(0x06, 0X63); //设置x轴速度
		sensor_write_register(0x16, 0X63); //设置Y速度
	} else if (speed_level == 12) {
		sensor_write_register(0x06, 0X45); //设置x轴速度
		sensor_write_register(0x16, 0X45); //设置Y速度
	} else if (speed_level == 13) {
		sensor_write_register(0x06, 0X44); //设置x轴速度
		sensor_write_register(0x16, 0X44); //设置Y速度
	} else if (speed_level == 14) {
		sensor_write_register(0x06, 0X43); //设置x轴速度
		sensor_write_register(0x16, 0X43); //设置Y速度
	} else if (speed_level == 15) {
		sensor_write_register(0x06, 0X25); //设置x轴速度
		sensor_write_register(0x16, 0X25); //设置Y速度
	} else if (speed_level == 16) {
		sensor_write_register(0x06, 0X24); //设置x轴速度
		sensor_write_register(0x16, 0X24); //设置Y速度
	} else if (speed_level == 17) {
		sensor_write_register(0x06, 0X23); //设置x轴速度
		sensor_write_register(0x16, 0X23); //设置Y速度
	} else if (speed_level == 18) {
		sensor_write_register(0x06, 0X0a); //设置x轴速度
		sensor_write_register(0x16, 0X0a); //设置Y速度
	} else if (speed_level == 19) {
		sensor_write_register(0x06, 0X09); //设置x轴速度
		sensor_write_register(0x16, 0X09); //设置Y速度
	} else if (speed_level == 20) {
		sensor_write_register(0x06, 0X08); //设置x轴速度
		sensor_write_register(0x16, 0X08); //设置Y速度
	} else if (speed_level == 21) {
		sensor_write_register(0x06, 0X07); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	} else if (speed_level == 22) {
		sensor_write_register(0x06, 0X06); //设置x轴速度
		sensor_write_register(0x16, 0X06); //设置Y速度
	} else if (speed_level == 23) {
		sensor_write_register(0x06, 0X05); //设置x轴速度
		sensor_write_register(0x16, 0X05); //设置Y速度
	} else if (speed_level == 24) {
		sensor_write_register(0x06, 0X04); //设置x轴速度
		sensor_write_register(0x16, 0X04); //设置Y速度
	} else if (speed_level == 25) {
		sensor_write_register(0x06, 0X03); //设置x轴速度
		sensor_write_register(0x16, 0X03); //设置Y速度
	} else if (speed_level == 26) {
		sensor_write_register(0x06, 0X02); //设置x轴速度
		sensor_write_register(0x16, 0X02); //设置Y速度
	} else if (speed_level == 27) {
		sensor_write_register(0x06, 0x0a); //设置x轴速度
		sensor_write_register(0x16, 0x0a); //设置Y速度
	} else if (speed_level == 28) {
		sensor_write_register(0x06, 0X01); //设置x轴速度
		sensor_write_register(0x16, 0X01); //设置Y速度
	} else if (speed_level == 29) {
		sensor_write_register(0x06, 0x00); //设置x轴速度
		sensor_write_register(0x16, 0x00); //设置Y速度
	}
#else
	char reg;

#ifdef CONFIG_USE_TMI8150B
	if (PTZ_SPEED_LEVEL_LOW == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 512;
		ptz_priv.ptz_y_stepmd = 512;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X07); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	}

	else if (PTZ_SPEED_LEVEL_MID == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 512;
		ptz_priv.ptz_y_stepmd = 512;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X04); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	} else if (PTZ_SPEED_LEVEL_HIGH == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 512;
		ptz_priv.ptz_y_stepmd = 512;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X03); //设置x轴速度
		sensor_write_register(0x16, 0X06); //设置Y速度

	} else {
		ptz_priv.ptz_x_stepmd = 512;
		ptz_priv.ptz_y_stepmd = 512;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X03); //设置x轴速度
		sensor_write_register(0x16, 0X06); //设置Y速度
	}
#else
	if (PTZ_SPEED_LEVEL_LOW == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 256;
		ptz_priv.ptz_y_stepmd = 256;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X07); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	} else if (PTZ_SPEED_LEVEL_MID == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 256;
		ptz_priv.ptz_y_stepmd = 256;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X07); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	} else if (PTZ_SPEED_LEVEL_HIGH == g_speed_level) {
		ptz_priv.ptz_x_stepmd = 256;
		ptz_priv.ptz_y_stepmd = 256;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X05); //设置x轴速度
		sensor_write_register(0x16, 0X05); //设置Y速度

	} else {
		ptz_priv.ptz_x_stepmd = 256;
		ptz_priv.ptz_y_stepmd = 256;
		reg = aw_get_step_md(ptz_priv.ptz_y_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x15, reg); //设置step模式细分和 方向

		//设置step模式细分和 方向
		reg = aw_get_step_md(ptz_priv.ptz_x_stepmd);
		reg |= 0x05; //设置方向
		sensor_write_register(0x05, reg);

		sensor_write_register(0x06, 0X07); //设置x轴速度
		sensor_write_register(0x16, 0X07); //设置Y速度
	}
#endif
	return 0;
#endif

	return 0;
}

int aw_set_ptz_speed(ptz_speed_level_t speed_level)
{
	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_x_lock);
	pthread_mutex_lock(&ptz_priv.ptz_y_lock);

	//设置速度前先停止转动
	aw_ptz_run_stop();

	//让线程知道电机已经stop了
	ptz_priv.ptz_x_is_running = 0;
	ptz_priv.ptz_y_is_running = 0;

	g_speed_level = speed_level;
	_aw_set_ptz_speed(speed_level);

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
	pthread_mutex_unlock(&ptz_priv.ptz_y_lock);

	return 0;
}

//speed
int aw_get_ptz_speed(ptz_speed_level_t *speed_level)
{
	*speed_level = g_speed_level;
	return 0;
}

//用于拆分步数，并按时上报电机位移情况
static void *ptz_x_update_step_task(void *para)
{
	int x_need = 0;
	char dir;
	int num_all = 0;
	int num_count = 0;
	unsigned int val;
	aw_debug("run x step task \n");
	while (ptz_priv.ptz_x_update_task_status) {
		usleep(ptz_priv.update_time * 1000); //多久轮询一次

		//加上控制锁,确保控制同步
		pthread_mutex_lock(&ptz_priv.ptz_x_lock);

		//检查是否电机在运动
		if (!ptz_priv.ptz_x_is_running) {
			goto x_task_unlock;
		}

		/*读取x电机走了多少步, 还需要走多少步*/
		//获取电机要走多步
		val = sensor_read_register(0x0b);
		num_all = val;
		val = sensor_read_register(0x0c);
		num_all += (int)(val & 0x1f) << 8;
		aw_debug("x_num_all:%d \n", num_all);

		/*读出上一次的值*/
		val = (unsigned char)sensor_read_register(0x08);
		num_count = val;
		val = (unsigned char)sensor_read_register(0x09);
		num_count += (int)(val & 0x1f) << 8;
		aw_debug("x_num_count:%d \n", num_count);

		//算出还需走多少步
		x_need = num_all - num_count;

		//对可能存在的异常做处理。如果剩余很多负数的(或者圈数寄存器是0)，则可能是芯片寄存器
		//没更新到，先暂时不上报步数，等待下次恢复正常上报
		if (x_need < (-3000) || x_need > 3000 || num_count == 0) {
			printf("x count erro, please wait next update \n");
			goto x_task_unlock;
		}
		aw_debug("need count : %d \n", x_need);

		//上报x轴运行的差值
		xy_count.delta_x += num_count - ptz_priv.ptz_x_last_step;
		aw_debug("pthread x update: %d \n",
			 (num_count - ptz_priv.ptz_x_last_step));
		ptz_priv.ptz_x_last_step =
			num_count; //记录本次，为下一步差值做计算。

		//ptz_priv.ptz_x_gop_step 是结构缝隙所消耗的步数，不需要上报
#if 0
		if (ptz_priv.ptz_x_gop_step)
			aw_debug("X GOP_STEP %d \n", ptz_priv.ptz_x_gop_step);
#endif
		xy_count.delta_x += ptz_priv.ptz_x_gop_step;
		ptz_priv.ptz_x_gop_step = 0;

		//TODO:上报位置：

		//电机已经运行完
		if (x_need == 0) {
			/*先关闭X轴，不要动*/
			//val = (unsigned char)sensor_read_register(0x05);
			//val |= 0x0f;
			//sensor_write_register(0x05, val); //关闭x轴
			ptz_priv.ptz_x_is_running = 0;
			aw_debug("x: %d \n", xy_count.delta_x);
		}

	x_task_unlock:
		pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
	}
}

//用于拆分步数，并按时上报电机位移情况
static void *ptz_y_update_step_task(void *para)
{
	int y_need = 0;
	char dir;
	int num_all = 0;
	int num_count = 0;
	unsigned int val;
	aw_debug("run y step task \n");
	while (ptz_priv.ptz_y_update_task_status) {
		usleep(ptz_priv.update_time * 1000); //多久轮询一次

		//加上控制锁,确保控制同步
		pthread_mutex_lock(&ptz_priv.ptz_y_lock);

		//检查是否电机在运动
		if (!ptz_priv.ptz_y_is_running) {
			goto y_task_unlock;
		}

		/*读取y电机走了多少步, 还需要走多少步*/
		//获取电机要走多步
		val = sensor_read_register(0x1b);
		num_all = val;
		val = sensor_read_register(0x1c);
		num_all += (int)(val & 0x1f) << 8;

		/*读出上一次的值*/
		val = (unsigned char)sensor_read_register(0x18);
		num_count = val;
		val = (unsigned char)sensor_read_register(0x19);
		num_count += (int)(val & 0x1f) << 8;
		//算出还需走多少步
		y_need = num_all - num_count;

		//对可能存在的异常做处理。如果剩余很多负数的(或者圈数寄存器是0)，则可能是芯片寄存器
		//没更新到，先暂时不上报步数，等待下次恢复正常上报
		if (y_need < -3000 || y_need > 3000 || num_count == 0) {
			printf("y count erro, please wait next update \n");
			goto y_task_unlock;
		}

		//上报x轴运行的差值
		xy_count.delta_y += num_count - ptz_priv.ptz_y_last_step;
		aw_debug("pthread y update: %d \n",
			 (num_count - ptz_priv.ptz_y_last_step));
		ptz_priv.ptz_y_last_step =
			num_count; //记录本次，为下一步差值做计算。

		//ptz_priv.ptz_y_gop_step 是结构缝隙所消耗的步数，不需要上报
#if 0
		if (ptz_priv.ptz_y_gop_step)
			aw_debug("Y GOP_STEP %d \n", ptz_priv.ptz_y_gop_step);
#endif
		xy_count.delta_y += ptz_priv.ptz_y_gop_step;
		ptz_priv.ptz_y_gop_step = 0;

		//TODO:上报位置：

		//电机已经运行完
		if (y_need == 0) {
			/*先关闭Y轴，不要动*/
			//val = (unsigned char)sensor_read_register(0x15);
			//val |= 0x0f;
			//sensor_write_register(0x15, val);  //关闭Y轴
			ptz_priv.ptz_y_is_running = 0;
		}

	y_task_unlock:
		pthread_mutex_unlock(&ptz_priv.ptz_y_lock);
	}
}

//H step time
unsigned int aw_ptz_hstep_time()
{
	char reg;
	char cdvi_reg;
	char csel_reg;
	unsigned int cdvi, csel;
	unsigned int stepmd =
		ptz_priv.ptz_x_stepmd; //细分步数，默认设为256，也可以从寄存器读出.
	reg = sensor_read_register(0x06); //获取x轴时钟分频
	aw_debug("The 0x06: %x \n", reg);
	cdvi_reg = reg >> 5;
	csel_reg = reg & 0x1f;

	cdvi = 1 << cdvi_reg;
	csel = csel_reg + 1;
	aw_debug("cdvi_reg: %d, csel_reg:%d, cdvi %d, csel%d \n", cdvi_reg,
		 csel_reg, cdvi, csel);

	//计算公式
	//每一步计算公式：
	//时钟为28MHz, 经过分频和细分后得：x = 28MHz/cdvi/csel/stepmd ;
	//每一步时间为： y = (1/x) * stepmd; (注意这里时间单位换算)
	aw_debug("the time %ld us\n", 256 * stepmd * cdvi * csel / 29);
	return (256 * stepmd * cdvi * csel / 29 + 1000); //+5000 是加上固定偏差
}

//V step time
unsigned int aw_ptz_vstep_time()
{
	char reg;
	char cdvi_reg;
	char csel_reg;
	unsigned int cdvi, csel;
	unsigned int stepmd =
		ptz_priv.ptz_y_stepmd; //细分步数，默认设为256，也可以从寄存器读出.
	reg = sensor_read_register(0x016); //获取y轴时钟分频
	aw_debug("The 0x06: %x \n", reg);
	cdvi_reg = reg >> 5;
	csel_reg = reg & 0x1f;

	cdvi = 1 << cdvi_reg;
	csel = csel_reg + 1;
	aw_debug("cdvi_reg: %d, csel_reg:%d, cdvi %d, csel%d \n", cdvi_reg,
		 csel_reg, cdvi, csel);

	//计算公式
	//算出1秒跑多少步：
	//时钟为28MHz, 经过分频和细分后得：x = 28MHz/cdvi/csel/stepmd ;
	//每一步时间为： y = (1/x) * stepmd; (注意这里时间单位换算)
	aw_debug("the time %ld us\n", 256 * stepmd * cdvi * csel / 29);
	return (256 * stepmd * cdvi * csel / 29 + 1000); //+5000 是加上固定偏差
}

int aw_ptz_stop(U32 *step, U32 *direction)
{
	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_x_lock);
	pthread_mutex_lock(&ptz_priv.ptz_y_lock);

	aw_ptz_run_stop();

	//让线程知道电机已经stop了
	ptz_priv.ptz_x_is_running = 0;
	ptz_priv.ptz_y_is_running = 0;

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
	pthread_mutex_unlock(&ptz_priv.ptz_y_lock);
}

int aw_ptz_up(int max_y, int breset, int step)
{
	if (!step)
		return 0;
	//如果x轴还在动或y轴需要反向运动，先调用暂停接口；
	if ((ptz_priv.ptz_x_is_running == 1) || (ptz_priv.ptz_y_last_dir == -1))
		aw_ptz_stop(NULL, NULL);

	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_y_lock);

	aw_ptz_run_up(step);

	//让线程知道电机已经开始运动了
	ptz_priv.ptz_y_is_running = 1;

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_y_lock);
	return 0;
}

int aw_ptz_down(int max_y, int breset, int step)
{
	if (!step)
		return 0;

	//如果x轴还在动或y轴需要反向运动，先调用暂停接口；
	if ((ptz_priv.ptz_x_is_running == 1) || (ptz_priv.ptz_y_last_dir == 1))
		aw_ptz_stop(NULL, NULL);

	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_y_lock);

	aw_ptz_run_down(step);

	//让线程知道电机已经开始运动了
	ptz_priv.ptz_y_is_running = 1;

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_y_lock);
}

int aw_ptz_left(int max_x, int breset, int step)
{
	if (!step)
		return 0;

	//如果y轴还在动或x轴需要反向运动，先调用暂停接口；
	if ((ptz_priv.ptz_y_is_running == 1) || (ptz_priv.ptz_x_last_dir == -1))
		aw_ptz_stop(NULL, NULL);

	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_x_lock);

	aw_ptz_run_left(step);

	//让线程知道电机已经开始运动了
	ptz_priv.ptz_x_is_running = 1;

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
}

int aw_ptz_right(int max_x, int breset, int step)
{
	if (!step)
		return 0;

	//如果y轴还在动或x轴需要反向运动，先调用暂停接口；
	if ((ptz_priv.ptz_y_is_running == 1) || (ptz_priv.ptz_x_last_dir == 1))
		aw_ptz_stop(NULL, NULL);

	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_x_lock);

	aw_ptz_run_right(step);

	//让线程知道电机已经开始运动了
	ptz_priv.ptz_x_is_running = 1;

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
}

#define ONESTEP_COUNT 170
//one step down
int aw_ptz_onestep_down()
{
	aw_ptz_down(0, 0, ONESTEP_COUNT);
	return ONESTEP_COUNT;
}
//one step up
int aw_ptz_onestep_up()
{
	aw_ptz_up(0, 0, ONESTEP_COUNT);
	return ONESTEP_COUNT;
}
//one step right
int aw_ptz_onestep_right()
{
	aw_ptz_right(0, 0, ONESTEP_COUNT);
	return ONESTEP_COUNT;
}
//one step left
int aw_ptz_onestep_left()
{
	aw_ptz_left(0, 0, ONESTEP_COUNT);
	return ONESTEP_COUNT;
}

status_code_t aw_mortor_init(void *cb_func)
{
	memset(&ptz_priv, 0x00, sizeof(ptz_priv));

	int err = pthread_mutex_init(&ptz_priv.ptz_x_lock, NULL);
	if (err != 0) {
		AW_ERROR(("motor init erro: x pthread mutex init fail! \n"));
		return -1;
	}

	err = pthread_mutex_init(&ptz_priv.ptz_y_lock, NULL);
	if (err != 0) {
		AW_ERROR(("motor init erro: y pthread mutex init fail! \n"));
		return -1;
	}

	err = pthread_create(&ptz_priv.ptz_x_update_task, NULL,
			     ptz_x_update_step_task, NULL);
	if (err != 0) {
		AW_ERROR(("motor init erro: pthread creat fail! \n"));
		return -1;
	}

	err = pthread_create(&ptz_priv.ptz_y_update_task, NULL,
			     ptz_y_update_step_task, NULL);
	if (err != 0) {
		AW_ERROR(("motor init erro: pthread creat fail! \n"));
		return -1;
	}

#ifdef CONFIG_USE_TMI8150B
	ptz_priv.ptz_x_stepmd = 512;
	ptz_priv.ptz_y_stepmd = 512;
#else
	ptz_priv.ptz_x_stepmd = 256;
	ptz_priv.ptz_y_stepmd = 256;
#endif
	ptz_priv.ptz_x_update_task_status = 1;
	ptz_priv.ptz_y_update_task_status = 1;

	ptz_priv.update_time = 150; // 250ms

	//设置芯片主频
	sensor_write_register(0x03, 0X1d);

#ifdef CONFIG_USE_TMI8150B
	//解锁圈数相位操作
	int val;
	val = sensor_read_register(0x02);
	val |= 0x02;
	sensor_write_register(0x02, val);
#endif

	//设置初始速度
	aw_set_ptz_speed(PTZ_SPEED_LEVEL_MID);

	return 0;
}

//for test
int ptz_show_step(void)
{
	printf("x:%d y:%d \n", xy_count.delta_x, xy_count.delta_y);
	return 0;
}

status_code_t sk_task_delay(const U32 ms)
{
#if 0
	usleep(u32Ms * 1000);
#else
	struct timeval tv;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
#endif
	return AW_SUCCESS;
}

int motor_test(void)
{
	int ret;
	unsigned int time = 0;
	int times = 80;
#if 0
	while (times) {
		times--;

#if 1
		//26
		aw_debug("right: 26 \n");
		aw_ptz_right(403, 1, 26);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 26);
		//usleep(3240);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//63
		aw_debug("left: 63 \n");
		aw_ptz_left(403, 1, 63);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 64);
		//	usleep(2840);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//59
		aw_debug("right: 59 \n");
		aw_ptz_right(403, 1, 59);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 60);
		//	usleep(2040);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//34
		aw_debug("left: 34 \n");
		aw_ptz_left(403, 1, 34);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 35);
		//	usleep(1640)
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//33
		aw_debug("left: 33 \n");
		aw_ptz_left(403, 1, 33);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 34);
		//	usleep(1240);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//26
		aw_debug("right: 26 \n");
		aw_ptz_right(403, 1, 26);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 27);

		//	usleep(1640);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//46
		aw_debug("right: 46 \n");
		aw_ptz_right(403, 1, 46);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 47);
		//	usleep(1240);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		//27
		aw_debug("left: 27 \n");
		aw_ptz_left(403, 1, 27);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * 28);
		//	usleep(1640);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		aw_debug("up: 20 \n");
		aw_ptz_up(403, 1, 20);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * 21);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		aw_debug("down: 20 \n");
		aw_ptz_down(403, 1, 20);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * 21);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		aw_debug("up: 28 \n");
		aw_ptz_up(403, 1, 28);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * 29);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		aw_debug("down: 28 \n");
		aw_ptz_down(403, 1, 28);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * 29);
		aw_ptz_stop(&ret, &ret);
		ptz_show_step();
	}
#else
		int step;

		step = 15;
		aw_debug("left: %d \n", step);
		aw_ptz_left(403, 1, step);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * (step));
		//aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		step = 15;
		aw_debug("right: %d \n", step);
		aw_ptz_right(403, 1, step);
		sk_task_delay((aw_ptz_hstep_time() / 1000) * (step));
		//aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		step = 15;
		aw_debug("up: %d \n", step);
		aw_ptz_up(403, 1, step);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * (step));
		//aw_ptz_stop(&ret, &ret);
		ptz_show_step();

		step = 15;
		aw_debug("down: %d \n", step);
		aw_ptz_down(403, 1, step);
		sk_task_delay((aw_ptz_vstep_time() / 1000) * (step));
		//aw_ptz_stop(&ret, &ret);
		ptz_show_step();
	}

	//wait ptz motor stop
	while (ptz_priv.ptz_x_is_running || ptz_priv.ptz_y_is_running)
		;

	aw_debug("run finsh, now check x:%d, y:%d \n", xy_count.delta_x,
		 xy_count.delta_y);
	if (xy_count.delta_x > 0) {
		aw_debug("righ: %d \n", xy_count.delta_x);
		aw_ptz_right(403, 1, xy_count.delta_x);

	} else if (xy_count.delta_x < 0) {
		aw_debug("lift: %d \n", xy_count.delta_x);
		aw_ptz_right(403, 1, (xy_count.delta_x * -1));
	}

	if (xy_count.delta_y > 0) {
		aw_debug("down: %d \n", xy_count.delta_y);
		aw_ptz_down(403, 1, xy_count.delta_y);

	} else if (xy_count.delta_y < 0) {
		aw_debug("up: %d \n", xy_count.delta_y);
		aw_ptz_up(403, 1, (xy_count.delta_y * -1));
	}

	while (ptz_priv.ptz_x_is_running || ptz_priv.ptz_y_is_running)
		;

#endif

#else

#define X_MAX (150)
#define Y_MAX (50)
	int step = 0;
	times = 60;
	while (times) {
		times--;

		switch (times % 4) {
		case 0:
			step = rand() % (X_MAX);
			step += GAP_STEP_X;
			ret = xy_count.delta_x + step;
			printf("ret: %d \n", ret);
			if (ret < X_MAX && ret > -X_MAX) {
				printf("=====>left : %d \n", step);
				aw_ptz_left(403, 1, step);
				sk_task_delay((aw_ptz_hstep_time() / 1000) *
					      (step));
			}
			break;
		case 1:
			step = rand() % (X_MAX);
			step += GAP_STEP_X;
			ret = xy_count.delta_x - step;
			printf("ret: %d \n", ret);
			if (ret < X_MAX && ret > -X_MAX) {
				printf("=====>right : %d \n", step);
				aw_ptz_right(403, 1, step);
				sk_task_delay((aw_ptz_hstep_time() / 1000) *
					      (step));
			}
			break;
		case 2:
			step = rand() % (Y_MAX);
			step += GAP_STEP_Y;
			ret = xy_count.delta_y + step;
			printf("ret: %d \n", ret);
			if (ret < Y_MAX && ret > -Y_MAX) {
				printf("=====>up : %d \n", step);
				aw_ptz_up(403, 1, step);
				sk_task_delay((aw_ptz_hstep_time() / 1000) *
					      (step));
			}
			break;
		case 3:
			step = rand() % (Y_MAX);
			step += GAP_STEP_Y;
			printf("ret: %d \n", ret);
			ret = xy_count.delta_y - step;
			if (ret < Y_MAX && ret > -Y_MAX) {
				printf("======>dwon : %d \n", step);
				aw_ptz_down(403, 1, step);
				sk_task_delay((aw_ptz_hstep_time() / 1000) *
					      (step));
			}
			break;
		}
	}

	printf("===> revocer \n");
	//wait ptz motor stop
	while (ptz_priv.ptz_x_is_running || ptz_priv.ptz_y_is_running)
		;

	aw_debug("run finsh, now check x:%d, y:%d \n", xy_count.delta_x,
		 xy_count.delta_y);
	if (xy_count.delta_x > 0) {
		aw_debug("righ: %d \n", xy_count.delta_x);
		aw_ptz_right(403, 1, xy_count.delta_x);

	} else if (xy_count.delta_x < 0) {
		aw_debug("left: %d \n", xy_count.delta_x);
		aw_ptz_left(403, 1, (xy_count.delta_x * -1));
	}

	while (ptz_priv.ptz_x_is_running || ptz_priv.ptz_y_is_running)
		;

	if (xy_count.delta_y > 0) {
		aw_debug("down: %d \n", xy_count.delta_y);
		aw_ptz_down(403, 1, xy_count.delta_y);

	} else if (xy_count.delta_y < 0) {
		aw_debug("up: %d \n", xy_count.delta_y);
		aw_ptz_up(403, 1, (xy_count.delta_y * -1));
	}

	while (ptz_priv.ptz_x_is_running || ptz_priv.ptz_y_is_running)
		;

#endif
	printf("end \n");
	ptz_show_step();
}

int aw_set_ir(int enable)
{
	//加上控制锁,确保控制同步
	pthread_mutex_lock(&ptz_priv.ptz_x_lock);
	pthread_mutex_lock(&ptz_priv.ptz_y_lock);

	//设置速度前先停止转动
	aw_ptz_run_stop();

	//让线程知道电机已经stop了
	ptz_priv.ptz_x_is_running = 0;
	ptz_priv.ptz_y_is_running = 0;

	if (enable == 0) {
		printf("disable ir \n");
#ifdef CONFIG_USE_TMI8150B
		sensor_write_register(0x11, 0x84); //正转
#else
		sensor_write_register(0x10, 0x84); //正转
#endif

	} else if (enable = 1) {
		printf("enable ir \n");
#ifdef CONFIG_USE_TMI8150B
		sensor_write_register(0x11, 0x88); //反转
#else
		sensor_write_register(0x10, 0x88); //反转
#endif
	}

#ifdef CONFIG_USE_TMI8150B
	sensor_write_register(0x02, 0x8b); //启动
	usleep(200000); //200ms
	sensor_write_register(0x11, 0x08); //启动
#else
	sensor_write_register(0x02, 0x89); //启动
	usleep(200000); //200ms
	sensor_write_register(0x10, 0x08); //启动
#endif

	//解锁
	pthread_mutex_unlock(&ptz_priv.ptz_x_lock);
	pthread_mutex_unlock(&ptz_priv.ptz_y_lock);

	return 0;
}

//for test
int main(int argc, char *argv[])
{
	char str[256] = { 0 };
	char val[16] = { 0 };
	int num = 0;
	unsigned char speed = 0;
	int i = 0;
	unsigned int time = 0;
	char reg;

	printf("version: cc\n");

	aw_spi_init();

	aw_mortor_init(NULL);
#if 1
	num = atoi(argv[1]);
	printf("input %d \n", num);
	if (num == 1)
		aw_set_ir(0);
	else
		aw_set_ir(1);
	return 0;
#endif
#if 0
	int motor = 0;
	while (1) {
		memset(str, 0, sizeof(str));
		memset(val, 0, sizeof(val));
		fgets(str, 256, stdin);
		if (!strncmp(str, "x", strlen("x"))) {
			printf("enable x motor \n");
			motor = 0;
		} else if (!strncmp(str, "y", strlen("y"))) {
			printf("enable y motor \n");
			motor = 1;
		} else {
			num = atoi(str);

			if (num < 0) {
				num = num * -1;
				aw_debug("will run :%d \n", num);
				if (motor == 0)
					aw_ptz_right(0, 0, num);
				else
					aw_ptz_up(0, 0, num);
			} else {
				aw_debug("will run :%d \n", num);
				if (motor == 0)
					aw_ptz_left(0, 0, num);
				else
					aw_ptz_down(0, 0, num);
			}
			sk_task_delay((aw_ptz_vstep_time() / 1000) * (num + 1));
		}
	}
#endif
#if 1
	aw_debug("1:left, 2:right, 3:up, 4:down 5:stop 9:exit\n");
	while (1) {
		memset(str, 0, sizeof(str));
		memset(val, 0, sizeof(val));
		fgets(str, 256, stdin);
		num = atoi(str);

		switch (num) {
		case 0:
			motor_test();
			break;
		case 1:
			//ptz_show_step();
			aw_ptz_onestep_left();
			break;
		case 2:
			aw_ptz_onestep_right();
			break;
		case 3:
			aw_ptz_onestep_up();
			aw_ptz_hstep_time();
			aw_ptz_vstep_time();
			break;
		case 4:
			aw_ptz_onestep_down();
			aw_ptz_hstep_time();
			aw_ptz_vstep_time();
			break;
		case 5:
			aw_ptz_hstep_time();
			aw_ptz_vstep_time();
			break;
		case 6:
//speed = rand();
#if 0
			ptz_priv.ptz_x_stepmd = 4 * (1 << i);
			ptz_priv.ptz_y_stepmd = 4 * (1 << i);
			aw_ptz_right(403, 1, 1);
			aw_ptz_up(403, 1, 1);
			aw_debug("ptz_x_stepmd:%d \n", ptz_priv.ptz_x_stepmd);
			i++;
			if (i > 6)
				i = 0;
			for (speed = 0; speed < 28; speed++) {
				aw_debug("set speed :%d \n", speed);
				aw_set_ptz_speed(speed);
				usleep(3000);
				time = aw_ptz_hstep_time();
				if (((time - 8000) < 30000) &&
				    ((time - 8000) > 20000))
					break;
			}
#else
			if (speed > 28)
				speed = 0;
			aw_debug("set speed :%d \n", speed);
			aw_set_ptz_speed(speed);
			speed += 1;
#endif
#if 0
			speed = rand();
			speed %= 3;
			aw_debug("set speed :%d \n", speed);
			aw_set_ptz_speed(speed);
#endif

			break;
		case 7:
			aw_set_ir(0);
			break;
		case 8:
			aw_set_ir(1);
			break;
		case 9:
			return 0;
		default:
			//aw_ptz_hstep_time();
			//aw_ptz_vstep_time();
			if (num < 0) {
				num = num * -1;
				aw_debug("will run :%d \n", num);
				aw_ptz_up(0, 0, num);
			} else {
				aw_debug("will run :%d \n", num);
				aw_ptz_down(0, 0, num);
			}
			break;
		}
	}
#endif
	return 0;
}
