/*
 * audio_common.c - audio api used by sunxi media framework.
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

#include "local_audio_common.h"

#include "audio_common.h"


#if AUDIO_PROCESS_SUNXI_AUDIO_ONE
#pragma message("[THH]AUDIO DRIVER V1")
#elif AUDIO_PROCESS_SUNXI_AUDIO_TWO
#pragma message("[THH]AUDIO DRIVER V2")
#else
#pragma message("[THH]DRIVER VERSION MACRO")
#endif

static char card[32] = "default";

/* Open pcm device */
void *open_audio(const char *name, snd_pcm_stream_t flags, pcm_config_t *config)
{
	int ret;
	snd_pcm_t *pcm_dev = NULL;
	snd_pcm_hw_params_t *hw;
	int format;

	if (flags == SND_PCM_STREAM_CAPTURE) {
		ret = snd_pcm_open(&pcm_dev, name, SND_PCM_STREAM_CAPTURE, 0);
		if (ret < 0) {
			printf("open pcm err\n");
			return NULL;
		}

		format = snd_pcm_format_physical_width(config->format);
		if (format < 0)
		{
			printf("snd pcm format physical width err\n");
			return NULL;
		}

		printf("arecord samplerate=%d, ch=%d, bits=%d \n", \
		config->rate, config->channels, format);

		snd_pcm_hw_params_malloc(&hw);
		snd_pcm_hw_params_any(pcm_dev, hw);
		snd_pcm_hw_params_set_access(pcm_dev,
			hw, SND_PCM_ACCESS_RW_INTERLEAVED);
		snd_pcm_hw_params_set_format(pcm_dev,
			hw, config->format);
		snd_pcm_hw_params_set_channels(pcm_dev, hw, config->channels);
		snd_pcm_hw_params_set_rate(pcm_dev, hw, config->rate, 0);

		if (!config->period_size)
			config->period_size = REC_PERIOD_SIZE;
		if (!config->period_size && !config->period_count)
			config->period_count = REC_PERIOD_COUNT;

		snd_pcm_hw_params_set_period_size(pcm_dev, hw, config->period_size, 0);
		snd_pcm_hw_params_set_buffer_size(pcm_dev, hw, config->period_size * config->period_count);
		if (snd_pcm_hw_params(pcm_dev, hw) < 0) {
			printf("snd_pcm_hw_params error\n");
			snd_pcm_close(pcm_dev);
			return NULL;
		}
		snd_pcm_hw_params_free(hw);
	} else if (flags == SND_PCM_STREAM_PLAYBACK) {
		ret = snd_pcm_open(&pcm_dev, name, SND_PCM_STREAM_PLAYBACK, 0);
		if (ret < 0) {
			printf("open pcm err\n");
			return NULL;
		}

		format = snd_pcm_format_physical_width(config->format);
		if (format < 0)
		{
			printf("snd pcm format physical width err\n");
			return NULL;
		}

		printf("aplay samplerate=%d, ch=%d, bits =%d \n", \
		config->rate, config->channels, format);
		/*bytes_perframe = config->bitwidth * config->channels / 8;*/
		snd_pcm_hw_params_malloc(&hw);
		snd_pcm_hw_params_any(pcm_dev, hw);
		snd_pcm_hw_params_set_access(pcm_dev,
			hw, SND_PCM_ACCESS_RW_INTERLEAVED);
		snd_pcm_hw_params_set_format(pcm_dev,
			hw, config->format);
		snd_pcm_hw_params_set_channels(pcm_dev, hw, config->channels);
		snd_pcm_hw_params_set_rate(pcm_dev, hw, config->rate, 0);

		if (!config->period_size)
			config->period_size = REPLAY_PERIOD_SIZE;
		if (!config->period_size && !config->period_count)
			config->period_count = REPLAY_PERIOD_COUNT;

		snd_pcm_hw_params_set_period_size(pcm_dev, hw, config->period_size, 0);
		snd_pcm_hw_params_set_buffer_size(pcm_dev, hw, config->period_size * config->period_count);

		if (snd_pcm_hw_params(pcm_dev, hw) < 0) {
			printf("snd_pcm_hw_params error\n");
			snd_pcm_close(pcm_dev);
			return NULL;
		}
		snd_pcm_hw_params_free(hw);
	} else {
		printf("input param flag %d error\n", flags);
		return NULL;
	}

	return pcm_dev;
}

int close_audio(void *hal, unsigned int flags)
{
	int ret = 0;

	if (!hal) {
		printf("pcm null\n");
		return  -1;
	}

	snd_pcm_t *pcm_dev = (snd_pcm_t *)hal;

	ret = snd_pcm_drain(pcm_dev);
	if (ret < 0)
		printf("stop failed!, return %d\n", ret);

	/* close card */
	ret = snd_pcm_close(pcm_dev);
	if (ret < 0) {
		printf("audio close error:%d\n", ret);
	}

	return ret;
}

int audio_write(void *hal, void *data, unsigned long count)
{
	int ret = 0;
	snd_pcm_t *pcm_dev = (snd_pcm_t *)hal;

	ret = snd_pcm_writei(pcm_dev, data, count);
	if (ret == -EPIPE) {
		int res;
		printf("snd_pcm_writei err EPIPE, underrun occurred\n");
		res = snd_pcm_prepare(pcm_dev);
		if (res < 0)
			printf("snd_pcm_prepare err(%d, %s)\n", res, snd_strerror(res));
		return ret;
	} else if (ret < 0) {
		printf("write to pcm err(%d, %s)\n", ret, snd_strerror(ret));
		return ret;
	}
	return ret;
}


