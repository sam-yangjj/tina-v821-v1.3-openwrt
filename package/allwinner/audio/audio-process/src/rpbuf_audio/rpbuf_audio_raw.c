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
#include <alsa/asoundlib.h>

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdbool.h>

#include "rpbuf_audio_ctrl.h"
#include "rpbuf_audio_ctrl_raw.h"

#include "local_rpbuf_audio.h"
#include "rpbuf_audio_raw.h"

static bool g_rpbuf_init_raw_done[RPBUF_AUDIO_CRTL_RAW_COUNTS] = {0};
static rpbuf_audio_raw_ctx_t *g_rpbuf_raw_hal[RPBUF_AUDIO_CRTL_RAW_COUNTS] = {NULL};

static pthread_mutex_t g_rpbuf_audio_raw_init_mutex = PTHREAD_MUTEX_INITIALIZER;

static LIST_HEAD(g_rpbuf_audio_raw_entry);

int rpbuf_audio_raw_is_available(rpbuf_audio_raw_ctx_t* ctx, const char* name)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	rpbuf_buffer_entry_t *pos, *tmp;
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	const char *rpbuf_audio_name = NULL;

	pthread_mutex_lock(&rpbuf_hal->rpbuf_mutex);

	list_for_each_entry_safe(pos, tmp, &g_rpbuf_audio_raw_entry, list) {
		rpbuf_audio_name = rpbuf_buffer_name(pos->buffer);
		if (!strncmp(name, rpbuf_audio_name, strlen(rpbuf_audio_name))) {
			rpbuf_buffer = pos->buffer;
			break;
		}
	}
	pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);

	if (rpbuf_buffer == NULL) {
		printf("rpbuf_buffer is not found\n");
		return ret;
	}

	ret = rpbuf_buffer_is_available(rpbuf_buffer);

	return ret;
}

static rpbuf_buffer_entry_t *rpbuf_audio_raw_create(void* ctx, int ctrl_id, const char *name, int sync, size_t len)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;

	rpbuf_buffer_entry_t *rpbuf_entry = NULL;

	rpbuf_entry = malloc(sizeof(*rpbuf_entry));
	if (!rpbuf_entry) {
		printf("failed to alloc rpbuf audio entry\r\n");
		return NULL;
	}

	memset(rpbuf_entry, 0, sizeof(*rpbuf_entry));

	rpbuf_entry->buffer = rpbuf_alloc_buffer(ctrl_id, name, len);
	if (rpbuf_entry->buffer == NULL) {
		printf("rpbuf_alloc_buffer for name %s (len: %ld) failed\n", name, len);
		goto free_rpbuf_entry;
	}

	if (sync)
		rpbuf_set_sync_buffer(rpbuf_entry->buffer, 1);

	pthread_mutex_lock(&rpbuf_hal->rpbuf_mutex);
	list_add(&rpbuf_entry->list, &g_rpbuf_audio_raw_entry);
	pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);

	return rpbuf_entry;

free_rpbuf_entry:
	free(rpbuf_entry);
	return NULL;

}

static void rpbuf_audio_raw_destroy(void* ctx, const char *name)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	rpbuf_buffer_entry_t *rpbuf_entry = NULL;
	rpbuf_buffer_entry_t *pos, *tmp;
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	const char *rpbuf_audio_name = NULL;

	pthread_mutex_lock(&rpbuf_hal->rpbuf_mutex);

	list_for_each_entry_safe(pos, tmp, &g_rpbuf_audio_raw_entry, list) {
		rpbuf_audio_name = rpbuf_buffer_name(pos->buffer);
		if (!strncmp(name, rpbuf_audio_name, strlen(rpbuf_audio_name))) {
			rpbuf_entry = pos;
			rpbuf_buffer = pos->buffer;
			break;
		}
	}

	if (rpbuf_buffer == NULL || rpbuf_entry == NULL) {
		printf("rpbuf_buffer or rpbuf_entry is not found\n");
		pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);
		return;
	}


	/* double check */
	if (rpbuf_buffer != rpbuf_entry->buffer) {
		printf("rpbuf_buffer %p is not match %p\n", rpbuf_buffer, rpbuf_entry->buffer);
		pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);
		return;
	}

	list_del(&rpbuf_entry->list);

	printf("rpbuf audio free %s start\n",rpbuf_buffer_name(rpbuf_buffer));

	rpbuf_free_buffer(rpbuf_buffer);

	free(rpbuf_entry);

	rpbuf_buffer = NULL;

	rpbuf_entry = NULL;

	pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);

	return;
}

