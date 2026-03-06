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
#include <stdio.h> /*标准输入输出定义*/
#include <stdlib.h> /*标准函数库定义*/
#include <unistd.h> /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /*文件控制定义*/
#include <termios.h> /*PPSIX 终端控制定义*/
#include <errno.h> /*错误号定义*/
#include <string.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <sys/time.h>

enum parameter_type {
	PT_PROGRAM_NAME = 0,
	PT_DEV_NAME,
	PT_CYCLE,
	PT_NUM
};

int interface = 0;
int baudrate = 115200;
int bits = 8;
char event = 'N';
int stop_bit = 0;
int data_size = 16;
int cnt = 100;
int port = 1;
int flow_ctrl = 0;
int debug_flag = 0;
int loopback = 0;
int time_size = 0;
int rtx = 0;
int debug_time_flag = 0;

#define DBG(string, args...) \
do { \
	printf("%s, %s()%d---", __FILE__, __FUNCTION__, __LINE__); \
	printf(string, ##args); \
	printf("\n"); \
} while (0)

static int OpenDev(char *name)
{
	int fd = open(name, O_RDWR ); //| O_NOCTTY | O_NDELAY

	if (-1 == fd)
		DBG("Can't Open(%s)!", name);

	return fd;
}

static int set_rs485_config(int fd)
{
	int ret;
	struct serial_rs485 rs485_config;

	ret = ioctl (fd, TIOCGRS485, &rs485_config);
	if (ret < 0) {
		printf("Can not get rs485 config, ioctl failed %d (errno: %d)\n", ret, errno);
		return -1;
	}

	rs485_config.flags = SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND;
	rs485_config.delay_rts_before_send = 10; // 发送前延迟 10ms，目前没有生效
	rs485_config.delay_rts_after_send = 5; // 发送后延迟 5ms，目前没有生效

	ret = ioctl (fd, TIOCSRS485, &rs485_config);
	if (ret < 0) {
		printf("Can not set rs485 config, ioctl failed %d (errno: %d)\n", ret, errno);
		return -1;
	}

	return 0;
}

static int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;

	if (tcgetattr(fd, &oldtio) != 0) {
		perror("Setup Serial fail");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD; //CLOCAL:忽略modem控制线 CREAD：打开接受者
	newtio.c_cflag &= ~CSIZE; //字符长度掩码。取值为：CS5，CS6，CS7或CS8
	switch(nBits) {
		case 5: newtio.c_cflag |= CS5; break;
		case 6: newtio.c_cflag |= CS6; break;
		case 7: newtio.c_cflag |= CS7; break;
		case 8: newtio.c_cflag |= CS8; break;
	}
	switch(nEvent) {
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD; //奇校验
			newtio.c_iflag |= INPCK;
		break;
		case 'E':
			newtio.c_iflag |= INPCK;
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD; //偶校验
		break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
		break;
	}
	printf("nSpeed:%d\n", nSpeed);
	switch(nSpeed) {
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		case 1000000:
			cfsetispeed(&newtio, B1000000);
			cfsetospeed(&newtio, B1000000);
			break;
		case 1500000:
			cfsetispeed(&newtio, B1500000);
			cfsetospeed(&newtio, B1500000);
			break;
		case 3000000:
			cfsetispeed(&newtio, B3000000);
			cfsetospeed(&newtio, B3000000);
			break;
		case 4000000:
			cfsetispeed(&newtio, B4000000);
			cfsetospeed(&newtio, B4000000);
			break;
		default: {
			#if 1
			struct serial_struct ss;
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			tcflush(fd, TCIFLUSH);
			tcsetattr(fd, TCSANOW, &newtio);
			if ((ioctl(fd, TIOCGSERIAL, &ss) < 0))
				printf("-----ioctl failed\n");
			ss.flags = ASYNC_SPD_CUST;
			ss.custom_divisor = ss.baud_base/nSpeed;
			/*ss.custom_divisor = ss.baud_base/330000;*/
			printf("baud base:%u\n", ss.baud_base);
			if ((ioctl(fd, TIOCSSERIAL, &ss) < 0))
				printf("-----ioctl failed\n");
			#endif
		}
		break;
	}
	if (nStop == 1)
		newtio.c_cflag &= ~CSTOPB; //CSTOPB:设置两个停止位，而不是一个
	else if (nStop == 2)
		newtio.c_cflag |= CSTOPB;

	newtio.c_cc[VTIME] = 0; //VTIME:非cannoical模式读时的延时，以十分之一秒位单位
	newtio.c_cc[VMIN] = 0; //VMIN:非canonical模式读到最小字符数

	if (flow_ctrl == 1)
		newtio.c_cflag |= CRTSCTS;
	else
		newtio.c_cflag &= ~CRTSCTS;

	tcflush(fd,TCIFLUSH); // 改变在所有写入 fd 引用的对象的输出都被传输后生效，所有已接受但未读入的输入都在改变发生前丢弃。
	if ((tcsetattr(fd,TCSANOW,&newtio))!=0) //TCSANOW:改变立即发生
	{
		perror("com set error"); return -1;
	}
	printf("set done!\n\r");
	return 0;
}

static void str_print(char *buf, int len, int r)
{
	int i;

	if (debug_flag == 0)
		return;

	if (r == 1)
		printf("==== read data ====\n");
	else
		printf("==== write data ====\n");

	for (i=0; i<len; i++) {
		if (i%16 == 0)
			printf("\n");
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
}

static int buffer_verify(char *src, char *dst, unsigned long len)
{
	int ret = 0;
	unsigned long i;
	unsigned long err_bit = 0;

	for (i = 0; i < len; i++) {
		if (src[i] == dst[i])
			continue;
		err_bit ++;
		ret = -1;
	}
	if (err_bit)
		printf("verify error bit:%ld\n", err_bit);

	return ret;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-dbcsf]\n", prog);
	puts("  -d  uart to test, input 0/1/2... (default uart1)\n"
		 "  -I  interface (0: normal, 485: rs485, defalut 0)\n"
		 "  -b  baudate (defalut 115200)\n"
		 "  -n  test cycle (default 100 times)\n"
		 "  -s  data size (default 16 Bytes)\n"
		 "  -t  print the transmit information after a few times(default 10 times)\n"
		 "  -f  flow control (default n)\n"
		 "  -l  loopback (default n)\n"
		 "  -g  set to open debug log\n"
		 "  -m  change to recive mode(default is trans mode)\n"
		 "  -p  print the speed info (default n)\n"
		 "  -i  set data bits\n"
		 "  -z  set parity (O: odd parity, E:even parity)\n"
		 "  -w  set stop bit (1, 2)\n"
		 "  -h  print help\n");
	printf("if you want to test urat speed, please set recive mode first\n");
	printf("speed test(trans) eg:uart_test -d 1 -n 100 -t 10 -p\n");
	printf("speed test(recive) eg:uart_test -d 1 -n 100 -t 10 -p -m\n");
	printf("loopback eg:uart_test -d 1 -b 1500000 -n 10 -s 256 -l -g\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		int c;

		c = getopt(argc, argv, "d:I:b:n:s:t:i:z:w:fpgrhlm");

		if (c == -1)
			break;
		switch (c) {
		case 'd':
			port = atoi(optarg);
			break;
		case 'I':
			interface = atoi(optarg);
			break;
		case 'b':
			baudrate = atoi(optarg);
			break;
		case 'n':
			cnt = atoi(optarg);
			break;
		case 's':
			data_size = atoi(optarg);
			break;
		case 't':
			time_size = atoi(optarg);
			break;
		case 'f':
			flow_ctrl = 1;
			break;
		case 'p':
			debug_time_flag = 1;
			break;
		case 'g':
			debug_flag = 1;
			break;
		case 'l':
			loopback = 1;
			break;
		case 'h':
			print_usage(argv[0]);
			break;
		case 'm':
			rtx = 1;
			break;
		case 'i':
			bits = atoi(optarg);
			break;
		case 'z':
			event = *optarg;
			break;
		case 'w':
			stop_bit = atoi(optarg);
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	struct timeval dly_tm, start, end;
	int i = 0, err_cnt = 0, suc_cnt = 0, io_cnt = 0, ret = 0, fd = 0, temp = 0, fd_loop, x;
	int sec = 0, usec = 0, final_sec = 0, final_usec = 0, ave_time = 0;
	int write_size = 0, read_size = 0;
	char *read_buf, *write_buf;
	char buf[256];
	fd_set rd_fdset;

	char uart_port[16] = {0};

	parse_opts(argc, argv);


	sprintf(uart_port, "/dev/ttyAS%d", port);
	if (access(uart_port, F_OK) != 0) {
		sprintf(uart_port, "/dev/ttyS%d", port);
	}

	printf("=======%s test args=======\n", argv[0]);
	printf("uart_port: %s.\n", uart_port);
	printf("interface: %d.\n", interface);
	printf("baudrate: %d.\n", baudrate);
	printf("bits: %d.\n", bits);
	printf("event: %c.\n", event);
	printf("stop_bit: %d.\n", stop_bit);
	printf("test cycle: %d.\n", cnt);
	printf("bytes_per_cycle: %d.\n", data_size);
	printf("flow ctrl:%s.\n", flow_ctrl?"true":"false");
	printf("debug log:%s.\n", debug_flag?"true":"false");
	printf("%s\n", rtx?"recive mode":"trans mode");
	printf("\n");

	write_buf = (char *)malloc(data_size * sizeof(char));
	if (NULL == write_buf) {
		printf("malloc for write_buf error\n");
		return -1;
	}

	read_buf = (char *)malloc(data_size * sizeof(char));
	if (NULL == read_buf) {
		printf("malloc for read_buf error\n");
		free(write_buf);
		write_buf = NULL;
		return -1;
	}
	memset(write_buf, 0, data_size * sizeof(char));
	memset(read_buf, 0, data_size * sizeof(char));

	fd = OpenDev(uart_port);
	if (fd < 0) {
		printf("open uart%d failed\n", port);
		goto help;
	}

	set_opt(fd, baudrate, bits, event, stop_bit);

	if (interface == 485)
		set_rs485_config(fd);

	printf("Select(%s), Cnt %d. \n", uart_port, cnt);
	char loopback_port[64];
	if (loopback == 1) {
		sprintf(loopback_port, "/sys/devices/platform/soc/uart%d/loopback", port);
		fd_loop = OpenDev(loopback_port);
		if (fd_loop < 0) {
			if (strncmp(uart_port, "/dev/ttyAS", 9) == 0) {
				sprintf(loopback_port, "/sys/class/tty/ttyAS%d/device/loopback", port);
			} else {
				sprintf(loopback_port, "/sys/class/tty/ttyS%d/device/loopback", port);
			}
			printf("try to open %s\n", loopback_port);
			fd_loop = OpenDev(loopback_port);
		}
		write(fd_loop, "enable", 7);
	}

	i = 0;

	if (!rtx) {
		while (i < cnt) {
			tcflush(fd,TCIOFLUSH);
			gettimeofday(&start, NULL);
			for (x = 0; x < data_size; x++)
				write_buf[x] = x;
			write_size = write(fd, write_buf, data_size);
			if (debug_flag == 1)
				printf("[cnt:%d]:transmit %d bytes.\n", i, write_size);
			str_print(write_buf, write_size, 0);
			usleep(200000);

			FD_ZERO(&rd_fdset);
			FD_SET(fd, &rd_fdset);

			dly_tm.tv_sec = 10;
			dly_tm.tv_usec = 0;

			i++;
			ret = select(fd+1, &rd_fdset, NULL, NULL, &dly_tm);
			if (ret == 0) {
				printf("%d:select timeout\n", i);
				err_cnt++;
				continue;
			}
			if (ret < 0) {
				printf("%d:select(%s) return %d. [%d]: %s \n", i, uart_port, ret, errno,
						strerror(errno));
				err_cnt++;
				continue;
			}
			memset(read_buf, 0, data_size * sizeof(char));
			read_size = read(fd, read_buf, data_size);
			str_print(read_buf, read_size, 1);
			gettimeofday(&end, NULL);
			tcflush(fd,TCIFLUSH);

			if (buffer_verify(read_buf, write_buf, data_size) != 0) {
				err_cnt++;
				printf("transfer error occurred, err_cnt:%d\n", err_cnt);
			}
			io_cnt++;
			if (debug_time_flag == 1) {
				sec += end.tv_sec - start.tv_sec;
				usec += end.tv_usec - start.tv_usec;
				if (io_cnt % time_size == 0) {
					printf("usetime:  %ds %dus\n", sec, usec);
					final_sec += sec;
					final_usec += usec;
					sec = 0;
				}
				if (cnt == i)
					printf("The test finally speed: %ds %dus\n", final_sec, final_usec);
			}
		}
		suc_cnt = io_cnt - err_cnt;
		printf("total test cnt:%d, succeed cnt:%d, error cnt:%d\n", io_cnt, suc_cnt, err_cnt);
		usec = 0;
		io_cnt = 0;
		err_cnt = 0;
	} else {
		while (1) {
			FD_ZERO(&rd_fdset);
			FD_SET(fd, &rd_fdset);

			dly_tm.tv_sec = 10;
			dly_tm.tv_usec = 0;

			i++;
			ret = select(fd+1, &rd_fdset, NULL, NULL, &dly_tm);
			if (ret == 0) {
				printf("%d:select timeout\n", i);
				if (i > 10) {
					printf("error, test failed\n");
					break;
				}
				continue;
			}
			if (ret < 0) {
				printf("%d:select(%s) return %d. [%d]: %s \n", i, uart_port, ret, errno,
						strerror(errno));
				if (i > 10) {
					printf("error, test failed\n");
					break;
				}
				continue;
			}
			memset(read_buf, 0, data_size * sizeof(char));
			read_size = read(fd, read_buf, data_size);
			tcflush(fd, TCIFLUSH);
			tcflush(fd, TCIOFLUSH);
			write_size = write(fd, read_buf, data_size);
			if (debug_flag == 1)
				printf("%d:transmit %d bytes.\n", i, write_size);
			str_print(read_buf, write_size, 0);
		}
	}
	printf("Uart test complete\n");
	free(read_buf);
	free(write_buf);
	close(fd);
	if (loopback == 1)
		close(fd_loop);
	return err_cnt;
help:
	print_usage(argv[0]);
	return -1;
}
