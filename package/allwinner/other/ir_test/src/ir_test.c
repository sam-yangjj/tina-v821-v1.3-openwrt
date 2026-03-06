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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/lirc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define NEC_NBITS		32
#define NEC_UNIT		562500  /* ns */
#define NEC_HEADER_PULSE	(16 * NEC_UNIT) /* 9ms */
#define NECX_HEADER_PULSE	(8  * NEC_UNIT) /* Less common NEC variant */
#define NEC_HEADER_SPACE	(8  * NEC_UNIT) /* 4.5ms */
#define NEC_REPEAT_SPACE	(4  * NEC_UNIT) /* 2.25ms */
#define NEC_BIT_PULSE		(1  * NEC_UNIT)
#define NEC_BIT_0_SPACE		(1  * NEC_UNIT)
#define NEC_BIT_1_SPACE		(3  * NEC_UNIT)
#define	NEC_TRAILER_PULSE	(1  * NEC_UNIT)
#define	NEC_TRAILER_SPACE	(10 * NEC_UNIT) /* even longer in reality */

#define NS_TO_US(nsec)		((nsec) / 1000)

#define GPIO_IR_RAW_BUF_SIZE 128
#define DEFAULT_CARRIER_FREQ 38000
#define DEFAULT_DUTY_CYCLE   33
/*#define DEFAULT_CARRIER_FREQ 25000
#define DEFAULT_DUTY_CYCLE   25*/

uint32_t rx_raw_buf[GPIO_IR_RAW_BUF_SIZE];
uint32_t tx_raw_buf[GPIO_IR_RAW_BUF_SIZE];

static int fd;
static int fd1;
struct pollfd poll_fds[1];
static int int_exit;
static int kernel_flag;
pthread_t tid;

static void print_usage(const char *argv0)
{	printf("usage: %s [options]\n", argv0);
	printf("\n");
	printf("ir receive test:\n");
	printf("\tir_test rx\n");
	printf("\n");
	printf("ir send test:\n");
	printf("\tir_test tx <code>\n");
	printf("\texample:ir_test tx 0x4fb23dc\n");
	printf("\n");
	printf("ir loop test, rx&tx:\n");
	printf("\tir_test loop\n");
	printf("\n");
}

static int nec_modulation_byte(uint32_t *buf, uint8_t code)
{
	int i = 0;
	uint8_t mask = 0x01;

	while (mask) {
		/*low bit first*/
		if (code & mask) {
			/*bit 1*/
			*(buf + i) = LIRC_PULSE(NS_TO_US(NEC_BIT_PULSE));
			*(buf + i + 1) = LIRC_SPACE(NS_TO_US(NEC_BIT_1_SPACE));
		} else {
			/*bit 0*/
			*(buf + i) = LIRC_PULSE(NS_TO_US(NEC_BIT_PULSE));
			*(buf + i + 1) = LIRC_SPACE(NS_TO_US(NEC_BIT_0_SPACE));
		}
		mask <<= 1;
		i += 2;
	}
	return i;
}

static int nec_ir_encode(uint32_t *raw_buf, uint32_t key_code)
{
	uint8_t address, not_address, command, not_command;
	uint32_t *head_p, *data_p, *stop_p;

	address	= (key_code >> 24) & 0xff;
	not_address = (key_code >> 16) & 0xff;
	command	= (key_code >>  8) & 0xff;
	not_command = (key_code >>  0) & 0xff;
	printf("address:0x%02x ~address:0x%02x\n", address, not_address);
	printf("command:0x%02x ~command:0x%02x\n", command, not_command);
	/*head bit*/
	head_p = raw_buf;
	*(head_p) = LIRC_PULSE(NS_TO_US(NEC_HEADER_PULSE));
	*(head_p + 1) = LIRC_SPACE(NS_TO_US(NEC_HEADER_SPACE));

	/*data bit*/
	data_p = raw_buf + 2;
	nec_modulation_byte(data_p,  address);

	data_p += 16;
	nec_modulation_byte(data_p,  not_address);

	data_p += 16;
	nec_modulation_byte(data_p,  command);

	data_p += 16;
	nec_modulation_byte(data_p,  not_command);

	/*stop bit*/
	stop_p = data_p + 16;
	*(stop_p) = LIRC_PULSE(NS_TO_US(NEC_TRAILER_PULSE));
	*(stop_p + 1) = LIRC_SPACE(NS_TO_US(NEC_TRAILER_SPACE));

	/*return the total size of nec protocal pulse*/
	/* linux-5.4 needs 67 byte datas */
	if (kernel_flag == 1)
		return ((NEC_NBITS + 2) * 2 - 1);
	else
		return ((NEC_NBITS + 2) * 2);
}