static void rpbuf_audio_raw_ctrl_deinit(void *handle)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)handle;
	int index = rpbuf_hal->rpbuf_raw_index;

	if (rpbuf_hal->rpbuf_ctl[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_ctrl_name[index]);
		memset(rpbuf_hal->rpbuf_raw_ctrl_name[index], 0, sizeof(rpbuf_hal->rpbuf_raw_ctrl_name[index]));
	}


	if (rpbuf_hal->rpbuf_ctl_ack[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_ctrl_ack_name[index]);
		memset(rpbuf_hal->rpbuf_raw_ctrl_ack_name[index], 0, sizeof(rpbuf_hal->rpbuf_raw_ctrl_ack_name[index]));
	}
}

static int rpbuf_audio_raw_ctrl_init(void *handle)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)handle;
	int ret = -1;
	int index = rpbuf_hal->rpbuf_raw_index;

	snprintf(rpbuf_hal->rpbuf_raw_ctrl_name[index], sizeof(rpbuf_hal->rpbuf_raw_ctrl_name[index]), "%s_%d", RPBUF_CTL_RAW_NAME, index);

	if (sizeof(rpbuf_audio_remote_ctrl_raw_t) > RPBUF_CTRL_RAW_LENGTH) {
		printf("rpbuf_alloc_buffer for name %s (len: %ld) is big than %d\n", \
			rpbuf_hal->rpbuf_raw_ctrl_name[index], sizeof(rpbuf_audio_remote_ctrl_raw_t), RPBUF_CTRL_RAW_LENGTH);
		ret = -1;
		goto init_err_out;
	}

	printf("rpbuf audio ctrl name %s, len %ld\n", rpbuf_hal->rpbuf_raw_ctrl_name[index], sizeof(rpbuf_audio_remote_ctrl_raw_t));

	/* dsp recv, A7 send ctrl cmd.
	 * if remote core don't create the same name rpbuf, it will block.
	 */
	rpbuf_hal->rpbuf_ctl[index] = rpbuf_audio_raw_create(rpbuf_hal, RPBUF_CTRL_ID, rpbuf_hal->rpbuf_raw_ctrl_name[index], 1, RPBUF_CTRL_RAW_LENGTH);
	if (rpbuf_hal->rpbuf_ctl[index] == NULL) {
		printf("rpbuf_alloc_buffer for name %s (len: %ld) failed\n", rpbuf_hal->rpbuf_raw_ctrl_name[index], sizeof(rpbuf_audio_remote_ctrl_raw_t));
		ret = -1;
		goto init_err_out;
	}

	while (!rpbuf_buffer_is_available(rpbuf_hal->rpbuf_ctl[index]->buffer)) {
		usleep(5000);
	}

	snprintf(rpbuf_hal->rpbuf_raw_ctrl_ack_name[index], sizeof(rpbuf_hal->rpbuf_raw_ctrl_ack_name[index]), "%s_%d", RPBUF_CTL_RAW_ACK_NAME, index);

	printf("rpbuf audio ctrl ack name %s, len %ld\n", rpbuf_hal->rpbuf_raw_ctrl_ack_name[index], sizeof(rpbuf_audio_remote_ctrl_raw_t));

	/* dsp recv, A7 send ctrl cmd.
	 * if remote core don't create the same name rpbuf, it will block.
	 */
	rpbuf_hal->rpbuf_ctl_ack[index] = rpbuf_audio_raw_create(rpbuf_hal, RPBUF_CTRL_ID, rpbuf_hal->rpbuf_raw_ctrl_ack_name[index], 0, RPBUF_CTRL_RAW_LENGTH);
	if (rpbuf_hal->rpbuf_ctl_ack[index] == NULL) {
		printf("rpbuf_alloc_buffer for name %s (len: %ld) failed\n", rpbuf_hal->rpbuf_raw_ctrl_ack_name[index], sizeof(rpbuf_audio_remote_ctrl_raw_t));
		ret = -1;
		goto init_err_out;
	}

	while (!rpbuf_buffer_is_available(rpbuf_hal->rpbuf_ctl_ack[index]->buffer)) {
		usleep(5000);
	}

	return 0;

init_err_out:
	if (rpbuf_hal->rpbuf_ctl[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_ctrl_name[index]);
	}
	if (rpbuf_hal->rpbuf_ctl_ack[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_ctrl_ack_name[index]);
	}
	return ret;
}


