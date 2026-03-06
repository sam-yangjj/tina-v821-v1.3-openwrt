/*
 * local_rpbuf_audio.h - common lib api used by local sunix audio pre aec process.
 *
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


#ifndef __LOCAL_RPBUF_AUDIO_H__
#define __LOCAL_RPBUF_AUDIO_H__

#include <pthread.h>
#include <list.h>
#include "librpbuf.h"

#define RPBUF_AUDIO_CRTL_AEC_COUNTS		1

#define RPBUF_AUDIO_CRTL_EQ_COUNTS		2

#define RPBUF_AUDIO_CRTL_RAW_COUNTS		1

typedef struct rpbuf_buffer_entry {
	rpbuf_buffer_t *buffer;
	struct list_head list;
}rpbuf_buffer_entry_t;

struct rpbuf_audio_aec_ctx {
	rpbuf_buffer_entry_t *rpbuf_ctl[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_ctl_ack[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_ctl_ack_downlink[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_aec_in[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_aec_out[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_aec_downlink_in[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_aec_downlink_out[RPBUF_AUDIO_CRTL_AEC_COUNTS];
	pthread_mutex_t      rpbuf_mutex;
	pthread_mutex_t      rpbuf_send_mutex;
	pthread_mutex_t      rpbuf_recv_mutex;
	pthread_mutex_t      rpbuf_recv_downlink_mutex;
	char rpbuf_aec_ctrl_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_ctrl_ack_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_ctrl_ack_downlink_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_in_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_out_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_downlink_in_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	char rpbuf_aec_downlink_out_name[RPBUF_AUDIO_CRTL_AEC_COUNTS][32];
	int rpbuf_aec_count;
	unsigned int rpbuf_aec_index;
	unsigned int  rpbuf_aec_in_len;
	unsigned int  rpbuf_aec_out_len;
	unsigned int  rpbuf_aec_downlink_in_len;
	unsigned int  rpbuf_aec_downlink_out_len;
};


struct rpbuf_audio_eq_ctx {
	rpbuf_buffer_entry_t *rpbuf_ctl[RPBUF_AUDIO_CRTL_EQ_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_ctl_ack[RPBUF_AUDIO_CRTL_EQ_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_eq_in[RPBUF_AUDIO_CRTL_EQ_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_eq_out[RPBUF_AUDIO_CRTL_EQ_COUNTS];
	pthread_mutex_t      rpbuf_mutex;
	pthread_mutex_t      rpbuf_send_mutex;
	pthread_mutex_t      rpbuf_recv_mutex;
	char rpbuf_eq_ctrl_name[RPBUF_AUDIO_CRTL_EQ_COUNTS][32];
	char rpbuf_eq_ctrl_ack_name[RPBUF_AUDIO_CRTL_EQ_COUNTS][32];
	char rpbuf_eq_in_name[RPBUF_AUDIO_CRTL_EQ_COUNTS][32];
	char rpbuf_eq_out_name[RPBUF_AUDIO_CRTL_EQ_COUNTS][32];
	unsigned int rpbuf_eq_index;
	unsigned int  rpbuf_eq_in_len;
	unsigned int  rpbuf_eq_out_len;
};

struct rpbuf_audio_raw_ctx {
	rpbuf_buffer_entry_t *rpbuf_ctl[RPBUF_AUDIO_CRTL_RAW_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_ctl_ack[RPBUF_AUDIO_CRTL_RAW_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_raw_in[RPBUF_AUDIO_CRTL_RAW_COUNTS];
	rpbuf_buffer_entry_t *rpbuf_raw_out[RPBUF_AUDIO_CRTL_RAW_COUNTS];
	pthread_mutex_t      rpbuf_mutex;
	pthread_mutex_t      rpbuf_send_mutex;
	pthread_mutex_t      rpbuf_recv_mutex;
	char rpbuf_raw_ctrl_name[RPBUF_AUDIO_CRTL_RAW_COUNTS][32];
	char rpbuf_raw_ctrl_ack_name[RPBUF_AUDIO_CRTL_RAW_COUNTS][32];
	char rpbuf_raw_in_name[RPBUF_AUDIO_CRTL_RAW_COUNTS][32];
	char rpbuf_raw_out_name[RPBUF_AUDIO_CRTL_RAW_COUNTS][32];
	unsigned int rpbuf_raw_index;
	unsigned int  rpbuf_raw_in_len;
	unsigned int  rpbuf_raw_out_len;
};



#endif /* __LOCAL_RPBUF_AUDIO_H__*/

