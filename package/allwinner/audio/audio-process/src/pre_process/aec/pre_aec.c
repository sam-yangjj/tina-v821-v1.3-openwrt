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


#include "local_pre_aec.h"
#include "pre_aec.h"


static void print_aec_prms(const aec_prms_t *prms)
{

    SNDERR("  sampling_rate: %d, bits_per_sample: %d, chan: %d",
            prms->sampling_rate, prms->bits_per_sample, prms->chan);

    SNDERR("  nlpMode: %d, skewMode: %d, metricsMode: %d delay_logging %d",
            prms->aec_config.nlpMode, prms->aec_config.skewMode, prms->aec_config.metricsMode, prms->aec_config.delay_logging);

}

int pre_aec_rpbuf_is_available(snd_pcm_pre_aec_t *preaec, const char* name)
{
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	rpbuf_buffer_t * buffer = NULL;
	int i = 0;
	int ret = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awaec->rp_buffer[i]), name, strlen(name) + 1)) {
			buffer = awaec->rp_buffer[i];
			break;
		}
	}

	if (i >= PRE_AEC_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
		return 0;
	}

	mutex_eq = awaec->mutex_send_eq[i];

    pthread_mutex_lock(&mutex_eq);
	ret = rpbuf_buffer_is_available(buffer);
    pthread_mutex_unlock(&mutex_eq);

	return ret;
}

snd_pcm_sframes_t pre_aec_transfer(snd_pcm_pre_aec_t *preaec, const char* name,
        const void *data, unsigned long data_offset,
        unsigned long size)
{
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	rpbuf_buffer_t * buffer = NULL;
	int ret = -1;
	void *buf_addr = NULL;
	size_t len = 0;
	int i = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awaec->rp_buffer[i]), name, strlen(name) + 1)) {
			buffer = awaec->rp_buffer[i];
			break;
		}
	}

	if (i >= PRE_AEC_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
		return 0;
	}

	mutex_eq = awaec->mutex_send_eq[i];

    pthread_mutex_lock(&mutex_eq);

    if (!buffer) {
        SNDERR("Invalid buffer");
        pthread_mutex_unlock(&mutex_eq);
        return 0;
    }

	if (awaec->aec_enabled) {

		len = rpbuf_buffer_len(buffer);
		if (len < size) {
			SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(buffer), len, size);
			pthread_mutex_unlock(&mutex_eq);
			ret = 0;
			goto err_out;
		}

		buf_addr = rpbuf_buffer_addr(buffer);

	    memcpy(buf_addr + data_offset, data, size);

		ret = rpbuf_transmit_buffer(buffer, data_offset, size);
		if (ret < 0) {
			SNDERR("transmit buffer %s error\n", rpbuf_buffer_name(buffer));
			pthread_mutex_unlock(&mutex_eq);
			goto err_out;
		}
    }

    pthread_mutex_unlock(&mutex_eq);

    return size;

err_out:
	return ret;
}

int pre_aec_recv(snd_pcm_pre_aec_t *preaec, const char* name,
        void *data, unsigned long *data_offset,
        unsigned long *size, int recv_time)
{
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	rpbuf_buffer_t * buffer = NULL;
	int ret = -1;
	void *buf_addr= NULL;
	size_t len = 0;
	int i = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awaec->rp_buffer[i]), name, strlen(name))) {
			buffer = awaec->rp_buffer[i];
			break;
		}
	}

	if (i >= PRE_AEC_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
        return 0;
	}

	mutex_eq = awaec->mutex_recv_eq[i];

    pthread_mutex_lock(&mutex_eq);

	if (!buffer) {
        SNDERR("Invalid buffer");
        pthread_mutex_unlock(&mutex_eq);
        return 0;
    }

	if (awaec->aec_enabled) {

		buf_addr = rpbuf_buffer_addr(buffer);

		ret = rpbuf_receive_buffer(buffer, (unsigned int *)data_offset, (unsigned int *)size, recv_time);
		if (ret < 0) {
			if (ret != -ETIMEDOUT)
				printf("receive buffer %s error, ret=%d\n", rpbuf_buffer_name(buffer), ret);
			else
				printf("receive buffer %s timeout\n", rpbuf_buffer_name(buffer));
			goto err_out;
		}

		len = rpbuf_buffer_len(buffer);
		if (len != *size) {
			SNDERR("buffer %s len %lu is not equal to input size %lu\n", rpbuf_buffer_name(buffer), len, *size);
			goto err_out;
		}

		memcpy(data, buf_addr + *data_offset, *size);
	}

	pthread_mutex_unlock(&mutex_eq);

	return 0;
err_out:
	pthread_mutex_unlock(&mutex_eq);
	return ret;
}