int rpbuf_audio_raw_ctrl_send_cmd(rpbuf_audio_raw_ctx_t *ctx, rpbuf_audio_remote_ctrl_raw_t *rpbuf_ctl_arg, unsigned long data_offset)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hdl = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	void *buf_addr = NULL;
	size_t len = 0;
	size_t size = sizeof(rpbuf_audio_remote_ctrl_raw_t);
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	int index = 0;

	if (!rpbuf_hdl) {
		printf("rpbuf_hdl is null!\n");
		return -1;
	}

	pthread_mutex_lock(&rpbuf_hdl->rpbuf_send_mutex);

	index = rpbuf_hdl->rpbuf_raw_index;

	rpbuf_buffer = rpbuf_hdl->rpbuf_ctl[index]->buffer;

	len = rpbuf_buffer_len(rpbuf_buffer);
	if (len < size) {
		printf("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(rpbuf_buffer), len, size);
		ret = 0;
		pthread_mutex_unlock(&rpbuf_hdl->rpbuf_send_mutex);
		goto err_out;
	}

	buf_addr = rpbuf_buffer_addr(rpbuf_buffer);

	memcpy(buf_addr + data_offset, rpbuf_ctl_arg, size);

	ret = rpbuf_transmit_buffer(rpbuf_buffer, data_offset, size);
	if (ret < 0) {
		printf("transmit buffer %s error\n", rpbuf_buffer_name(rpbuf_buffer));
		pthread_mutex_unlock(&rpbuf_hdl->rpbuf_send_mutex);
		goto err_out;
	}

	pthread_mutex_unlock(&rpbuf_hdl->rpbuf_send_mutex);

	return 0;

err_out:
	return ret;
}


int rpbuf_audio_raw_ctrl_recv_cmd(rpbuf_audio_raw_ctx_t *ctx, void *data, unsigned long *data_offset,
                                 unsigned long *size, int recv_time)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hdl = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	void *buf_addr = NULL;
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	size_t len = 0;
	int index = 0;

	if (!rpbuf_hdl) {
		printf("rpbuf_hdl is null!\n");
		return -1;
	}

	pthread_mutex_lock(&rpbuf_hdl->rpbuf_recv_mutex);

	index = rpbuf_hdl->rpbuf_raw_index;

	rpbuf_buffer = rpbuf_hdl->rpbuf_ctl_ack[index]->buffer;

	buf_addr = rpbuf_buffer_addr(rpbuf_buffer);

	ret = rpbuf_receive_buffer(rpbuf_buffer, (unsigned int *)data_offset, (unsigned int *)size, recv_time);
	if (ret < 0) {
		if (ret != -ETIMEDOUT)
			printf("receive buffer %s error, ret=%d\n", rpbuf_buffer_name(rpbuf_buffer), ret);
		else
			printf("receive buffer %s timeout\n", rpbuf_buffer_name(rpbuf_buffer));
		pthread_mutex_unlock(&rpbuf_hdl->rpbuf_recv_mutex);
		goto err_out;
	}

	len = rpbuf_buffer_len(rpbuf_buffer);
	if (len < *size) {
		printf("buffer %s len %lu is not equal to input size %lu\n", rpbuf_buffer_name(rpbuf_buffer), len, *size);
		pthread_mutex_unlock(&rpbuf_hdl->rpbuf_recv_mutex);
		goto err_out;
	}

	memcpy(data, buf_addr + *data_offset, *size);

	pthread_mutex_unlock(&rpbuf_hdl->rpbuf_recv_mutex);

	return 0;

err_out:
	return ret;
}

int rpbuf_audio_raw_transfer_data(rpbuf_audio_raw_ctx_t *ctx, const char* name, void *data,
                                 unsigned long data_offset,unsigned long size)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	void *buf_addr = NULL;
	size_t len = 0;
	rpbuf_buffer_entry_t *pos, *tmp;
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	const char *rpbuf_audio_name = NULL;

	pthread_mutex_lock(&rpbuf_hal->rpbuf_mutex);
	list_for_each_entry_safe(pos, tmp, &g_rpbuf_audio_raw_entry, list) {
		rpbuf_audio_name = rpbuf_buffer_name(pos->buffer);
		if (!strncmp(name, rpbuf_audio_name, strlen(rpbuf_audio_name))) {
			rpbuf_buffer = pos->buffer;
			break;
		}
	}
	pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);

	if (rpbuf_buffer == NULL) {
		printf("rpbuf_buffer is not found\n");
		return ret;
	}

	len = rpbuf_buffer_len(rpbuf_buffer);
	if (len < size) {
		printf("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(rpbuf_buffer), len, size);
		ret = 0;
		goto err_out;
	}

	buf_addr = rpbuf_buffer_addr(rpbuf_buffer);

    memcpy(buf_addr + data_offset, data, size);

	ret = rpbuf_transmit_buffer(rpbuf_buffer, data_offset, size);
	if (ret < 0) {
		printf("transmit buffer %s error\n", rpbuf_buffer_name(rpbuf_buffer));
		goto err_out;
	}

	return 0;

err_out:
	return ret;
}

