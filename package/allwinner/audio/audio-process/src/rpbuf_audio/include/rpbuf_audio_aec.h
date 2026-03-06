/*
 * rpbuf_audio_aec.h - common lib api used by sunxi rpbuf audio process.
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

#ifndef __RPBUF_AUDIO_AEC_H__
#define __RPBUF_AUDIO_AEC_H__

#ifdef CONFIG_COMPONENT_NF_AEC
#include "nf_DSP.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct rpbuf_audio_aec_ctx rpbuf_audio_aec_ctx_t;

int rpbuf_audio_aec_is_available(rpbuf_audio_aec_ctx_t* ctx, const char* name);

int rpbuf_audio_aec_ctrl_send_cmd(rpbuf_audio_aec_ctx_t *ctx, rpbuf_audio_remote_ctrl_aec_t *rpbuf_ctl_arg, unsigned long data_offset);

int rpbuf_audio_aec_ctrl_recv_cmd(rpbuf_audio_aec_ctx_t *ctx, void *data, unsigned long *data_offset,
                                 unsigned long *size, int recv_time);

int rpbuf_audio_aec_downlink_ctrl_recv_cmd(rpbuf_audio_aec_ctx_t *ctx, void *data, unsigned long *data_offset,
                                 unsigned long *size, int recv_time);

int rpbuf_audio_aec_transfer_data(rpbuf_audio_aec_ctx_t *ctx, const char* name, void *data,
                                 unsigned long data_offset,unsigned long size);

int rpbuf_audio_aec_recv_data(rpbuf_audio_aec_ctx_t *ctx, const char* name, void *data, unsigned long *data_offset,
                              unsigned long *size, int recv_time);


void rpbuf_audio_aec_data_deinit(rpbuf_audio_aec_ctx_t *ctx);

int rpbuf_audio_aec_data_init(rpbuf_audio_aec_ctx_t *ctx, int aec_in_len, int aec_out_len);

void rpbuf_audio_aec_downlink_data_deinit(rpbuf_audio_aec_ctx_t *ctx);

int rpbuf_audio_aec_downlink_data_init(rpbuf_audio_aec_ctx_t *ctx, int aec_in_len, int aec_out_len);

int rpbuf_audio_aec_get_data_name(rpbuf_audio_aec_ctx_t *ctx, char *aec_in, char *aec_out);

int rpbuf_audio_aec_downlink_get_data_name(rpbuf_audio_aec_ctx_t *ctx, char *aec_in, char *aec_out);

void rpbuf_audio_aec_deinit(rpbuf_audio_aec_ctx_t *ctx);

rpbuf_audio_aec_ctx_t* rpbuf_audio_aec_init(int index);


#ifdef __cplusplus
}
#endif

#endif /* __PRE_AEC_H__*/

