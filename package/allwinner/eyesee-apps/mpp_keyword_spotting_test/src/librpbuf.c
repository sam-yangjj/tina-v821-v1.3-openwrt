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

#include "librpbuf.h"

#include "local.h"

#define MAX_WAIT_TRY_TIMES 1000
static int wait_buf_dev_file_ready(const char *file_path)
{
	int ret, try_cnt = 0;

	while (1) {
		ret = access(file_path, F_OK);
		printf("##ret = %d\n", ret);
		if (!ret)
			return 0;

		try_cnt++;
		if (try_cnt >= MAX_WAIT_TRY_TIMES)
			break;

		usleep(1000);
	}

	fprintf(stderr, "wait rpbuf buf dev file '%s' ready timeout, error: '%s'\n", file_path, strerror(errno));
	return -1;
}

rpbuf_buffer_t *rpbuf_alloc_buffer(int ctrl_id, const char *name, size_t len)
{
	rpbuf_buffer_t *buffer;
	size_t name_len;
	char ctrl_dev_path[128];
	char buf_dev_path[128];
	int ctrl_fd;
	int buf_fd;
	void *addr;
	int ret, flags;

	name_len = strlen(name);
	if (name_len == 0 || name_len >= RPBUF_NAME_SIZE) {
		fprintf(stderr, "invalid buffer name \"%s\", its length should be "
				"in range [1, %d)\n", name, RPBUF_NAME_SIZE);
		goto err_out;
	}

	buffer = calloc(1, sizeof(rpbuf_buffer_t));
	if (!buffer) {
		fprintf(stderr, "%s: calloc for rpbuf_buffer_t failed\n", name);
		goto err_out;
	}
	strncpy(buffer->info.name, name, RPBUF_NAME_SIZE);
	buffer->info.len = len;

	snprintf(ctrl_dev_path, sizeof(ctrl_dev_path), "/dev/rpbuf_ctrl%d", ctrl_id);
	snprintf(buf_dev_path, sizeof(buf_dev_path), "/dev/rpbuf-%s", name);

	ctrl_fd = open(ctrl_dev_path, O_RDWR);
	if (ctrl_fd < 0) {
		fprintf(stderr, "failed to open rpbuf ctrl dev file '%s' (%s)\n", ctrl_dev_path, strerror(errno));
		goto err_free_buffer;
	}
	buffer->ctrl_fd = ctrl_fd;

	flags = fcntl(ctrl_fd, F_GETFD);
	flags |= FD_CLOEXEC;
	fcntl(ctrl_fd, F_SETFD, flags);

	ret = ioctl(ctrl_fd, RPBUF_CTRL_DEV_IOCTL_CREATE_BUF, &buffer->info);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: ioctl CREATE_BUF failed (%s)\n",
				name, strerror(errno));
		goto err_close_ctl_fd;
	}

	wait_buf_dev_file_ready(buf_dev_path);

	buf_fd = open(buf_dev_path, O_RDWR);
	if (buf_fd < 0) {
		fprintf(stderr, "failed to open rpbuf buf dev file '%s' (%s)\n", buf_dev_path, strerror(errno));
		goto err_ioctl_destroy_buf;
	}
	buffer->buf_fd = buf_fd;

	flags = fcntl(buf_fd, F_GETFD);
	flags |= FD_CLOEXEC;
	fcntl(buf_fd, F_SETFD, flags);

	addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr, "buffer %s: mmap failed (%s)\n", name, strerror(errno));
		goto err_close_buf_fd;
	}
	buffer->addr = addr;

	return buffer;

err_close_buf_fd:
	close(buf_fd);
err_ioctl_destroy_buf:
	ioctl(ctrl_fd, RPBUF_CTRL_DEV_IOCTL_DESTROY_BUF, &buffer->info);
err_close_ctl_fd:
	close(ctrl_fd);
err_free_buffer:
	free(buffer);
err_out:
	return NULL;
}