int rpbuf_audio_raw_recv_data(rpbuf_audio_raw_ctx_t *ctx, const char* name, void *data, unsigned long *data_offset,
                              unsigned long *size, int recv_time)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	void *buf_addr = NULL;
	size_t len = 0;
	rpbuf_buffer_entry_t *pos, *tmp;
	rpbuf_buffer_t *rpbuf_buffer = NULL;
	const char *rpbuf_audio_name = NULL;

	pthread_mutex_lock(&rpbuf_hal->rpbuf_mutex);

	list_for_each_entry_safe(pos, tmp, &g_rpbuf_audio_raw_entry, list) {
		rpbuf_audio_name = rpbuf_buffer_name(pos->buffer);
		if (!strncmp(name, rpbuf_audio_name, strlen(rpbuf_audio_name))) {
			rpbuf_buffer = pos->buffer;
			break;
		}
	}
	pthread_mutex_unlock(&rpbuf_hal->rpbuf_mutex);

	if (rpbuf_buffer == NULL) {
		printf("rpbuf_buffer is not found\n");
		return ret;
	}

	buf_addr = rpbuf_buffer_addr(rpbuf_buffer);

	ret = rpbuf_receive_buffer(rpbuf_buffer, (unsigned int *)data_offset, (unsigned int *)size, recv_time);
	if (ret < 0) {
		if (ret != -ETIMEDOUT)
			printf("receive buffer %s error, ret=%d\n", rpbuf_buffer_name(rpbuf_buffer), ret);
		else
			printf("receive buffer %s timeout\n", rpbuf_buffer_name(rpbuf_buffer));
		goto err_out;
	}

	len = rpbuf_buffer_len(rpbuf_buffer);
	if (len < *size) {
		printf("buffer %s len %lu is not equal to input size %lu\n", rpbuf_buffer_name(rpbuf_buffer), len, *size);
		goto err_out;
	}

	memcpy(data, buf_addr + *data_offset, *size);

	return 0;
err_out:

	return ret;
}


void rpbuf_audio_raw_record_data_deinit(rpbuf_audio_raw_ctx_t *ctx)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int index = rpbuf_hal->rpbuf_raw_index;

	if (rpbuf_hal->rpbuf_raw_out[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_out_name[index]);
		rpbuf_hal->rpbuf_raw_out[index] = NULL;
		memset(rpbuf_hal->rpbuf_raw_out_name[index], 0, sizeof(rpbuf_hal->rpbuf_raw_out_name[index]));
	}

}

int rpbuf_audio_raw_record_data_init(rpbuf_audio_raw_ctx_t *ctx, unsigned int arecord_rpbuf_len)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	int index = rpbuf_hal->rpbuf_raw_index;

	rpbuf_hal->rpbuf_raw_out_len = arecord_rpbuf_len;

	printf("rpbuf audio name %s, rpbuf_arecord_len %u \n", RPBUF_RAW_OUT_NAME, rpbuf_hal->rpbuf_raw_out_len);

	if (rpbuf_hal->rpbuf_raw_out_len == 0) {
		printf("rpbuf_arecord_len %d is invalid\n", rpbuf_hal->rpbuf_raw_out_len);
		ret = -1;
		return ret;
	}

	snprintf(rpbuf_hal->rpbuf_raw_out_name[index], sizeof(rpbuf_hal->rpbuf_raw_out_name[index]), "%s_%d", RPBUF_RAW_OUT_NAME, index);

	/*  DSP to A7 data
	 * if remote core don't create the same name rpbuf, it will block.
	 */
	rpbuf_hal->rpbuf_raw_out[index] = rpbuf_audio_raw_create(ctx, RPBUF_CTRL_ID, rpbuf_hal->rpbuf_raw_out_name[index], 1, rpbuf_hal->rpbuf_raw_out_len);
	if (rpbuf_hal->rpbuf_raw_out[index] == NULL) {
		printf("rpbuf_alloc_buffer for name %s (len: %d) failed\n", rpbuf_hal->rpbuf_raw_out_name[index], rpbuf_hal->rpbuf_raw_out_len);
		ret = -1;
		goto init_err_out;
	}

	while (!rpbuf_buffer_is_available(rpbuf_hal->rpbuf_raw_out[index]->buffer)) {
		usleep(5000);
	}

	printf("rpbuf %s ready\n", rpbuf_hal->rpbuf_raw_out_name[index]);


	return 0;