const float offset_percent = 0.10;
static int within_tolerance(unsigned int value, unsigned int target)
{
	int tolerance = (int)target * offset_percent;
	return value >= (target - tolerance) && value <= (target + tolerance);
}

static int parse_bit(unsigned int pulse, unsigned int space) {
	if (!within_tolerance(pulse, NS_TO_US(NEC_UNIT))) {
		return -1;
	}

	if (within_tolerance(space, NS_TO_US(NEC_BIT_0_SPACE))) {
		return 0;
	} else if (within_tolerance(space, NS_TO_US(NEC_BIT_1_SPACE))) {
		return 1;
	}

	return -1;
}

static int nec_ir_decode(unsigned int *buf, int size, unsigned int *result)
{
	int i,bit_index;
	unsigned char address_byte = 0;
	unsigned char address_inv_byte = 0;
	unsigned char command_byte = 0;
	unsigned char command_inv_byte = 0;

	// head:2 data_32bit:64 stop:1
	if (size < 67) {
		printf("width num must over than 67\n");
		printf("head width num:2 data width num:64 stop width num:1\n");
		return -1;
	}

	// head
	if ((within_tolerance(buf[0], NS_TO_US(NEC_HEADER_PULSE)) == 0)
		|| (within_tolerance(buf[1], NS_TO_US(NEC_HEADER_SPACE)) == 0)) {
		printf("header pulse width error\n");
		return -1;
	}

	// data
	bit_index = 0;
	for (int i = 2; i < (64 + 2); i += 2) {
        int bit = parse_bit(buf[i], buf[i + 1]);
        if (bit == -1) {
			printf("parse bit failed i:%d\n",i);
            return -1;
        }

        if (bit_index < 8) {
            address_byte |= (bit << (bit_index));
        } else if (bit_index < 16) {
            address_inv_byte |= (bit << (bit_index - 8));
        } else if (bit_index < 24) {
            command_byte |= (bit << (bit_index - 16));
        } else if (bit_index < 32) {
            command_inv_byte |= (bit << (bit_index - 24));
        }

		bit_index++;
	}

	if (address_inv_byte != (~address_byte & 0xFF)) {
		printf("address check error!\n");
		return -1;
	}

	if (command_inv_byte != (~command_byte & 0xFF)) {
		printf("command check error!\n");
		return -1;
	}

	// *address = address_byte;
	// *command = command_byte;
	*result = address_byte << 24;
	*result |= address_inv_byte << 16;
	*result |= command_byte << 8;
	*result |= command_inv_byte;
	return 0;
}

void *ir_recv_thread(void *arg)
{
	int size = 0, size_t = 0;
	int i = 0;
	int ret;
	int total = 0;
	unsigned int result;

	poll_fds[0].fd = fd;
	poll_fds[0].events = POLLIN | POLLERR;
	poll_fds[0].revents = 0;
	while (!int_exit) {
		ret = poll(poll_fds, 1, 12);
		if (!ret) {
			//printf("time out\n");
			// printf("\n--------------------\n");
			if (total) {
				printf("recv %d nec pulse width(microsecond):\n", total);
				size_t = total / sizeof(uint32_t);
				for (i = 0; i < size_t; i++) {
					rx_raw_buf[i] = rx_raw_buf[i] & 0xffffff;
					printf("%d ", rx_raw_buf[i]);
					if ((i) && (i % 8 == 0))
						printf("\n");
				}
				printf("\n");
				if (nec_ir_decode(rx_raw_buf, size_t, &result) == 0) {
					printf("decode result:0x%08x\n",result);
				}
			}
			total = 0;
		} else {
			if (poll_fds[0].revents == POLLIN) {
				size = read(fd, (char *)(rx_raw_buf) + total,
						GPIO_IR_RAW_BUF_SIZE);
				total += size;
				// size_t = size / sizeof(uint32_t);
				// printf("size:%d size_t:%d\n",size,size_t);
				// printf("Get nec pulse width(microsecond):\n");
				// for (i = 0; i < size_t; i++) {
				// 	dura = rx_raw_buf[i] & 0xffffff;
				// 	printf("%d ", dura);
				// 	if ((total++) % 8 == 0)
				// 		printf("\n");
				// }
			}
		}
	}

	return NULL;
}

