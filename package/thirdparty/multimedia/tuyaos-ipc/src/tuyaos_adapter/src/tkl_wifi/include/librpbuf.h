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
#ifndef __LIBRPBUF_H__
#define __LIBRPBUF_H__

#include <sys/types.h>
#include <signal.h>
#include <linux/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define RPBUF_PERF_POINT_LOCAL_FLUSH 0
#define RPBUF_PERF_POINT_LOCAL_NOTIFY 1
#define RPBUF_PERF_POINT_LOCAL_WAIT_ACK 2
#define RPBUF_PERF_POINT_REMOTE_RECV 3
#define RPBUF_PERF_POINT_REMOTE_EXEC_CB 4
#define RPBUF_PERF_POINT_REMOTE_SEND_ACK 5
#define RPBUF_PERF_POINT_REMOTE_FINISH 6
#define RPBUF_PERF_POINT_LOCAL_FINISH 7

#define RPBUF_PERF_STAGE_LOCAL_PREPARE 0
#define RPBUF_PERF_STAGE_LOCAL_FLUSH 1
#define RPBUF_PERF_STAGE_LOCAL_NOTIFY 2
#define RPBUF_PERF_STAGE_REMOTE_RECV 3
#define RPBUF_PERF_STAGE_REMOTE_PREPARE 4
#define RPBUF_PERF_STAGE_REMOTE_EXEC_CB 5
#define RPBUF_PERF_STAGE_REMOTE_SEND_ACK 6
#define RPBUF_PERF_STAGE_LOCAL_FINISH 7

#define RPBUF_PERF_STAGE_NUM (RPBUF_PERF_STAGE_LOCAL_FINISH + 1)
#define RPBUF_PERF_POINT_NUM RPBUF_PERF_STAGE_NUM

typedef struct rpbuf_perf_data {
	int is_sync;
	__u32 timestamp[RPBUF_PERF_POINT_NUM];
	__u32 time_span[RPBUF_PERF_STAGE_NUM];
	__u64 raw_timestamp;
} rpbuf_perf_data_t;

typedef struct __rpbuf_buffer rpbuf_buffer_t;

typedef void (*sighandler_t)(int);
/**
 * rpbuf_alloc_buffer - Allocate a rpbuf buffer
 * @ctrl_id: specify which /dev/rpbuf_ctrl to use to allocate buffer
 * @name: buffer name
 * @len: buffer length
 *
 * This function will block until the buffer is available , or a few seconds
 * timeout (normally 15 seconds).
 *
 * Only both local and remote allocate this buffer (with the same name and
 * length), it is called 'available'. We should not access a buffer until it
 * becomes available.
 *
 * After this function return successfully, we can use rpbuf_buffer_addr() to
 * get the buffer start address and access it directly.
 *
 * Return pointer to rpbuf buffer on success, or NULL on failure.
 */
rpbuf_buffer_t *rpbuf_alloc_buffer(int ctrl_id, const char *name, size_t len);

/**
 * rpbuf_free_buffer - Free a rpbuf buffer
 * @buffer: rpbuf buffer
 */
int rpbuf_free_buffer(rpbuf_buffer_t *buffer);

/**
 * rpbuf_buffer_addr - Get the start address of rpbuf buffer
 * @buffer: rpbuf buffer
 *
 * Through this address, we can directly read/write data from/to this buffer.
 */
void *rpbuf_buffer_addr(rpbuf_buffer_t *buffer);

/**
 * rpbuf_buffer_name - Get the name of rpbuf buffer
 * @buffer: rpbuf buffer
 */
const char *rpbuf_buffer_name(rpbuf_buffer_t *buffer);

/**
 * rpbuf_buffer_len - Get the length of rpbuf buffer
 * @buffer: rpbuf buffer
 */
size_t rpbuf_buffer_len(rpbuf_buffer_t *buffer);

/**
 * rpbuf_buffer_is_available - check whether a rpbuf buffer is available
 * @buffer: rpbuf buffer
 *
 * 'available' means a status that both local and remote have allocated this
 * buffer. It doesn't indicate whether the buffer is being read/written by remote.
 *
 * Normally we needn't call this function, because after rpbuf_alloc_buffer()
 * returns successfully, the buffer is already available. But in case remote
 * frees the buffer midway, we can call this function to check buffer every time
 * before we read/write from/to it.
 *
 * Return 1 when available, 0 when not available.
 */
int rpbuf_buffer_is_available(rpbuf_buffer_t *buffer);

/**
 * rpbuf_transmit_buffer - Transmit rpbuf buffer to remote
 * @buffer: rpbuf buffer
 * @offset: the address offset to store data
 * @data_len: the valid length of data
 *
 * This function won't access the actual data in buffer, it only transmits the
 * 'offset' and 'data_len' information to remote, telling remote that the data
 * is in this area.
 *
 * Return 0 on success, or a negative number on failure.
 */
int rpbuf_transmit_buffer(rpbuf_buffer_t *buffer, unsigned int offset, unsigned int data_len);

/**
 * rpbuf_receive_buffer - Receive rpbuf buffer from remote
 * @buffer: rpbuf buffer
 * @offset(out): the address offset information received from remote
 * @data_len(out): the data length information received from remote
 * @timeout_ms: -1: wait infinitely; 0: not wait; >0: wait several milliseconds
 *
 * Get the 'offset' and 'data_len' information received from remote. If there
 * is previous arrived information, this function will return immediately.
 * Otherwise it will block to wait for 'timeout_ms'.
 *
 * Returns:
 * If 'timeout_ms' < 0:
 *     0 on success, or a negative number on failure.
 * If 'timeout_ms' >= 0:
 *     remaining milliseconds on success, or -ETIMEDOUT after 'timeout_ms' elapsed,
 *     or other negative numbers on failure.
 */
int rpbuf_receive_buffer(rpbuf_buffer_t *buffer, unsigned int *offset,
		unsigned int *data_len, int timeout_ms);

/**
 * rpbuf_install_rx_signal - install rpbuf buffer rx signal(SIGIO)
 * @buffer: rpbuf buffer
 * @handler: handler function, typedef void (*sighandler_t)(int);
 *
 * Returns: 0 on success, or a negative number on failure.
 */
int rpbuf_install_rx_signal(rpbuf_buffer_t *buffer, sighandler_t handler);

/**
 * rpbuf_install_rx_signal - remove rpbuf buffer rx signal(SIGIO)
 * @buffer: rpbuf buffer
 *
 * Returns: 0 on success, or a negative number on failure.
 */

int rpbuf_remove_rx_signal(rpbuf_buffer_t *buffer);

/**
 * rpbuf_set_sync_buffer- set the buffer to use sync transfer
 * @buffer: rpbuf buffer
 * @is_sync: if > 0, sync; else async
 *
 * if sent sync, it will block until the remoteproc complete 'rx_cb'.
 * if send async, this function doesn't care about remoteproc.
 * default is async transmit.
 *
 * Returns: 0 on success, or a negative number on failure.
 */
int rpbuf_set_sync_buffer(rpbuf_buffer_t *buffer, uint32_t is_sync);

int rpbuf_get_perf_data(rpbuf_buffer_t *buffer, rpbuf_perf_data_t *perf_data);

void rpbuf_dump_perf_data(const rpbuf_perf_data_t *perf_data);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __LIBRPBUF_H__ */