int audio_read(void *hal, void *data, unsigned long count)
{
	int ret = 0;
	snd_pcm_t *pcm_dev = (snd_pcm_t *)hal;

	ret = snd_pcm_readi(pcm_dev, data, count);
	//printf("read ret %d, alloc len %lu\n", ret, count);
	if (ret == -EPIPE) {
		int res;
		printf("audio read data error ret %d\n", ret);
		res = snd_pcm_prepare(pcm_dev);
		if (res < 0)
			printf("snd_pcm_prepare err(%d, %s)\n", res, snd_strerror(res));
		return ret;
	}  else if (ret <= 0) {
		printf("audio read data error ret %d\n", ret);
		return ret;
	}
	return ret;
}


int audio_set_playback_route(const char *name, int status, route_type_t type)
{
	int err = 0;
	snd_ctl_t *handle = NULL;
	snd_ctl_elem_value_t *control = NULL;

	printf("enter audio set playback route status [%d]\n", status);
	err = snd_ctl_open(&handle, name, 0);
	if (err < 0) {
		printf("Control %s open error: %s\n", name, snd_strerror(err));
		return err;
	}
	err = snd_ctl_elem_value_malloc(&control);
	if (err < 0) {
		printf("Control %s malloc error: %s\n", name, snd_strerror(err));
		goto EXIT;
	}


	switch (type) {

		case HeadPhone:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "HPOUT Switch");
#else
			snd_ctl_elem_value_set_name(control, "Headphone Switch");
#endif
			snd_ctl_elem_value_set_boolean(control, 0, status);
			break;
		}
		case HpSpeaker:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "HPOUT Switch");
#else
			snd_ctl_elem_value_set_name(control, "Headphone Switch");
#endif
			snd_ctl_elem_value_set_boolean(control, 0, status);

			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "SPK Switch");
#else
			snd_ctl_elem_value_set_name(control, "HpSpeaker Switch");
#endif
			snd_ctl_elem_value_set_boolean(control, 0, status);
			break;
		}
		case CodecHubMode:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "tx hub mode");
#else
			snd_ctl_elem_value_set_name(control, "codec hub mode");
#endif
			snd_ctl_elem_value_set_enumerated(control, 0, status);
			break;
		}
		case DaudioHubMode:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "tx hub mode");
#else
			snd_ctl_elem_value_set_name(control, "sunxi daudio audio hub mode");
#endif
			snd_ctl_elem_value_set_enumerated(control, 0, status);
			break;
		}
		case DaudioLoopBack:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "loopback debug");
#else
			snd_ctl_elem_value_set_name(control, "sunxi daudio loopback debug");
#endif
			snd_ctl_elem_value_set_enumerated(control, 0, status);
			break;
		}

		default:
			printf("type %d  is error\n", type);
			goto EXIT;

	}


	err = snd_ctl_elem_write(handle, control);
	if (err < 0) {
		printf("Control %s element write error: %s\n", name,
			snd_strerror(err));
		goto EXIT;
	}

EXIT:
	if (handle) {
		snd_ctl_close(handle);
		handle = NULL;
	}

	if (control) {
		snd_ctl_elem_value_free(control);
		control = NULL;
	}
	return err;
}


int audio_set_record_route(const char *name, int status, route_type_t type)
{
	int err = 0;
	snd_ctl_t *handle = NULL;
	snd_ctl_elem_value_t *control = NULL;

	printf("enter audio set record route status [%d]\n", status);
	err = snd_ctl_open(&handle, name, 0);
	if (err < 0) {
		printf("Control %s open error: %s\n", name, snd_strerror(err));
		return err;
	}
	err = snd_ctl_elem_value_malloc(&control);
	if (err < 0) {
		printf("Control %s malloc error: %s\n", name, snd_strerror(err));
		goto EXIT;
	}

	switch (type) {

		case MicCapture:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "MIC3 Switch");
#else
			snd_ctl_elem_value_set_name(control, "ADC3 Input MIC3 Boost Switch");
#endif
			snd_ctl_elem_value_set_boolean(control, 0, status);
			break;
		}
		case DaudioLoopBack:
		{
			snd_ctl_elem_value_set_interface(control, SND_CTL_ELEM_IFACE_MIXER);
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
			snd_ctl_elem_value_set_name(control, "loopback debug");
#else
			snd_ctl_elem_value_set_name(control, "sunxi daudio loopback debug");
#endif
			snd_ctl_elem_value_set_enumerated(control, 0, status);
			break;
		}

		default:
			printf("type %d  is error\n", type);
			goto EXIT;

	}

	err = snd_ctl_elem_write(handle, control);
	if (err < 0) {
		printf("Control %s element write error: %s\n", name,
			snd_strerror(err));
		goto EXIT;
	}

EXIT:
	if (handle) {
		snd_ctl_close(handle);
		handle = NULL;
	}

	if (control) {
		snd_ctl_elem_value_free(control);
		control = NULL;
	}
	return err;
}