void ir_test_close(int sig)
{	/* allow the stream to be closed gracefully */
	signal(sig, SIG_IGN);
	int_exit = 1;
	if (fd1 != fd)
		close(fd1);
	close(fd);
}

int main(int argc, char **argv)
{
	int ret;
	int size = 0, size_t = 0;
	int i = 0;
	int duty_cycle, carrier_freq;
	int key_code = 0;
	int err = 0;
	int cnt = 0;
	int swap;
	/* use for check the current kernel version */
	int fd_version;
	char version_buf[256];

	fd_version = open("/proc/version", O_RDONLY);
	if (fd_version < 0) {
		printf("The system is not mount proc, please check\n");
		return -1;
	}
	ret = read(fd_version, version_buf, sizeof(version_buf));
	if (ret < 0) {
		printf("Can't not read /proc/verison\n");
		return -1;
	}
	switch(version_buf[14]) {
		case '4':
			kernel_flag = 0;
			printf("This is 4.* linux kernel\n");
			break;
		case '5':
			kernel_flag = 1;
			printf("This is 5.* linux kernel\n");
			break;
		default:
			kernel_flag = 0;
			printf("This is %s.* kernel which is not support\n", version_buf[14]);
			break;
	}
	close(fd_version);

	/* catch ctrl-c to shutdown cleanly */
	signal(SIGINT, ir_test_close);

	if (argc < 2) {
		print_usage(argv[0]);
		return -1;
	}

	fd = open("/dev/lirc0", O_RDWR);
	if (fd < 0) {
		printf("can't open lirc0 recv, check driver!\n");
		return 0;
	} else {
		printf("lirc0 open succeed.\n");
	}

	fd1 = open("/dev/lirc1", O_RDWR);
	if (fd1 < 0) {
		printf("can't open lirc1 send, check driver!\n");
		fd1 = fd;
		printf("There is no lirc1, use the lirc0 as lirc1.\n");
	} else {
		printf("lirc1 open succeed.\n");
	}

	/* default lirc0 is rx, lirc1 is tx */
	/* If path exists, lirc0 is tx */
	if (access("/sys/class/rc/rc0/irtx", F_OK) == 0) {
		printf("lirc0 is tx, so need swap fd\n");
		swap = fd;
		fd = fd1;
		fd1 = swap;
	}

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-n")) {
			if (i + 1 >= argc) {
				printf("Option -n expects an argument.\n\n");
				print_usage(argv[0]);
				goto OUT;
			}
			continue;
		}
	}

	if (!strcmp(argv[1], "rx")) {
		err = pthread_create(&tid, NULL, (void *)ir_recv_thread, NULL);
		if (err != 0) {
			printf("create pthread error: %d\n", __LINE__);
			goto OUT;
		}

		do {
			usleep(1000);
		} while (!int_exit);

	} else if (!strcmp(argv[1], "tx")) {
		if (argc < 3) {
			fprintf(stderr, "No data passed\n");
			goto OUT;
		}

		if (sscanf(argv[2], "%x", &key_code) != 1) {
			fprintf(stderr, "no input data: %s\n", argv[2]);
			goto OUT;
		}

		duty_cycle = DEFAULT_DUTY_CYCLE;
		if (ioctl(fd1, LIRC_SET_SEND_DUTY_CYCLE, &duty_cycle)) {
			fprintf(stderr,
				"lirc0: could not set carrier duty: %s\n",
				strerror(errno));
			goto OUT;
		}

		carrier_freq = DEFAULT_CARRIER_FREQ;
		if (ioctl(fd1, LIRC_SET_SEND_CARRIER, &carrier_freq)) {
			fprintf(stderr,
				"lirc0: could not set carrier freq: %s\n",
				strerror(errno));
			goto OUT;
		}

		printf("irtest: send key code : 0x%x\n", key_code);

		size = nec_ir_encode(tx_raw_buf, key_code);
		/*dump the raw data*/
		printf("send nec pulse width(microsecond):\n");
		for (i = 0; i < size; i++) {
			printf("%d ", *(tx_raw_buf + i) & 0x00FFFFFF);
			if ((i + 1) % 8 == 0)
				printf("\n");
		}
		printf("\n");

		/* linux-5.4 IR core should delete bit24~bit31 */
		if (kernel_flag == 1) {
			for (i = 0; i < size; i++) {
				tx_raw_buf[i] = (tx_raw_buf[i] & 0x00FFFFFF);
			}
		}

		if (argc > 4 && !strcmp(argv[3], "-n")) {
                        cnt = atoi(argv[4]);
                        for (i = 0; i < cnt; i++) {
                                size_t = size * sizeof(uint32_t);
                                ret = write(fd1, (char *)tx_raw_buf, size_t);
                                if (ret > 0)
                                        printf("irtest No.%d: send %d bytes ir raw data\n\n",
                                                i, ret);
				else
					printf("irtest No.%d: send %d failed!\n", i, ret);
				usleep(100*1000);
                        }
                } else {
			size_t = size * sizeof(uint32_t);
			ret = write(fd1, (char *)tx_raw_buf, size_t);
			if (ret > 0)
				printf("irtest: send %d bytes ir raw data\n\n", ret);
			else
				printf("irtest: send %d failed\n", ret);
		}

	} else if (!strcmp(argv[1], "loop")) {

		/*code: 0x13*/
		key_code = 0x04fb13ec;
		err = pthread_create(&tid, NULL, (void *)ir_recv_thread, NULL);
		if (err != 0) {
			printf("create pthread error: %d\n", __LINE__);
			goto OUT;
		}

		duty_cycle = DEFAULT_DUTY_CYCLE;
		if (ioctl(fd1, LIRC_SET_SEND_DUTY_CYCLE, &duty_cycle)) {
			fprintf(stderr,
				"lirc0: could not set carrier duty: %s\n",
				strerror(errno));
			goto OUT;
		}
		carrier_freq = DEFAULT_CARRIER_FREQ;
		if (ioctl(fd1, LIRC_SET_SEND_CARRIER, &carrier_freq)) {
			fprintf(stderr,
				"lirc0: could not set carrier freq: %s\n",
				strerror(errno));
			goto OUT;
		}

		/*echo 50ms transmit one frame */

		if (argc > 3 && !strcmp(argv[2], "-n")) {
			cnt = atoi(argv[3]);
			for (i = 0; i < cnt; i++) {
				size = nec_ir_encode(tx_raw_buf, key_code);
				size_t = size * sizeof(uint32_t);
				/* linux-5.4 IR core should delete bit24~bit31 */
				if (kernel_flag == 1) {
					for (i = 0; i < size; i++) {
						tx_raw_buf[i] = (tx_raw_buf[i] & 0x00FFFFFF);
					}
				}
				printf("send key code : 0x%x, No.%d\n", key_code, i);
				write(fd1, (char *)tx_raw_buf, size_t);
				usleep(100*1000);
				if (int_exit)
					break;
			}
                } else {
                        i = 0;
			do {
				size = nec_ir_encode(tx_raw_buf, key_code);
				size_t = size * sizeof(uint32_t);
				/* linux-5.4 IR core should delete bit24~bit31 */
				if (kernel_flag == 1) {
					for (i = 0; i < size; i++) {
						tx_raw_buf[i] = (tx_raw_buf[i] & 0x00FFFFFF);
					}
				}
				printf("send key code : 0x%x, %d\n", key_code, i++);
				write(fd1, (char *)tx_raw_buf, size_t);
				usleep(100*1000);
			} while (!int_exit);
		}
	} else if (!strcmp(argv[1], "-n")) {
		print_usage(argv[0]);
	}

OUT:
	if (fd1 != fd)
		close(fd1);
	close(fd);
	return 0;
}