int rpbuf_free_buffer(rpbuf_buffer_t *buffer)
{
	int ret;

	munmap(buffer->addr, buffer->info.len);

	close(buffer->buf_fd);

	ret = ioctl(buffer->ctrl_fd, RPBUF_CTRL_DEV_IOCTL_DESTROY_BUF, &buffer->info);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: ioctl DESTROY_BUF failed (%s)\n",
				buffer->info.name, strerror(errno));
		/* Go on releasing other resources rather than return immediately. */
	}

	close(buffer->ctrl_fd);

	free(buffer);

	return 0;
}

void *rpbuf_buffer_addr(rpbuf_buffer_t *buffer)
{
	return buffer->addr;
}

const char *rpbuf_buffer_name(rpbuf_buffer_t *buffer)
{
	return buffer->info.name;
}

size_t rpbuf_buffer_len(rpbuf_buffer_t *buffer)
{
	return buffer->info.len;
}

int rpbuf_buffer_is_available(rpbuf_buffer_t *buffer)
{
	int ret;
	uint8_t is_available;

	ret = ioctl(buffer->buf_fd, RPBUF_BUF_DEV_IOCTL_CHECK_AVAIL, &is_available);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: ioctl CHECK_AVAIL failed (%s)\n",
				buffer->info.name, strerror(errno));
		/* Return 0 means not available */
		return 0;
	}

	return is_available;
}

int rpbuf_transmit_buffer(rpbuf_buffer_t *buffer, uint32_t offset, uint32_t data_len)
{
	struct rpbuf_buffer_xfer xfer;
	int ret;

	xfer.offset = offset;
	xfer.data_len = data_len;

	ret = ioctl(buffer->buf_fd, RPBUF_BUF_DEV_IOCTL_TRANSMIT_BUF, &xfer);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: ioctl TRANSMIT_BUF failed (%s)\n",
				buffer->info.name, strerror(errno));
		return ret;
	}

	return 0;
}

int rpbuf_receive_buffer(rpbuf_buffer_t *buffer, uint32_t *offset,
		uint32_t *data_len, int timeout_ms)
{
	struct rpbuf_buffer_xfer xfer;
	int ret;

	xfer.timeout_ms = timeout_ms;

	ret = ioctl(buffer->buf_fd, RPBUF_BUF_DEV_IOCTL_RECEIVE_BUF, &xfer);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: ioctl RECEIVE_BUF failed (%s)\n",
				buffer->info.name, strerror(errno));
		return ret;
	}

	*offset = xfer.offset;
	*data_len = xfer.data_len;

	return timeout_ms < 0 ? 0 : xfer.timeout_ms;
}

int rpbuf_install_rx_signal(rpbuf_buffer_t *buffer, sighandler_t handler)
{
	int flags, ret;
	sighandler_t old;

	ret = fcntl(buffer->buf_fd, F_SETOWN, getpid());
	if (ret < 0) {
		fprintf(stderr, "buffer %s: fcntl F_SETOWN failed (%s)\n",
				buffer->info.name, strerror(errno));
		return ret;
	}

	flags = fcntl(buffer->buf_fd, F_GETFL);
	if (flags < 0) {
		fprintf(stderr, "buffer %s: fcntl F_GETFL failed (%s)\n",
				buffer->info.name, strerror(errno));
		return errno;
	}

	ret = fcntl(buffer->buf_fd, F_SETFL, flags | FASYNC);
	if (ret < 0) {
		fprintf(stderr, "buffer %s: fcntl F_SETFL failed (%s)\n",
				buffer->info.name, strerror(errno));
		return ret;
	}

	old = signal(SIGIO, handler);
	if (old == SIG_ERR) {
		fprintf(stderr, "buffer %s: signal failed (%s)\n",
				buffer->info.name, strerror(errno));
		return errno;
	}

	return 0;
}

int rpbuf_remove_rx_signal(rpbuf_buffer_t *buffer)
{
	sighandler_t ret;

	ret = signal(SIGIO, SIG_DFL);
	if (ret == SIG_ERR) {
		fprintf(stderr, "buffer %s: signal failed (%s)\n",
				buffer->info.name, strerror(errno));
		return errno;
	}

	return 0;
}

