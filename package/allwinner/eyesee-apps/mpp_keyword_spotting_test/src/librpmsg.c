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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <inttypes.h>
#include <signal.h>

#include "librpmsg.h"
#include "rpmsg.h"

int rpmsg_alloc_ept(const char *ctrl_name, const char *name)
{
	struct rpmsg_ept_info info;
	size_t name_len;
	char ctrl_dev_path[128];
	char ept_dev_path[RPMSG_NAME_SIZE];
	int ctrl_fd;
	int ret;

	name_len = strlen(name);
	if (name_len == 0 || name_len >= RPMSG_NAME_SIZE) {
		fprintf(stderr, "invalid ept name \"%s\", its length should be "
				"in range [1, %d)\n", name, RPMSG_NAME_SIZE);
		goto err_out;
	}

	strncpy(info.name, name, RPMSG_NAME_SIZE);
	info.id = -1;

	snprintf(ctrl_dev_path, sizeof(ctrl_dev_path), "/dev/rpmsg_ctrl-%s", ctrl_name);
	snprintf(ept_dev_path, sizeof(ept_dev_path), "/dev/rpmsg-%s", name);

	ctrl_fd = open(ctrl_dev_path, O_RDWR);
	if (ctrl_fd < 0) {
		fprintf(stderr, "failed to open %s (%s)\n", ctrl_dev_path, strerror(errno));
		goto err_out;
	}

	ret = ioctl(ctrl_fd, RPMSG_CREATE_EPT_IOCTL, &info);
	if (ret < 0) {
		fprintf(stderr, "ept %s: ioctl CREATE_BUF failed (%s)\n",
				name, strerror(errno));
		goto err_close_ctl_fd;
	}
	close(ctrl_fd);

	return info.id;

err_close_ctl_fd:
	close(ctrl_fd);
err_out:
	return -ENODEV;
}

int rpmsg_free_ept(const char *ctrl_name, int ept_id)
{
	struct rpmsg_ept_info info;
	char ctrl_dev_path[128];
	int ctrl_fd;
	int ret;

	snprintf(ctrl_dev_path, sizeof(ctrl_dev_path), "/dev/rpmsg_ctrl-%s", ctrl_name);

	info.id = ept_id;

	ctrl_fd = open(ctrl_dev_path, O_RDWR);
	if (ctrl_fd < 0) {
		fprintf(stderr, "failed to open %s (%s)\n", ctrl_dev_path, strerror(errno));
		return -EINVAL;
	}

	ret = ioctl(ctrl_fd, RPMSG_DESTROY_EPT_IOCTL, &info);
	if (ret < 0) {
		fprintf(stderr, "ept %s: ioctl DESTROY_EPT failed (%s)\n",
				info.name, strerror(errno));
		return -ENODEV;
	}

	close(ctrl_fd);

	return 0;
}

int rpmsg_open_ctrl_dev(const char *ctrl_name)
{
	char ctrl_dev_path[128];
	int ctrl_fd;
	int len;

	if (!ctrl_name)
		return -EINVAL;

	len = strlen(ctrl_name);
	if (len <= 0 || len > 112)
		return -EINVAL;

	memset(ctrl_dev_path, 0, sizeof(ctrl_dev_path));
	snprintf(ctrl_dev_path, sizeof(ctrl_dev_path), "/dev/rpmsg_ctrl-%s", ctrl_name);

	ctrl_fd = open(ctrl_dev_path, O_RDWR);
	if (ctrl_fd < 0)
		return -ENODEV;

	return ctrl_fd;
}

int rpmsg_alloc_ept_by_name(int ctrl_fd, const char *name, int auto_free)
{
	char ept_dev_path[RPMSG_NAME_SIZE + 12];
	struct rpmsg_ept_info info;
	int len;
	int ret;

	if (ctrl_fd < 0 || !name)
		return -EINVAL;

	len = strlen(name);
	if (len <= 0 || len >= RPMSG_NAME_SIZE)
		return -EINVAL;

	memset(&info, 0, sizeof(info));
	memcpy(info.name, name, len);
	info.id = -1;

	snprintf(ept_dev_path, sizeof(ept_dev_path), "/dev/rpmsg-%s", name);

	if (auto_free)
		ret = ioctl(ctrl_fd, RPMSG_CREATE_AF_EPT_IOCTL, &info);
	else
		ret = ioctl(ctrl_fd, RPMSG_CREATE_EPT_IOCTL, &info);
	if (ret < 0)
		return ret;

	return info.id;
}

int rpmsg_free_ept_by_id(int ctrl_fd, int ept_id)
{
	struct rpmsg_ept_info info;
	int ret;

	if (ctrl_fd < 0 || ept_id < 0)
		return -EINVAL;

	memset(&info, 0, sizeof(info));
	info.id = ept_id;

	ret = ioctl(ctrl_fd, RPMSG_DESTROY_EPT_IOCTL, &info);
	if (ret < 0)
		return ret;

	return 0;
}

