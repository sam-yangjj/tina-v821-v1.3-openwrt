/*
 * pre_aec.h - common lib api used by sunix audio pre process.
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


#ifndef __PRE_AEC_H__
#define __PRE_AEC_H__

#include "echo_cancellation.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum rpbuf_arg_pre_aec_type {
	CONFIG_RPBUFF = 0,
	AEC_IN_RPBUFF,
	AEC_IN_RECV_RPBUFF,
	AEC_OUT_RPBUFF,
	RPBUFF_MAX = 0X7FFFFFFF
}rpbuf_arg_pre_aec_type;


typedef struct  rpbuf_arg_pre_aec{
	char config_name[32];  // config aec
	char aec_in_name[32];  // aec in
	char aec_in_recv_name[32];  // aec in recv
	char aec_out_name[32];  //aec out
}rpbuf_arg_pre_aec;


typedef struct
{
	/* sampling rate */
	int sampling_rate;
	/* bits_per_sample */
	int bits_per_sample;
	/* channel num */
	int chan;
	/* aec parameters*/
	AecConfig aec_config;
}aec_prms_t;

typedef struct snd_pcm_pre_aec snd_pcm_pre_aec_t;

int pre_aec_open(snd_pcm_pre_aec_t **preaec, rpbuf_arg_pre_aec *aec_arg, int ctrl_id, size_t aec_in_len, size_t aec_out_len);

int pre_aec_close(snd_pcm_pre_aec_t *preaec);

int pre_aec_hw_params(snd_pcm_pre_aec_t *preaec, aec_prms_t *config, unsigned int enabled);

int pre_aec_hw_free(snd_pcm_pre_aec_t *preaec, const char* name);

snd_pcm_sframes_t pre_aec_transfer(snd_pcm_pre_aec_t *preaec, const char* name,
        const void *data, unsigned long data_offset,
        unsigned long size);

int pre_aec_recv(snd_pcm_pre_aec_t *preaec, const char* name,
        void *data, unsigned long *data_offset,
        unsigned long *size, int recv_time);

int pre_aec_rpbuf_is_available(snd_pcm_pre_aec_t *preaec, const char* name);


#ifdef __cplusplus
}
#endif

#endif /* __PRE_AEC_H__*/