int rpbuf_set_sync_buffer(rpbuf_buffer_t *buffer, uint32_t is_sync)
{
	int ret = 0;

	ret = ioctl(buffer->buf_fd, RPBUF_BUF_DEV_IOCTL_SET_SYNC_BUF, &is_sync);
	if (ret < 0)
		fprintf(stderr, "buffer %s: ioctl SET_SYNC_BUF failed (%s)\n",
				buffer->info.name, strerror(errno));

	return ret;
}

int rpbuf_get_perf_data(rpbuf_buffer_t *buffer, rpbuf_perf_data_t *perf_data)
{
	int ret = 0;
	struct rpbuf_perf_data_ioctl perf_data_ioctl;

	if (!perf_data)
		return -EINVAL;

	perf_data_ioctl.len = sizeof(rpbuf_perf_data_t);
	perf_data_ioctl.data = perf_data;

	ret = ioctl(buffer->buf_fd, RPBUF_BUF_DEV_IOCTL_GET_PERF_DATA, &perf_data_ioctl);
	if (ret < 0)
		fprintf(stderr, "buffer %s: ioctl GET_PERF_DATA failed (%s)\n",
				buffer->info.name, strerror(errno));

	return ret;
}

void rpbuf_dump_perf_data(const rpbuf_perf_data_t *perf_data)
{
	uint64_t raw_ts;
	uint32_t ts, time_span;

	raw_ts = perf_data->raw_timestamp;
	printf("Mode: %s\n", perf_data->is_sync? "Sync" : "Async");
	printf("Timestamp:\n");
	printf(">Start       : 0 us [%"PRIu64".%"PRIu64" ms]\n", raw_ts / 1000, raw_ts % 1000);

	ts = perf_data->timestamp[RPBUF_PERF_POINT_LOCAL_FLUSH];
	printf(">Flush cache : %u us\n", ts);

	ts = perf_data->timestamp[RPBUF_PERF_POINT_LOCAL_NOTIFY];
	printf(">Notify      : %u us\n", ts);

	ts = perf_data->timestamp[RPBUF_PERF_POINT_LOCAL_WAIT_ACK];
	printf(">Wait ACK    : %u us\n", ts);

	if (perf_data->is_sync) {
		ts = perf_data->timestamp[RPBUF_PERF_POINT_REMOTE_RECV];
		printf("  <Recv      : %u us\n", ts);

		ts = perf_data->timestamp[RPBUF_PERF_POINT_REMOTE_EXEC_CB];
		printf("  <Exec CB   : %u us\n", ts);

		ts = perf_data->timestamp[RPBUF_PERF_POINT_REMOTE_SEND_ACK];
		printf("  <Send ack  : %u us\n", ts);

		ts = perf_data->timestamp[RPBUF_PERF_POINT_REMOTE_FINISH];
		printf("  <Finish    : %u us\n", ts);
	}
	ts = perf_data->timestamp[RPBUF_PERF_POINT_LOCAL_FINISH];
	printf(">End         : %u us\n",  ts);

	printf("Time span:\n");

	time_span = perf_data->time_span[RPBUF_PERF_STAGE_LOCAL_PREPARE];
	printf(">Prepare     : %u us\n", time_span);

	time_span = perf_data->time_span[RPBUF_PERF_STAGE_LOCAL_FLUSH];
	printf(">Flush cache : %u us\n", time_span);

	time_span = perf_data->time_span[RPBUF_PERF_STAGE_LOCAL_NOTIFY];
	printf(">Notify     : %u us\n", time_span);

	if (perf_data->is_sync) {
		time_span = perf_data->time_span[RPBUF_PERF_STAGE_REMOTE_RECV];
		printf("  <Recv        : %u us\n", time_span);

		time_span = perf_data->time_span[RPBUF_PERF_STAGE_REMOTE_PREPARE];
		printf("  <Preprae     : %u us\n", time_span);

		time_span = perf_data->time_span[RPBUF_PERF_STAGE_REMOTE_EXEC_CB];
		printf("  <Exec CB     : %u us\n", time_span);

		time_span = perf_data->time_span[RPBUF_PERF_STAGE_REMOTE_SEND_ACK];
		printf("  <Send ACK    : %u us\n", time_span);
	}
	time_span = perf_data->time_span[RPBUF_PERF_STAGE_LOCAL_FINISH];
	printf(">Local finish: %u us\n",  time_span);
}