init_err_out:
	if (rpbuf_hal->rpbuf_raw_out[index]) {
		rpbuf_audio_raw_destroy(rpbuf_hal, rpbuf_hal->rpbuf_raw_out_name[index]);
	}

	return ret;
}

int rpbuf_audio_raw_get_data_name(rpbuf_audio_raw_ctx_t *ctx, char *raw_in, char *raw_out)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int ret = -1;
	int index = rpbuf_hal->rpbuf_raw_index;

	if (rpbuf_hal == NULL)
		return -1;

	if (raw_in)
		strncpy(raw_in, rpbuf_hal->rpbuf_raw_in_name[index], sizeof(rpbuf_hal->rpbuf_raw_in_name[index]));

	if (raw_out)
		strncpy(raw_out, rpbuf_hal->rpbuf_raw_out_name[index], sizeof(rpbuf_hal->rpbuf_raw_out_name[index]));

	return 0;
}

rpbuf_audio_raw_ctx_t* rpbuf_audio_raw_init(int index)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = NULL;
	int ret = -1;

	if (index > RPBUF_AUDIO_CRTL_RAW_COUNTS - 1) {
		printf("err! rpbuf_audio_raw_init index %d is large than  %d \n", index, RPBUF_AUDIO_CRTL_RAW_COUNTS -1);
		return NULL;
	}

	pthread_mutex_lock(&g_rpbuf_audio_raw_init_mutex);

	if (!g_rpbuf_init_raw_done[index]) {
		rpbuf_hal = malloc(sizeof(rpbuf_audio_raw_ctx_t));
		if (rpbuf_hal == NULL) {
			printf("create rpbuf_audio_raw_ctx_t err\n");
			pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);
			goto error;
		}
		memset(rpbuf_hal, 0 , sizeof(rpbuf_audio_raw_ctx_t));

		rpbuf_hal->rpbuf_raw_index = index;

		ret = pthread_mutex_init(&rpbuf_hal->rpbuf_mutex, NULL);
		if (ret < 0) {
			printf("pthread_mutex_init error");
			pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);
			goto error;
		}

		ret = pthread_mutex_init(&rpbuf_hal->rpbuf_send_mutex, NULL);
		if (ret < 0) {
			printf("pthread_mutex_init send_mutex error");
			pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);
			goto error;
		}

		ret = pthread_mutex_init(&rpbuf_hal->rpbuf_recv_mutex, NULL);
		if (ret < 0) {
			printf("pthread_mutex_init recv_mutex error");
			pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);
			goto error;
		}

		ret = rpbuf_audio_raw_ctrl_init(rpbuf_hal);
		if (ret < 0) {
			printf("rpbuf_audio_raw_ctrl_init failed\n");
			pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);
			goto error;
		}

		g_rpbuf_init_raw_done[index] = true;
		g_rpbuf_raw_hal[index] = rpbuf_hal;

	} else {
		if (g_rpbuf_raw_hal[index])
			rpbuf_hal = g_rpbuf_raw_hal[index];
	}
	pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);

	return rpbuf_hal;

error:

	if (rpbuf_hal)
		rpbuf_audio_raw_ctrl_deinit(rpbuf_hal);

	if (rpbuf_hal)
		free(rpbuf_hal);

	return NULL;

}

void rpbuf_audio_raw_deinit(rpbuf_audio_raw_ctx_t *ctx)
{
	rpbuf_audio_raw_ctx_t *rpbuf_hal = (rpbuf_audio_raw_ctx_t *)ctx;
	int index = rpbuf_hal->rpbuf_raw_index;

	if (!rpbuf_hal)
		return;

	pthread_mutex_lock(&g_rpbuf_audio_raw_init_mutex);

	if (g_rpbuf_raw_hal[index]) {

		rpbuf_audio_raw_ctrl_deinit(rpbuf_hal);

		rpbuf_audio_raw_record_data_deinit(rpbuf_hal);

		pthread_mutex_destroy(&rpbuf_hal->rpbuf_mutex);

		pthread_mutex_destroy(&rpbuf_hal->rpbuf_send_mutex);

		pthread_mutex_destroy(&rpbuf_hal->rpbuf_recv_mutex);

		free(rpbuf_hal);

		rpbuf_hal = NULL;

		g_rpbuf_init_raw_done[index] = false;
	}

	g_rpbuf_raw_hal[index] = NULL;

	pthread_mutex_unlock(&g_rpbuf_audio_raw_init_mutex);

	return;
}

