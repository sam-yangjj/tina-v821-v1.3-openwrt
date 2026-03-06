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
#ifndef __LIBrpmsg_H__
#define __LIBrpmsg_H__

#include <sys/types.h>
#include <signal.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define RPMSG_NAME_SIZE					(32)
#define RPMSG_DATA_MAX_LEN				(512 - 16)

/**
 * rpmsg_alloc_ept - Allocate a rpmsg endpoint
 * @ctrl_name: specify which /dev/rpmsg-ctrl to use to allocate buffer
 * @name: endpoint name
 *
 * This function will block until the buffer is available , or a few seconds
 * timeout (normally 15 seconds).
 *
 * if successfully, it will return endpoint id. we can open /dev/rpmsg%d
 * to send and read data.
 *
 * Return pointer to rpmsg buffer on success, or negative number on failure.
 */
int rpmsg_alloc_ept(const char *ctrl_name, const char *name);

/**
 * rpmsg_free_buffer - Free a rpmsg endpoint
 * @ctrl_name: specify which /dev/rpmsg-ctrl to use to allocate buffer
 * @ept_id: rpmsg_alloc_ept function return value.
 *
 * Return pointer to rpmsg buffer on success, or negative number on failure.
 */
int rpmsg_free_ept(const char *ctrl_name, int ept_id);

/**
 * rpmsg_open_ctrl_dev - Open prmsg ctrl dev
 * @ctrl_name: specify which /dev/rpmsg-ctrl to use to allocate buffer
 *
 * Return rpmsg ctrl dev's file descriptor on success, or negative number on failure.
 */
int rpmsg_open_ctrl_dev(const char *ctrl_name);

/**
 * rpmsg_alloc_ept_by_name - Allocate a rpmsg endpoint by name
 * @ctrl_fd: rpmsg ctrl dev's file descriptor
 * @name: endpoint name
 * @auto_free: set non-zero value to automatically destroy endpoint
 *
 * If auto_free is non-zero, the endpoint will be automatically destroyed
 * when the file descriptor is closed.
 *
 * This function will block until the buffer is available , or a few seconds
 * timeout (normally 15 seconds).
 *
 * if successfully, it will return endpoint id. we can open /dev/rpmsg%d
 * to send and read data.
 *
 * Return endpoint id on success, or negative number on failure.
 */
int rpmsg_alloc_ept_by_name(int ctrl_fd, const char *name, int auto_free);

/**
 * rpmsg_free_ept_by_id - Free a rpmsg endpoint by id
 * @ctrl_fd: rpmsg ctrl dev's file descriptor
 * @ept_id: endpoint id.
 *
 * Return zero on success, or negative number on failure.
 */
int rpmsg_free_ept_by_id(int ctrl_fd, int ept_id);

/**
 * rpmsg_close_ctrl_dev - Close prmsg ctrl dev
 * @ctrl_fd: rpmsg ctrl dev's file descriptor
 *
 * Return zero on success, or negative number on failure.
 */
int rpmsg_close_ctrl_dev(int ctrl_fd);


#define RPMSG_PERF_POINT_SENDER_FILL 0
#define RPMSG_PERF_POINT_SENDER_NOTIFY 1
#define RPMSG_PERF_POINT_SENDER_END 2
#define RPMSG_PERF_POINT_RECEIVER_PREPARE 3
#define RPMSG_PERF_POINT_RECEIVER_EXEC_CB 4
#define RPMSG_PERF_POINT_RECEIVER_END 5

#define RPMSG_PERF_STAGE_SENDER_PREPARE 0
#define RPMSG_PERF_STAGE_SENDER_FILL 1
#define RPMSG_PERF_STAGE_SENDER_NOTIFY 2
#define RPMSG_PERF_STAGE_RECEIVER_RECV 3 /* sender notify to receiver prepare */
#define RPMSG_PERF_STAGE_RECEIVER_PREPARE 4
#define RPMSG_PERF_STAGE_RECEIVER_EXEC_CB 5

#define RPMSG_PERF_STAGE_NUM (RPMSG_PERF_STAGE_RECEIVER_EXEC_CB + 1)
#define RPMSG_PERF_POINT_NUM RPMSG_PERF_STAGE_NUM

typedef struct rpmsg_perf_data {
	uint32_t timestamp[RPMSG_PERF_POINT_NUM];
	uint32_t time_span[RPMSG_PERF_STAGE_NUM];
	uint64_t raw_timestamp;
} rpmsg_perf_data_t;

#define RPMSG_DELIVERY_PERF_DATA_MAGIC 0x46524550 /* PERF */

typedef struct rpmsg_perf_data_delivery {
	uint32_t magic;
	uint32_t data_len;
	rpmsg_perf_data_t data;
} rpmsg_perf_data_delivery_t;

#define RPMSG_DELIVERY_PERF_DATA_LEN (sizeof(rpmsg_perf_data_delivery_t))

int rpmsg_set_perf_data_delivery(int ept_dev_fd, int is_delivery);
int rpmsg_get_perf_data(const char *buf, uint32_t len, rpmsg_perf_data_t *perf_data, uint32_t *payload_offset);
void rpmsg_dump_perf_data(const rpmsg_perf_data_t *perf_data);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __LIBrpmsg_H__ */