int rpmsg_close_ctrl_dev(int ctrl_fd)
{
	if (ctrl_fd < 0)
		return -EINVAL;

	close(ctrl_fd);

	return 0;
}

int rpmsg_set_perf_data_delivery(int ept_dev_fd, int is_delivery)
{
	int ret = 0;

	ret = ioctl(ept_dev_fd, RPMSG_EPTDEV_DELIVER_PERF_DATA_IOCTL, &is_delivery);
	if (ret < 0)
		fprintf(stderr, "rpmsg eptdev fd %d: ioctl DELIVER_PERF_DATA failed (%s)\n",
				ept_dev_fd, strerror(errno));

	return ret;
}

int rpmsg_get_perf_data(const char *buf, uint32_t len, rpmsg_perf_data_t *perf_data, uint32_t *payload_offset)
{
	int ret = 0;
	const rpmsg_perf_data_delivery_t *perf_data_delivery;
	uint32_t target_len;

	if (!buf || len < sizeof(rpmsg_perf_data_delivery_t) ||!perf_data)
		return -EINVAL;

	perf_data_delivery = (const rpmsg_perf_data_delivery_t *)buf;

	if (perf_data_delivery->magic != RPMSG_DELIVERY_PERF_DATA_MAGIC) {
		fprintf(stderr, "invalid perf data delivery magic(0x%08x), shoule be 0x%08x\n",
				perf_data_delivery->magic, RPMSG_DELIVERY_PERF_DATA_MAGIC);
		return -EIO;
	}

	target_len = sizeof(rpmsg_perf_data_t);
	if (perf_data_delivery->data_len != target_len) {
		fprintf(stderr, "invalid rpmsg perf data len(%u), should be %u\n", perf_data_delivery->data_len, target_len);
		return -EINVAL;
	}

	memcpy(perf_data, &perf_data_delivery->data, target_len);

	if (payload_offset)
		*payload_offset = sizeof(rpmsg_perf_data_delivery_t);

	return 0;
}

#define pr_info printf

void rpmsg_dump_perf_data(const rpmsg_perf_data_t *perf_data)
{
	uint64_t raw_ts;
	uint32_t ts, time_span;

	raw_ts = perf_data->raw_timestamp;
	pr_info("Timestamp:\n");
	pr_info("Sender  >>\n");
	pr_info("Start         : 0 us [%"PRIu64".%"PRIu64" ms]\n", raw_ts / 1000, raw_ts % 1000);

	ts = perf_data->timestamp[RPMSG_PERF_POINT_SENDER_FILL];
	pr_info("Fill buffer   : %u us\n", ts);

	ts = perf_data->timestamp[RPMSG_PERF_POINT_SENDER_NOTIFY];
	pr_info("Notify        : %u us\n", ts);

	ts = perf_data->timestamp[RPMSG_PERF_POINT_SENDER_END];
	pr_info("End           : %u us\n",  ts);

	pr_info("Receiver<<\n");
	ts = perf_data->timestamp[RPMSG_PERF_POINT_RECEIVER_PREPARE];
	pr_info("Prepare       : %u us\n", ts);

	ts = perf_data->timestamp[RPMSG_PERF_POINT_RECEIVER_EXEC_CB];
	pr_info("Exec CB       : %u us\n", ts);

	ts = perf_data->timestamp[RPMSG_PERF_POINT_RECEIVER_END];
	pr_info("End           : %u us\n", ts);


	pr_info("Time span:\n");

	pr_info("Sender  >>\n");
	time_span = perf_data->time_span[RPMSG_PERF_STAGE_SENDER_PREPARE];
	pr_info("Prepare       : %u us\n", time_span);

	time_span = perf_data->time_span[RPMSG_PERF_STAGE_SENDER_FILL];
	pr_info("Fill buffer   : %u us\n", time_span);

	time_span = perf_data->time_span[RPMSG_PERF_STAGE_SENDER_NOTIFY];
	pr_info("Notify        : %u us\n", time_span);

	pr_info("Receiver<<\n");
	time_span = perf_data->time_span[RPMSG_PERF_STAGE_RECEIVER_RECV];
	pr_info("Recv          : %u us\n", time_span);

	time_span = perf_data->time_span[RPMSG_PERF_STAGE_RECEIVER_PREPARE];
	pr_info("Prepare       : %u us\n", time_span);

	time_span = perf_data->time_span[RPMSG_PERF_STAGE_RECEIVER_EXEC_CB];
	pr_info("Exec CB       : %u us\n", time_span);
}
