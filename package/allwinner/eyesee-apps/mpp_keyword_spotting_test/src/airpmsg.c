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
#include "librpbuf.h"
#include "librpmsg.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "airpmsg.h"
//rpmsg
const char *ctrl_name = "e907_rproc@43030000";
const char *ept_name = "rpmsg_lp";
struct rpmsg_ept_instance {
	int ept_id;
	int ept_fd;
};
#define RPBUF_NAME_SIZE	32	/* include trailing '\0' */

struct rpbuf_buffer_info {
	char name[RPBUF_NAME_SIZE];
	__u32 len;
};

struct __rpbuf_buffer {
	struct rpbuf_buffer_info info;
	int ctrl_fd;
	int buf_fd;
	void *addr;
};
typedef struct __rpbuf_buffer rpbuf_buffer_t;


struct rpmsg_ept_instance *eptdev;

int tkl_wifi_rpmsg_init(void)
{
	int ept_id = -1;
	int ret = 0;
	char ept_file_path[64];
	char send_buf[RPMSG_DATA_MAX_LEN] = { 0 };

	ret = rpmsg_alloc_ept(ctrl_name, ept_name);
	if (ret < 0) {
		printf("[lp_rpmsg]: rpmsg_alloc_ept for ept %s (%s) failed\n", ept_name, ctrl_name);
		ret = -1;
		goto out;
	}
	ept_id = ret;

	eptdev = malloc(sizeof(*eptdev));
	if (!eptdev) {
		printf("[lp_rpmsg]: malloc eptdev for rpmsg%d failed\r\n", ept_id);
		ret = -ENOMEM;
		goto free_ept;
	}
	memset(eptdev, 0, sizeof(*eptdev));
	eptdev->ept_id = ept_id;

	return 0;
free_ept:
	ret = rpmsg_free_ept(ctrl_name, ept_id);
	if (ret) {
		printf("[lp_rpmsg]: rpmsg_free_ept for /dev/rpmsg%d (%s) failed\n", ept_id, ctrl_name);
		ret = -1;
		goto out;
	}
	ret = 0;
	printf("[lp_rpmsg]: Delete rpmsg endpoint file: /dev/rpmsg%d\n", ept_id);
out:
	return ret;
}

int tkl_wifi_rpmsg_send(void *args, const int data_len)
{
	int ret = -1;
	char ept_file_path[64] = { 0 };

	snprintf(ept_file_path, sizeof(ept_file_path),
		"/dev/rpmsg%d", eptdev->ept_id);
again:
	eptdev->ept_fd = open(ept_file_path, O_RDWR);
	if (eptdev->ept_fd < 0) {
		printf("[lp_rpmsg]: open %s failed, try again now\r\n", ept_file_path);
		usleep(30 * 1000);
		goto again;
	}

	ret = write(eptdev->ept_fd, args, data_len);
	if (ret < 0) {
		printf("[lp_rpmsg]: %s:%d write file error=%s\n", __FUNCTION__, __LINE__, strerror(errno));
		if (errno == EPIPE) {
			printf("[lp_rpmsg]: %s ept has been removed\r\n", ept_name);
		}
		return -1;
	}
	return ret;
}

int tkl_wifi_rpmsg_recv(void *recv_buf, int data_len)
{
	int ret = -1;
	char ept_file_path[64] = { 0 };

	snprintf(ept_file_path, sizeof(ept_file_path),
		"/dev/rpmsg%d", eptdev->ept_id);
again:
	eptdev->ept_fd = open(ept_file_path, O_RDWR);
	if (eptdev->ept_fd < 0) {
		printf("[lp_rpmsg]: open %s failed\r\n", ept_file_path);
		usleep(30 * 1000);
		goto again;
	}

	ret = read(eptdev->ept_fd, recv_buf, data_len);
	if (ret < 0) {
		printf("[lp_rpmsg]: %s:%d read file error=%s\n", __FUNCTION__, __LINE__, strerror(errno));
		if (errno == EPIPE) {
			printf("[lp_rpmsg]: %s ept has been removed\r\n", ept_name);
		}
		return -1;
	}
	return ret;
}

int tkl_wifi_rpmsg_deinit(void)
{
	if (eptdev == NULL) {
		printf("[lp_rpmsg]:%s eptdev is NULL\n", __FUNCTION__);
		return 0;
	}
	close(eptdev->ept_fd);
	int ret = -1;
	ret = rpmsg_free_ept(ctrl_name, eptdev->ept_id);
	if (ret) {
		printf("[lp_rpmsg]: rpmsg_free_ept for /dev/rpmsg%d (%s) failed\n", eptdev->ept_id, ctrl_name);
		ret = -1;
	}
	free(eptdev);
	printf("[lp_rpmsg]: Delete rpmsg endpoint file: /dev/rpmsg%d\n", eptdev->ept_id);
	return ret;
}

//rpbuf

/* tkl_wifi_rpbuf_send */
int tkl_wifi_rpbuf_send(const char *rpbuf_name, const int rpbuf_len, void *args, const int data_len)
{
	int ret = -1;
	int ctrl_id = 0;  /* default id 0 */
	void *buf_addr = NULL;
	rpbuf_buffer_t *buffer = NULL;

	buffer = rpbuf_alloc_buffer(ctrl_id, rpbuf_name, rpbuf_len);
	if (!buffer) {
		printf("rpbuf_alloc_buffer for buffer %s (len: %zd) failed\n", rpbuf_name, rpbuf_len);
		ret = -1;
	}
	buf_addr = rpbuf_buffer_addr(buffer);
	rpbuf_set_sync_buffer(buffer, 1);
	memcpy(buf_addr, args, data_len);
	/* printf("buffer->buf_fd = %d\n", buffer->buf_fd); */
	ret = rpbuf_transmit_buffer(buffer, 0, data_len);
	if (ret < 0) {
		printf("transmit buffer %s error\n", rpbuf_name);
		goto free_buffer;
	}

free_buffer:
	rpbuf_free_buffer(buffer);
	return ret;
}

/* tkl_wifi_rpbuf_recv */
int tkl_wifi_rpbuf_recv(const char *rpbuf_name, const int rpbuf_len, void *recv_buf, int *data_len)
{
	int ret = -1;
	unsigned int offset = 0;
	int ctrl_id = 0;  /* default id 0 */
	void *buf_addr = NULL;
	rpbuf_buffer_t *buffer = NULL;

	buffer = rpbuf_alloc_buffer(ctrl_id, rpbuf_name, rpbuf_len);
	if (!buffer) {
		printf("rpbuf_alloc_buffer for buffer %s (len: %zd) failed\n", rpbuf_name, rpbuf_len);
		ret = -1;
	}
	buf_addr = rpbuf_buffer_addr(buffer);
	/* printf("buffer->buf_fd = %d\n", buffer->buf_fd); */
	ret = rpbuf_receive_buffer(buffer, &offset, data_len, -1);
	if (ret < 0) {
		printf("receive buffer error %s, ret=%d\n", buffer->info.name, ret);
		goto free_buffer;
	}
	memcpy(recv_buf, buf_addr + offset, *data_len);
	for (int i = 0; i < *data_len; ++i) {
		printf(" 0x%x", ((char *)recv_buf)[i]);
	}
	sleep(2);
free_buffer:
	rpbuf_free_buffer(buffer);
	return ret;
}


