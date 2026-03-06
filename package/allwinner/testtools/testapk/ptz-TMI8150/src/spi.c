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
/*
 * Author: wuguanling
 * Date:   2023-1-8
 * Description: use TMI8150 for driver motor(ptz)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/watchdog.h>

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <linux/rtc.h>

#include <sys/mman.h>
#include <sys/reboot.h>

//by AW-K.L
#include <linux/spi/spidev.h>
#include <unistd.h>

#define AW_ERRO(err_info)                                                      \
	do {                                                                   \
		printf err_info;                                               \
	} while (0)

#define AW_DEBUG(dbg_info)                                                     \
	do {                                                                   \
		printf dbg_info;                                               \
	} while (0)
#define aw_debug printf
//end AW-K.L

#define SPI_DEV "/dev/spidev3.0"; //这里需要根据不同的节点进行更改。

static int g_fd = -1;
static pthread_mutex_t gSpiMutex = PTHREAD_MUTEX_INITIALIZER;

//毫秒
static void delay_us(unsigned int nMillisecond)
{
	usleep(nMillisecond);
	return;
}

int sensor_spi_init(void)
{
	if (g_fd >= 0) {
		AW_ERRO(("!!!!!! {%s_%d}\n", __func__, __LINE__));
		return 0;
	}

	unsigned int value;
	int ret = 0;
	char file_name[] = SPI_DEV;

	g_fd = open(file_name, O_RDWR);
	if (g_fd < 0) {
		AW_DEBUG(("!!!!!!!!!!! Open %s error!\n", file_name));
		return -1;
	}

	value = 0; //SPI_MODE_3 | SPI_LSB_FIRST;
	ret = ioctl(g_fd, SPI_IOC_WR_MODE, &value);
	if (ret < 0) {
		AW_DEBUG(("ioctl SPI_IOC_WR_MODE err, value = %d ret = %d\n",
			  value, ret));
		return ret;
	}

	value = 8;
	ret = ioctl(g_fd, SPI_IOC_WR_BITS_PER_WORD, &value);
	if (ret < 0) {
		AW_DEBUG((
			"ioctl SPI_IOC_WR_BITS_PER_WORD err, value = %d ret = %d\n",
			value, ret));
		return ret;
	}

	value = 500000; //work clock
	ret = ioctl(g_fd, SPI_IOC_WR_MAX_SPEED_HZ, &value);
	if (ret < 0) {
		AW_DEBUG((
			"ioctl SPI_IOC_WR_MAX_SPEED_HZ err, value = %d ret = %d\n",
			value, ret));
		return ret;
	}
	pthread_mutex_init(&gSpiMutex, NULL);
	return 0;
}

int sensor_spi_exit(void)
{
	if (g_fd >= 0) {
		close(g_fd);
		g_fd = -1;
		return 0;
	}
	pthread_mutex_destroy(&gSpiMutex);
	return -1;
}

int sensor_write_register(unsigned char addr, unsigned int data)
{
	int ret = 0;
	int i = 0;
	int gpio_value = 0;
	struct spi_ioc_transfer mesg[1];
	unsigned char tx_buf[8] = { 0 };
	unsigned char rx_buf[8] = { 0 };

	tx_buf[0] = addr + 0x80;
	tx_buf[1] = (data % 256);
	tx_buf[2] = (data / 256);

#if 0
	aw_debug("write data: \n");
	for (i = 0; i < 2; i++)
		AW_DEBUG(("=============== 0x%x\n", tx_buf[i]));
#endif

	memset(mesg, 0, sizeof(mesg));
	mesg[0].tx_buf = (void *)tx_buf;
	mesg[0].len = 2;
	mesg[0].rx_buf = (void *)rx_buf;
	mesg[0].speed_hz = 500000;
	mesg[0].bits_per_word = 8;
	//mesg[0].delay_usecs = 100;
	//mesg[0].cs_change = 1;

	if (g_fd < 0) {
		AW_DEBUG(("SPI_IOC_MESSAGE error \n"));
		return -1;
	}

	pthread_mutex_lock(&gSpiMutex);
#if 1
	ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	if (ret < 0) {
		pthread_mutex_unlock(&gSpiMutex);
		AW_DEBUG(("SPI_IOC_MESSAGE error \n"));
		return -1;
	}
#endif

	pthread_mutex_unlock(&gSpiMutex);
	return 0;
}

unsigned int sensor_read_register(unsigned char addr)
{
	int ret = 0;
	int i = 0;
	int gpio_value = 0;
	struct spi_ioc_transfer mesg[1];
	unsigned char tx_buf[8] = { 0 };
	unsigned char rx_buf[8] = { 0 };

	tx_buf[0] = addr;
	tx_buf[1] = 0;
	tx_buf[2] = 0;
	tx_buf[3] = 0;
#if 0
	aw_debug("read addr: \n");
	for (i = 0; i < 2; i++) {
		printf("=============== 0x%x\n", tx_buf[i]);
	}
#endif

	memset(mesg, 0, sizeof(mesg));
	mesg[0].tx_buf = (void *)tx_buf;
	mesg[0].len = 2;
	mesg[0].rx_buf = (void *)rx_buf;
	mesg[0].speed_hz = 500000;
	mesg[0].bits_per_word = 8;
	//mesg[0].delay_usecs = 100;
	//mesg[0].cs_change = 1;

	if (g_fd < 0) {
		AW_DEBUG(("SPI_IOC_MESSAGE error \n"));
		return -1;
	}

#if 1

	ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	if (ret < 0) {
		AW_DEBUG(("SPI_IOC_MESSAGE error \n"));
		return -1;
	}

#endif

#if 0
	aw_debug("read data: \n");
	for (i = 0; i < 2; i++) {
		printf("=============== 0x%x\n", rx_buf[i]);
	}
#endif

	return ((rx_buf[1]) + (rx_buf[0] << 8));
}

/****************************************************************************
* 名    称：void TMI8150_init(void)
* 功    能：TMI8150初始化
* 入口参数: 无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
int TMI8150_init(void)
{
	int ret = -1;
	int i = 0;
	unsigned int reg_value = 0;
	unsigned char val = 0;

	ret = sensor_spi_init();
	if (ret < 0) {
		AW_DEBUG(
			("[%s,%d]  %s\n", __func__, __LINE__, strerror(errno)));
		return -1;
	}

	/*GPIO 228 EN 拉高 TMI8150芯片的使能脚*/
	//TODO GPIO EN
	system("echo 228 > /sys/class/gpio/export");
	system("echo high > /sys/class/gpio/gpio228/direction");

	/*reset chip*/
	val = (unsigned char)sensor_read_register(0x02);
	val &= 0x7f; //第7为写0 复位
	sensor_write_register(0x02, val);
	do {
		usleep(10000);
		val = (unsigned char)sensor_read_register(0x02);
		val |= 0x80; //第7为写1 清除复位
		sensor_write_register(0x02, val);
		val = (unsigned char)sensor_read_register(0x02);
		val &= 0x80; //第七位读出 1 是处于工作状态， 0是复位中
		usleep(10000);
	} while (!val);

	/*将圈数/ 计数清零*/
	sensor_write_register(0x0b, 0x00);
	sensor_write_register(0x0c, 0x00);
	sensor_write_register(0x08, 0x00);
	sensor_write_register(0x09, 0x00);
	/*将圈数/ 计数清零*/
	sensor_write_register(0x1b, 0x00);
	sensor_write_register(0x1c, 0x00);
	sensor_write_register(0x18, 0x00);
	sensor_write_register(0x19, 0x00);

	/*初始化 ab 轴*/
	sensor_write_register(0x04, 0x07); //set 手动控制
	sensor_write_register(0x06, 0x03); //设置X速度

	/*初始化 cb 轴*/
	sensor_write_register(0x14, 0x07); //set 手动控制
	sensor_write_register(0x16, 0x03); //设置Y速度

	aw_debug("TMI8150 init success \n");

	return 0;
}

int aw_spi_init()
{
	int ret = -1;
	ret = TMI8150_init();
	if (ret < 0) {
		AW_DEBUG(
			("[%s,%d]  %s\n", __func__, __LINE__, strerror(errno)));
		return -1;
	}
	return 0;
}
