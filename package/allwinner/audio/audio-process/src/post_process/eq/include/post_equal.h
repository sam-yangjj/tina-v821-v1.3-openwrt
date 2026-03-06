/*
 * post_equal.h - common lib api used by sunix audio post process.
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


#ifndef __POST_EQUAL_H__
#define __POST_EQUAL_H__

#include "eq.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum rpbuf_arg_post_type {
	START_RPBUFF = 0,
	RESET_RPBUFF,
	SEND_RPBUFF,
	RECV_RPBUFF,
	RPBUFF_MAX = 0X7FFFFFFF
}rpbuf_arg_post_type;


typedef struct  rpbuf_arg_post_eq{
	char aeq_config_name[32];
	char aeq_reset_config_name[32];
	char aeq_in_name[32];
	char aeq_out_name[32];
}rpbuf_arg_post_eq;


typedef struct snd_pcm_postequal snd_pcm_postequal_t;

int post_equal_open(snd_pcm_postequal_t **posteq, const char *pcm_name, rpbuf_arg_post_eq *eq_arg, int ctrl_id, unsigned int sync, size_t len);

int post_equal_close(snd_pcm_postequal_t *posteq);

int post_equal_hw_params(snd_pcm_postequal_t *posteq, unsigned int channels, unsigned int rate, unsigned int bits);
int post_equal_hw_free(snd_pcm_postequal_t *posteq);
snd_pcm_sframes_t post_equal_transfer(snd_pcm_postequal_t *posteq, const char* name,
        const void *data, unsigned long data_offset,
        unsigned long size);
int post_equal_recv(snd_pcm_postequal_t *posteq, const char* name,
        void *data, unsigned long *data_offset,
        unsigned long *size, int recv_time);

int post_equal_rpbuf_is_available(snd_pcm_postequal_t *preaec, const char* name);


#ifdef __cplusplus
}
#endif

#endif /* __POST_EQUAL_H__*/