int pre_aec_hw_params(snd_pcm_pre_aec_t *preaec, aec_prms_t *config, unsigned int enabled)
{
    int ret = 0;
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	void *buf_addr = NULL;
	unsigned int offset = 0;
	size_t len;

    if (!awaec) {
        ret = -1;
        goto out;
    }

    awaec->aec_enabled = enabled;

	print_aec_prms(config);

	len = rpbuf_buffer_len(awaec->rp_buffer[CONFIG_RPBUFF]);
	if (len < sizeof(aec_prms_t)) {
		SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(awaec->rp_buffer[CONFIG_RPBUFF]), len, sizeof(aec_prms_t));
		goto out;
	}

	buf_addr = rpbuf_buffer_addr(awaec->rp_buffer[CONFIG_RPBUFF]);

	memcpy(buf_addr, config, sizeof(aec_prms_t));

	ret = rpbuf_transmit_buffer(awaec->rp_buffer[CONFIG_RPBUFF], offset, sizeof(aec_prms_t));
	if (ret < 0) {
		SNDERR("transmit buffer %s error\n", rpbuf_buffer_name(awaec->rp_buffer[CONFIG_RPBUFF]));
		goto out;
	}

    ret = 0;
out:
    return ret;
}

int pre_aec_hw_free(snd_pcm_pre_aec_t *preaec, const char* name)
{
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	rpbuf_buffer_t * buffer = NULL;
	int i = 0;
	int ret = 0;
	pthread_mutex_t mutex_eq;

    if (!awaec) {
        return 0;
    }

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awaec->rp_buffer[i]), name, strlen(name) + 1)) {
			buffer = awaec->rp_buffer[i];
			break;
		}
	}

	if (i >= PRE_AEC_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
		return 0;
	}


	mutex_eq = awaec->mutex_send_eq[i];


    pthread_mutex_lock(&mutex_eq);

	if (buffer) {
		rpbuf_free_buffer(buffer);
		buffer = NULL;
	}

    pthread_mutex_unlock(&mutex_eq);

    return 0;
}

int pre_aec_close(snd_pcm_pre_aec_t *preaec)
{
    snd_pcm_pre_aec_t *awaec = (snd_pcm_pre_aec_t *)preaec;
	int i;

    if (!awaec) {
        return 0;
    }

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if (awaec->rp_buffer[i]) {
			rpbuf_free_buffer(awaec->rp_buffer[i]);
	        awaec->rp_buffer[i] = NULL;
    	}
	}

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awaec->mutex_recv_eq[i]);
	}
	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awaec->mutex_send_eq[i]);
	}

    free(awaec);
    awaec = NULL;
    return 0;
}

int pre_aec_open(snd_pcm_pre_aec_t **preaec, rpbuf_arg_pre_aec *aec_arg, int ctrl_id, size_t aec_in_len, size_t aec_out_len)
{
    snd_pcm_pre_aec_t *awaec= NULL;
    int err = 0;
	int i;

	if (!strlen(aec_arg->config_name) || !strlen(aec_arg->aec_in_name) || !strlen(aec_arg->aec_in_recv_name) || !strlen(aec_arg->aec_out_name)) {
		SNDERR("name is all null");
		return -1;
	}

	SNDERR("config_name %s  aec_in_name %s aec_out_name %s !\n", \
			aec_arg->config_name, aec_arg->aec_in_name, aec_arg->aec_out_name);


    awaec = calloc(1, sizeof(*awaec));
    if (!awaec) {
        err = -ENOMEM;
        goto error;
    }

	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
	    err = pthread_mutex_init(&awaec->mutex_send_eq[i], NULL);
	    if (err < 0) {
	        SNDERR("pthread_mutex_init error");
	        goto destroy_mutex_eq;
	    }
	}

    for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
	    err = pthread_mutex_init(&awaec->mutex_recv_eq[i], NULL);
	    if (err < 0) {
	        SNDERR("pthread_mutex_init error");
	        goto destroy_mutex_recv_eq;
	    }
	}

    /* It will be block to wait the remote process to alloc buffer, the waiting time is 15s */
	awaec->rp_buffer[CONFIG_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, aec_arg->config_name, sizeof(aec_prms_t));
	if (!awaec->rp_buffer[CONFIG_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", aec_arg->config_name, sizeof(aec_prms_t));
		err = -1;
        goto destroy_alloc_buffer;
	}

	awaec->rp_buffer[AEC_IN_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, aec_arg->aec_in_name, aec_in_len);
	if (!awaec->rp_buffer[AEC_IN_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", aec_arg->aec_in_name, aec_in_len);
		err = -1;
        goto destroy_alloc_buffer;
	}

	awaec->rp_buffer[AEC_IN_RECV_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, aec_arg->aec_in_recv_name, aec_in_len);
	if (!awaec->rp_buffer[AEC_IN_RECV_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", aec_arg->aec_in_recv_name, aec_in_len);
		err = -1;
        goto destroy_alloc_buffer;
	}

	awaec->rp_buffer[AEC_OUT_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, aec_arg->aec_out_name, aec_out_len);
	if (!awaec->rp_buffer[AEC_OUT_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", aec_arg->aec_out_name, aec_out_len);
		err = -1;
        goto destroy_alloc_buffer;
	}

	*preaec = awaec;

    return 0;

destroy_alloc_buffer:
	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		if (awaec->rp_buffer[i]) {
			rpbuf_free_buffer(awaec->rp_buffer[i]);
	        awaec->rp_buffer[i] = NULL;
    	}
	}
destroy_mutex_recv_eq:
	for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awaec->mutex_recv_eq[i]);
	}
destroy_mutex_eq:
    for (i = 0; i < PRE_AEC_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awaec->mutex_send_eq[i]);
	}
error:
    if (awaec) {
        free(awaec);
		awaec = NULL;
    }
    return err;
}

