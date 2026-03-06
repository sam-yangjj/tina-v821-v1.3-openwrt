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
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <alsa/asoundlib.h>
//#define __USE_GNU
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "rpbuf_audio_ctrl_eq.h"

#include "rpbuf_audio_eq.h"
#include "audio_common.h"
#include "thread_sem.h"

#define RPBUF_AUDIO_POST_EQ_DEFAULT_SIZE 960
#define RPBUF_DEFAULT_POST_EQ_BUF_CNT 4

#define POST_EQ_PCM_NAME_DEFAULT "default"
#define POST_EQ_LINE_STR_BUF_LEN_MAX 128

#define POST_EQ_DEFAULT_FORMAT		SND_PCM_FORMAT_S16_LE
#define POST_EQ_DEFAULT_RATE 		16000
#define POST_EQ_DEFAULT_CHANNEL 	6
#define POST_EQ_DEFAULT_MSEC 		16

#define RPBUF_EQ_OUT_CHANNEL		6

#define POST_EQ_FORMAT_DEFAULT		-1
#define POST_EQ_FORMAT_RAW		0
#define POST_EQ_FORMAT_VOC		1
#define POST_EQ_FORMAT_WAVE		2

#define EQ_DUMP_FILE
#define RPBUF_DSP_EQ_TEST
//#define RPBUF_DSP_EQ_DUMP_INPUT_DATA

/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092

static struct {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
	unsigned int msec;
} posthwparams, rposthwparams;

static snd_pcm_stream_t post_stream = SND_PCM_STREAM_PLAYBACK;
static int post_file_type = POST_EQ_FORMAT_DEFAULT;
static int g_rpbuf_index = 0;
static int g_config_eq = 0;
static char *pcm_name = POST_EQ_PCM_NAME_DEFAULT;
static char filename[32] = "/tmp/test.pcm";
static char EqConfigName[32] = "/etc/eq.conf";
static char PE4_output_low[32]  = "PE4 1 0 1 0";
static char PE4_output_high[32] = "PE4 1 1 1 0";
static char sunix_pin[64] = "/sys/kernel/debug/sunxi_pinctrl/sunxi_pin_configure";
static volatile sig_atomic_t in_aborting = 0;

/* RIFF HEADER of wav file */
struct RIFF_HEADER_DEF {
	char riff_id[4];     /* 'R', 'I', 'F', 'F' */
	uint32_t riff_size;
	char riff_format[4]; /* 'W', 'A', 'V', 'E' */
};

struct WAVE_FORMAT_DEF {
	uint16_t format_tag;
	uint16_t channels; /* channels of the audio stream */
	uint32_t samples_per_sec; /* sample rate */
	uint32_t avg_bytes_per_sec;
	uint16_t block_align;
	uint16_t bits_per_sample; /* bits per sample */
};

struct FMT_BLOCK_DEF {
	char fmt_id[4];    /* 'f', 'm', 't', ' ' */
	uint32_t fmt_size;
	struct WAVE_FORMAT_DEF wav_format;
};

struct DATA_BLOCK_DEF {
	char data_id[4];     /* 'R', 'I', 'F', 'F' */
	uint32_t data_size;
};

struct wav_info {
	struct RIFF_HEADER_DEF header;
	struct FMT_BLOCK_DEF   fmt_block;
	struct DATA_BLOCK_DEF  data_block;
};

typedef struct rpbuf_eq_ctx_t {
	snd_pcm_t *pcm_dev;
	rpbuf_audio_eq_ctx_t *rpbuf_audio_ctx;
	rpbuf_audio_remote_ctrl_eq_t *rpbuf_ctl_arg;
	int rpbuf_eq_index;
	char eq_in_name[32];
	char eq_out_name[32];
	FILE *infile;
	unsigned int rpbuf_eq_in_len;
	unsigned int rpbuf_eq_out_len;
	struct wav_info audio_info;
	int exit_config;
}rpbuf_eq_ctx_t;

static long long get_currenttime()
{
	struct timeval time;

	gettimeofday(&time, NULL);

	return ((long long)time.tv_sec * 1000000 + (long long)time.tv_usec);
}

static int to_pcm_fmt(struct wav_info *wav_info)
{
	int ret = 0;

	uint16_t bit = wav_info->fmt_block.wav_format.bits_per_sample;

	unsigned short format = wav_info->fmt_block.wav_format.format_tag;

	int big_endian = 0;

	int native_format;

	printf("riff_id %s, riff_format %s\n", wav_info->header.riff_id, wav_info->header.riff_format);

	if (!strncmp(wav_info->header.riff_id, "RIFF", sizeof(wav_info->header.riff_id)))
		big_endian = 0;
	else if (!strncmp(wav_info->header.riff_id, "RIFX", sizeof(wav_info->header.riff_id)))
		big_endian = 1;

	if (strncmp(wav_info->header.riff_format, "WAVE", sizeof(wav_info->header.riff_format))) {
		printf("riff_format %s is err\n", wav_info->header.riff_format);
		return -1;
	}

	switch (bit) {
		case 8:
			if (posthwparams.format != POST_EQ_DEFAULT_FORMAT &&
				posthwparams.format != SND_PCM_FORMAT_U8)
				printf("Warning: format is changed to U8\n");
			posthwparams.format = SND_PCM_FORMAT_U8;
			break;
		case 16:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S16_BE;
			else
				native_format = SND_PCM_FORMAT_S16_LE;
			if (posthwparams.format != POST_EQ_DEFAULT_FORMAT &&
				posthwparams.format != native_format)
				printf("Warning: format is changed to %s\n", \
					snd_pcm_format_name(native_format));
			posthwparams.format = native_format;
			break;
		case 24:
			switch (bit / posthwparams.channels) {
			case 3:
				if (big_endian)
					native_format = SND_PCM_FORMAT_S24_3BE;
				else
					native_format = SND_PCM_FORMAT_S24_3LE;
				if (posthwparams.format != POST_EQ_DEFAULT_FORMAT &&
					posthwparams.format != native_format)
					printf("Warning: format is changed to %s\n", \
						snd_pcm_format_name(native_format));
				posthwparams.format = native_format;
				break;
			case 4:
				if (big_endian)
					native_format = SND_PCM_FORMAT_S24_BE;
				else
					native_format = SND_PCM_FORMAT_S24_LE;
				if (posthwparams.format != POST_EQ_DEFAULT_FORMAT &&
					posthwparams.format != native_format)
					printf("Warning: format is changed to %s\n", \
						snd_pcm_format_name(native_format));
				posthwparams.format = native_format;
				break;
			default:
				printf(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)", \
					  wav_info->fmt_block.wav_format.bits_per_sample,
					  wav_info->fmt_block.wav_format.block_align,
					  posthwparams.channels);
				ret = -1;
			}
			break;
		case 32:
			if (format == WAV_FMT_PCM) {
				if (big_endian)
					native_format = SND_PCM_FORMAT_S32_BE;
				else
					native_format = SND_PCM_FORMAT_S32_LE;
							posthwparams.format = native_format;
			} else if (format == WAV_FMT_IEEE_FLOAT) {
				if (big_endian)
					native_format = SND_PCM_FORMAT_FLOAT_BE;
				else
					native_format = SND_PCM_FORMAT_FLOAT_LE;
				posthwparams.format = native_format;
			}
			break;
		default:
			printf(" can't play WAVE-files with sample %d bits wide", bit);
			ret = -1;
		}

	return ret;
}

static int rpbuf_pcm_s16le_split_to_plane(rpbuf_eq_ctx_t* priv, short *dst, short *src, unsigned int chan, unsigned int len)
{
	rpbuf_eq_ctx_t *private = (rpbuf_eq_ctx_t *)priv;
	uint32_t rate = 0;
	uint32_t channels = 0;
	uint8_t bitwidth = 0;
	uint32_t BytesPerSample = 0;
	short nSampleCount = 0;
	int i = 0;
	int j = 0;

	channels = chan;
	bitwidth = snd_pcm_format_physical_width(posthwparams.format);
	rate	 = posthwparams.rate;

	if (channels == 0 || bitwidth== 0 || rate == 0) {
		printf("input param error, rate %d, ch %d, width %d\n", \
				rate, channels, bitwidth);
		return -1;
	}

	BytesPerSample = bitwidth * channels / 8;

	nSampleCount = len / BytesPerSample;

	for (i = 0; i < nSampleCount; ++i) {
		for (j = 0; j < channels; j++) {
			dst[i + nSampleCount * j] = src[channels * i + j];
		}
	}

	return 0;
}

static int rpbuf_pcm_s16le_plane_to_split(rpbuf_eq_ctx_t* priv, short *dst, short *src, unsigned int chan, unsigned int len)
{
	rpbuf_eq_ctx_t *private = (rpbuf_eq_ctx_t *)priv;
	uint32_t rate = 0;
	uint32_t channels = 0;
	uint8_t bitwidth = 0;
	uint32_t BytesPerSample = 0;
	short nSampleCount = 0;
	int i = 0;
	int j = 0;

	channels = chan;
	bitwidth = snd_pcm_format_physical_width(posthwparams.format);
	rate     = posthwparams.rate;

	if (channels == 0 || bitwidth== 0 || rate == 0) {
		printf("input param error, rate %d, ch %d, width %d\n", \
				rate, channels, bitwidth);
		return -1;
	}

	BytesPerSample = bitwidth * channels / 8;

	nSampleCount = len / BytesPerSample;

	for (i = 0; i < nSampleCount; ++i) {
		for (j = 0; j < channels; j++) {
			dst[channels * i + j] = src[i + nSampleCount * j];
		}
	}

	return 0;
}

#ifdef CONFIG_COMPONENTS_PROCESS_AW_EQ
static int parse_config_to_eq_prms_transfer(const char *config_file, eq_prms_transfer_t *prms)
{
    int ret;
    FILE *fp = NULL;
    char line_str[POST_EQ_LINE_STR_BUF_LEN_MAX];
    int temp_int;
    int type;
    int frequency;
    int gain;
    float Q;
    int index = 0;

    if (!config_file || !prms) {
        printf("Invalid config_file or prms");
        ret = -1;
        goto out;
    }

    fp = fopen(config_file, "r");
    if (!fp) {
        printf("Failed to open %s (%s)", config_file, strerror(errno));
        ret = -1;
        goto out;
    }

    while (fgets(line_str, POST_EQ_LINE_STR_BUF_LEN_MAX, fp)) {
        if (sscanf(line_str, "bin_num=%d", &temp_int) == 1) {
            prms->biq_num = temp_int;
        } else if (sscanf(line_str, "params=%d %d %d %f",
                    &type, &frequency, &gain, &Q) == 4) {
            prms->core_prms[index].type = type;
            prms->core_prms[index].fc = frequency;
            prms->core_prms[index].G = gain;
            prms->core_prms[index].Q = Q;
            ++index;
        }
    }
    ret = 0;

    fclose(fp);
out:
    return ret;
}

static void rpbuf_print_eq_pararms(const eq_prms_transfer_t *prms)
{
    int i;
    for (i = 0; i < prms->biq_num; ++i) {
        const eq_core_prms_transfer_t *core_prms = &prms->core_prms[i];
        printf(" [Biquad%02i] index: %i, type: %i, freq: %d, gain: %d, Q: %.2f\n",
                i + 1, core_prms->index ,core_prms->type, core_prms->fc, core_prms->G, core_prms->Q);
    }
}
#endif

int rpbuf_eq_config(rpbuf_eq_ctx_t *priv, char *path)
{
	int ret = -1;
	int i = 0;

	/* config algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_CONFIG;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;

	priv->rpbuf_ctl_arg->channels = posthwparams.channels;
	priv->rpbuf_ctl_arg->out_channels = posthwparams.channels;
	priv->rpbuf_ctl_arg->bits = snd_pcm_format_physical_width(posthwparams.format);
	priv->rpbuf_ctl_arg->rate = posthwparams.rate;
	priv->rpbuf_ctl_arg->msec = posthwparams.msec;

	if (path == NULL || strlen(path) == 0)
		path = EqConfigName;

	printf("rpbuf eq config path:%s\n", path);

#ifdef CONFIG_COMPONENTS_PROCESS_AW_EQ
	eq_prms_transfer_t *eq_prms_transfer = &priv->rpbuf_ctl_arg->eqPrmsTransfer;

	/* eq config */
	ret = parse_config_to_eq_prms_transfer(path, eq_prms_transfer);
	if (ret != 0) {
		printf("parse_config_to_eq_prms_transfer failed\n");
		return ret;
	}

	eq_prms_transfer->chan = posthwparams.channels;
	eq_prms_transfer->sampling_rate = posthwparams.rate;
#endif

	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_CONFIG);
		return ret;
	}

	//rpbuf_print_eq_pararms(eq_prms_transfer);

	return 0;
}

int rpbuf_eq_init(rpbuf_eq_ctx_t *priv)
{
	int ret = -1;

	/* init rpbuf audio */
	priv->rpbuf_audio_ctx = rpbuf_audio_eq_init(priv->rpbuf_eq_index);
	if (priv->rpbuf_audio_ctx == NULL) {
		printf("rpbuf_audio_init failed\n");
		goto EXIT;
	}

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_ENABLE_ALG;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	priv->rpbuf_ctl_arg->value = 1;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_ENABLE_ALG);
		goto EXIT;
	}

#ifdef RPBUF_DSP_EQ_DUMP_INPUT_DATA
	/* enable raw data dump */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_DUMP_MERGE_DATA;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	priv->rpbuf_ctl_arg->value = 1;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_DUMP_MERGE_DATA);
		goto EXIT;
	}
#endif


	/* enable init */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_INIT;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_INIT);
		goto EXIT;
	}

	/* enable eq rpbuf transfer */
	ret = rpbuf_audio_eq_data_init(priv->rpbuf_audio_ctx, RPBUF_EQ_IN_LENGTH, RPBUF_EQ_OUT_LENGTH);
	if (ret != 0)
	{
		printf("%s %d rpbuf_audio_eq_data_init failed", __func__, __LINE__);
		goto EXIT;
	}

	ret = rpbuf_audio_eq_get_data_name(priv->rpbuf_audio_ctx, priv->eq_in_name, priv->eq_out_name);
	if (ret != 0)
	{
		printf("%s %d rpbuf_audio_eq_get_data_name failed", __func__, __LINE__);
		goto EXIT;
	}


	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_eq_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}

	return ret;
}

int rpbuf_eq_release(rpbuf_eq_ctx_t *priv)
{
	int ret = -1;

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_RELEASE;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_RELEASE);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_eq_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_eq_start(rpbuf_eq_ctx_t *priv)
{
	int ret = -1;

	/* start algorithms */
	memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_START;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_START);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_eq_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_eq_stop(rpbuf_eq_ctx_t *priv)
{
	int ret = -1;

	/* stop algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_eq_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_STOP;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_EQ;
	ret = rpbuf_audio_eq_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_STOP);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_eq_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

static void *rpbuf_eq_transmit_thread(void *params)
{
	rpbuf_eq_ctx_t *priv = (rpbuf_eq_ctx_t *)params;
	int ret = 0;
	void *data = NULL;
	void *recv_data = NULL;
	void *output_data = NULL;
	void *out_data = NULL;
	unsigned long output_size = 0;
	unsigned long size = priv->rpbuf_eq_in_len;
	unsigned long out_size = priv->rpbuf_eq_out_len;
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	//long long start_usec, end_usec;
	//float delta_usec;
	snd_pcm_sframes_t length;
	int timeout = 300;
	unsigned int frame_count = 0;
	int pin_fd = -1;
	rpbuf_audio_remote_ctrl_eq_t rpbuf_eq_ctrl_arg;

	data = malloc(size);
	if (!data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(data, 0, size);

	recv_data = malloc(out_size);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, out_size);

#ifdef RPBUF_DSP_EQ_DUMP_INPUT_DATA
	out_data = malloc(out_size);
	if (!out_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(out_data, 0, out_size);
#endif

#ifdef EQ_DUMP_FILE
	FILE *dumpfile = NULL;
	long write_size = 0;

	dumpfile = fopen(filename, "wb+");
	if (!dumpfile) {
		printf("open file %s error!\n", filename);
		goto exit;
	}
#endif

#ifdef RPBUF_SPEED_TEST

	pin_fd = open(sunix_pin, O_RDWR);
	if (pin_fd == -1) {
		printf("open file %s error!\n", sunix_pin);
		goto exit;
	}
#endif

#ifdef RPBUF_DSP_EQ_TEST

	ret = rpbuf_eq_start(priv);
	if (ret != 0) {
		printf("rpbuf_eq_start %d error!\n", ret );
		goto exit;
	}

	do {
		memset(&rpbuf_eq_ctrl_arg, 0 ,sizeof(rpbuf_audio_remote_ctrl_eq_t));
		ret = rpbuf_audio_eq_ctrl_recv_cmd(priv->rpbuf_audio_ctx, &rpbuf_eq_ctrl_arg, &recv_data_offset, &recv_size, 1);
		if (ret != 0 || (recv_size > sizeof(rpbuf_audio_remote_ctrl_eq_t)))
		{
			if (ret == -ETIMEDOUT)
			{
				printf("wait dsp ack timeout!\n");
				continue;
			}
			else
			{
				printf("rpbuf_eq_start ack %d error!\n", ret);
				goto exit;
			}
		}
	} while (ret);

	if (rpbuf_eq_ctrl_arg.cmd == RPBUF_CTL_START_ACK && rpbuf_eq_ctrl_arg.type == RPBUF_ALGO_EQ && \
		rpbuf_eq_ctrl_arg.value == RPBUF_ERROR_NONE)
	{
		printf("eq has been start finished in dsp\n");
	}
	else
	{
		printf("rpbuf eq get start ack failed cmd %d type %d value %d exit thread\n", rpbuf_eq_ctrl_arg.cmd, rpbuf_eq_ctrl_arg.type, rpbuf_eq_ctrl_arg.value);
		goto stop;
	}

#endif

	while (!in_aborting) {

		length = fread(data, 1, size, priv->infile);
		//printf("read  len %ld\n", length);
		if (length <= 0) {
			printf("read file error rlen %ld!\n", length);
			break;
		}

#ifdef RPBUF_SPEED_TEST
		ret = write(pin_fd, PE4_output_low, strlen(PE4_output_low));
		if (ret != strlen(PE4_output_low)) {
			printf("cmd: %s\t error: %s", PE4_output_low, strerror(errno));
			break;
		}
#endif

#ifdef RPBUF_DSP_EQ_TEST

		/* transfer data to dsp */
		ret = rpbuf_audio_eq_transfer_data(priv->rpbuf_audio_ctx, priv->eq_in_name, data, 0, size);
		if (ret < 0)
		{
			printf("rpbuf_audio_transfer_data error ret %d!\n", ret);
			break;
		}

		//start_usec = get_currenttime();

		memset(data, 0, size);

		ret = rpbuf_audio_eq_recv_data(priv->rpbuf_audio_ctx, priv->eq_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret != 0) {
			printf("rpbuf_audio_recv_data recv size %lu\n", recv_size);
			break;
		}

		if (recv_size != out_size) {
			printf("eq recv data error recv size %ld size %ld\n", recv_size, out_size);
			break;
		}

		//end_usec = get_currenttime();
		//delta_usec = end_usec - start_usec;
		//printf("size %lu, delta ms %f\n", recv_size, delta_usec / 1000);
#endif

#ifdef RPBUF_SPEED_TEST
		ret = write(pin_fd, PE4_output_high, strlen(PE4_output_high));
		if (ret != strlen(PE4_output_high)) {
			printf("cmd: %s\t error: %s", PE4_output_high, strerror(errno));
			break;
		}
#endif

#ifdef RPBUF_DSP_EQ_DUMP_INPUT_DATA
	output_data = recv_data + size;
	output_size = size;
	/* 2ch change to ch0ch0 ...ch0 ch1ch1 ...ch1 ch2ch2 ...ch2 ch3ch3 ...ch3 */
	rpbuf_pcm_s16le_split_to_plane(priv, out_data, recv_data, posthwparams.channels, size);
	rpbuf_pcm_s16le_split_to_plane(priv, out_data + size, recv_data + size, posthwparams.channels, size);
	/* 4ch change to ch0ch1ch2ch3 ... ch0ch1ch2ch3 ... ch0ch1ch2ch3 */
	rpbuf_pcm_s16le_plane_to_split(priv, recv_data, out_data, posthwparams.channels * 2, recv_size);
#else
	output_data = recv_data;
	output_size = recv_size;
#endif

#ifdef EQ_DUMP_FILE
		if (dumpfile) {
			ret = fwrite(recv_data, 1, recv_size, dumpfile);
			if (ret != recv_size)
				printf("[%s] line:%d fwrite err ! ret=%d, data_len=%ld\n", __func__, __LINE__, ret, recv_size);
			write_size += recv_size;
			if (write_size == recv_size * 100) {
				fflush(dumpfile);
				fsync(fileno(dumpfile));
				write_size = 0;
			}
		}
#endif

		frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, output_size);
		ret = audio_write(priv->pcm_dev, output_data, frame_count);
		if (ret < 0) {
			if (ret = -EPIPE)
				printf("audio write underrun, size %lu\n", output_size);
			else {
				printf("audio write err, size %lu\n", output_size);
				break;
			}
		}

	}

stop:

#ifdef RPBUF_DSP_EQ_TEST
	ret = rpbuf_eq_stop(priv);
	if (ret != 0) {
		printf("rpbuf_eq_stop %d error!\n", ret);
		goto exit;
	}
#endif

exit:

#ifdef RPBUF_SPEED_TEST
	if (pin_fd != -1)
		close(pin_fd);
#endif

	if (data)
		free(data);

	data = NULL;

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

#ifdef RPBUF_DSP_EQ_DUMP_INPUT_DATA
	if (out_data)
		free(out_data);

	out_data = NULL;
#endif

#ifdef EQ_DUMP_FILE
	if (dumpfile) {
		fflush(dumpfile);
		fsync(fileno(dumpfile));
		fclose(dumpfile);
		dumpfile = NULL;
	}
#endif

	printf("end of transmit thread\n");
	return NULL;
}

static void *rpbuf_eq_config_thread(void *params)
{
	rpbuf_eq_ctx_t *priv = (rpbuf_eq_ctx_t *)params;
	char buf[64] = {0};
	char path[32] = {0};
	char *ptr = NULL;
	char *str = NULL;

	while (!priv->exit_config) {
		str = fgets(buf, 64, stdin);
		if (str == NULL)
			continue;
		// config eq again.
		ptr = buf;
		while(*ptr++ != ' ' && *ptr != '\n');
		if (!strncmp(buf, "eqconfig ", 9)) {
			strncpy(path, ptr, strlen(ptr) - 1);
			printf("path:%s\n", path);
			rpbuf_eq_config(priv, path);
		}
		else {
			printf("unknow cmd: %s!", buf);
			printf("help: eq config path for config eq again\n");
		}
		memset(buf, 0, 64);
	}
	return NULL;
}

int rpbuf_post_eq_playback(char *filename)
{
	int ret = 0;
	int i = 0;
	rpbuf_eq_ctx_t *priv = NULL;
	pthread_t thread_tran = 0;
	pthread_t thread_config = 0;
	struct WAVE_FORMAT_DEF *wave_fmt = NULL;
	pcm_config_t pcm_config = {0};
	int frame_bytes = 0;

	printf("rpbuf_post_eq_playback enter\n");

	priv = malloc(sizeof(rpbuf_eq_ctx_t));
	if (priv) {
		memset(priv, 0 , sizeof(rpbuf_eq_ctx_t));
	} else {
		printf("malloc rpbuf_aec_ctx_t failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->rpbuf_ctl_arg = malloc(sizeof(rpbuf_audio_remote_ctrl_eq_t));
	if (priv->rpbuf_ctl_arg) {
		memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_eq_t));
	} else {
		printf("malloc rpbuf_audio_remote_ctrl_eq_t failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->infile = fopen(filename, "rb");
	if (!priv->infile) {
		printf("open file error!\n");
		goto EXIT;
	}

	if (post_file_type == POST_EQ_FORMAT_WAVE) {
		if (fread(&(priv->audio_info.header),
			sizeof(struct RIFF_HEADER_DEF), 1, priv->infile) != 1) {
			printf("read header info error");
			goto EXIT;
		}

		if (fread(&(priv->audio_info.fmt_block),
			sizeof(struct FMT_BLOCK_DEF), 1, priv->infile) != 1) {
			printf("read fmt_block info error");
			goto EXIT;
		}

		if (fread(&(priv->audio_info.data_block),
			sizeof(struct DATA_BLOCK_DEF), 1, priv->infile) != 1) {
			printf("read data_block info error");
			goto EXIT;
		}
		wave_fmt = &priv->audio_info.fmt_block.wav_format;
	}


	printf("here\n");

	if (wave_fmt) {
		ret = to_pcm_fmt(&priv->audio_info);
		if (ret != 0) {
			printf("to_pcm_fmt failed\n");
			goto EXIT;
		}
		posthwparams.channels = wave_fmt->channels;
		posthwparams.rate = wave_fmt->samples_per_sec;
	}

	frame_bytes = snd_pcm_format_physical_width(posthwparams.format) / 8 * posthwparams.channels;

	priv->rpbuf_eq_in_len = (frame_bytes * posthwparams.rate * posthwparams.msec) / 1000; /* 12 * 16 * 16 = 3072 bytes */

#ifdef RPBUF_DSP_EQ_DUMP_INPUT_DATA
	priv->rpbuf_eq_out_len = (priv->rpbuf_eq_in_len / posthwparams.channels) * \
						(posthwparams.channels + posthwparams.channels); // (3072/ 6)  * 12 (6ch + 6ch) = 6144 byte
#else
	priv->rpbuf_eq_out_len = (priv->rpbuf_eq_in_len / posthwparams.channels) * (posthwparams.channels); // (3072/ 6) * 6 (6ch eq)= 3072 byte
#endif

	printf("rpbuf_eq_in_len %d out %d\n", priv->rpbuf_eq_in_len, priv->rpbuf_eq_out_len);

	/* 1. open_audio */

	pcm_config.channels = posthwparams.channels;
	pcm_config.format = posthwparams.format;
	pcm_config.rate = posthwparams.rate;
	pcm_config.period_size = RPBUF_AUDIO_POST_EQ_DEFAULT_SIZE;
	pcm_config.period_count = RPBUF_DEFAULT_POST_EQ_BUF_CNT;

	priv->pcm_dev = open_audio(pcm_name, post_stream, &pcm_config);
	if (!priv->pcm_dev) {
		printf("open %s audio fail, stream %d!\n", pcm_name, post_stream);
		goto EXIT;
	}

#ifdef RPBUF_DSP_EQ_TEST

	priv->rpbuf_eq_index = g_rpbuf_index;

	/* init */
	ret = rpbuf_eq_init(priv);
	if (ret != 0) {
		printf("rpbuf_eq_init %d failed\n", ret);
		goto EXIT;
	}

	/* set eq param */
	ret = rpbuf_eq_config(priv, NULL);
	if (ret != 0) {
		printf("rpbuf_eq_config %d failed\n", ret);
		goto EXIT;
	}

	//usleep(1000 * 1000);

#endif

	/* start transmit thread*/

	pthread_create(&thread_tran,
				NULL,
				rpbuf_eq_transmit_thread,
				(void *)priv);

	pthread_setname_np(thread_tran, "rpbuf_transmit_thread");

	if (g_config_eq) {
		pthread_create(&thread_config,
					NULL,
					rpbuf_eq_config_thread,
					(void *)priv);

		pthread_setname_np(thread_config, "rpbuf_eq_config_thread");

	}

	/* end the wavread thread */
	pthread_join(thread_tran, NULL);
	thread_tran = 0;

	if (g_config_eq) {
		priv->exit_config = 1;
		pthread_join(thread_config, NULL);
	}


#ifdef RPBUF_DSP_EQ_TEST
	ret = rpbuf_eq_release(priv);
	if (ret != 0) {
		printf("rpbuf_eq_release %d failed\n", ret);
		goto EXIT;
	}
#endif

EXIT:

	printf("post equal playback exit\n");
	if (thread_tran)
		pthread_join(thread_tran, NULL);

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_eq_data_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}

	if (priv->infile) {
		fclose(priv->infile);
		priv->infile = NULL;
	}

	if (priv->pcm_dev) {

		ret = close_audio(priv->pcm_dev, post_stream);
		if (ret != 0) {
			printf("close_audio fail\n");
		}
		priv->pcm_dev = NULL;

		printf("close_audio exit\n");
	}

	if (priv->rpbuf_ctl_arg) {
		free(priv->rpbuf_ctl_arg);
		priv->rpbuf_ctl_arg = NULL;
	}

	if (priv) {
		free(priv);
		priv = NULL;
	}
	return ret;
}

int rpbuf_post_eq_capture(char *filename)
{
	printf("rpbuf_post_eq_capture\n");
	return 0;
}

static void rpbuf_eq_print_help_msg(void)
{
	printf("\n");
	printf("USAGE:\n");
	printf("  rpbuf_eq_demo [OPTIONS]\n");
	printf("\n");
	printf("OPTIONS:\n");
	printf("  -D          : select PCM by name\n");
	printf("  -t          : sfile type (voc, wav, raw or au)\n");
	printf("  -c          : channels\n");
	printf("  -f          : sample format (case insensitive)\n");
	printf("  -r          : sample rate\n");
	printf("  -s          : eq process per mseconds(ms)\n");
	printf("  -P          : PlayBack\n");
	printf("  -C          : Capture\n");
	printf("\n");
	printf("e.g.\n");
	printf("  rpbuf_eq_demo -DPlaybackEQ -s 16 -t wav -P /data/test.wav \n");
	printf("  rpbuf_eq_demo -DPlaybackEQ -s 16 -c 2 -r 16000 -f S16_LE -P /data/test.pcm \n");
	printf("\n");
}

static void signal_handler(int sig)
{
	if (in_aborting)
		return;

	in_aborting = 1;

	signal(sig, SIG_DFL);
}


int main(int argc, char **argv)
{
	int ret = 0;
	int c;
	int tmp;
	long long_tmp;

	if (argc <= 1) {
		rpbuf_eq_print_help_msg();
		ret = -1;
		goto out;
	}

	g_rpbuf_index = 0;
	g_config_eq = 0;
	post_file_type = POST_EQ_FORMAT_DEFAULT;
	rposthwparams.format = POST_EQ_DEFAULT_FORMAT;
	rposthwparams.rate = POST_EQ_DEFAULT_RATE;
	rposthwparams.channels = POST_EQ_DEFAULT_CHANNEL;
	rposthwparams.msec = POST_EQ_DEFAULT_MSEC;

	while ((c = getopt(argc, argv, "hD:t:c:f:r:s:i:n:PC")) != -1) {
		switch (c) {
		case 'h':
			rpbuf_eq_print_help_msg();
			ret = 0;
			goto out;
		case 'D':
			pcm_name = optarg;
			break;
		case 't':
			if (strcasecmp(optarg, "raw") == 0)
				post_file_type = POST_EQ_FORMAT_RAW;
			else if (strcasecmp(optarg, "voc") == 0)
				post_file_type = POST_EQ_FORMAT_VOC;
			else if (strcasecmp(optarg, "wav") == 0)
				post_file_type = POST_EQ_FORMAT_WAVE;
			else {
				printf("unrecognized file format %s", optarg);
				return 1;
			}
			break;
		case 'c':
			if (rposthwparams.channels < 1 || rposthwparams.channels > 256) {
				printf("value %u for channels is invalid", rposthwparams.channels);
				return 1;
			}
			rposthwparams.channels = strtol(optarg, NULL, 0);
			break;
		case 'f':
			if (rposthwparams.format == SND_PCM_FORMAT_UNKNOWN) {
				printf("wrong extended format '%s'", optarg);
				return 1;
			}
			rposthwparams.format = snd_pcm_format_value(optarg);
			break;
		case 'r':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 300)
				tmp *= 1000;
			if (tmp < 2000 || tmp > 192000) {
				printf("bad speed value %d", tmp);
				return 1;
			}
			rposthwparams.rate = tmp;
			break;
		case 's':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			rposthwparams.msec = tmp;
			break;
		case 'i':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			g_rpbuf_index = tmp;
			break;
		case 'n':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0) {
				printf("bad time value %d", tmp);
				return 1;
			}
			g_config_eq = tmp;
			break;
		case 'P':
			post_stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'C':
			post_stream = SND_PCM_STREAM_CAPTURE;
			break;
		default:
			printf("Invalid option: -%c\n", c);
			rpbuf_eq_print_help_msg();
			ret = -1;
			goto out;
		}
	}

	if (optind > argc - 1) {
		printf("Please enter the filename\n");
		ret = -1;
		goto out;
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);

	posthwparams = rposthwparams;

	if (post_stream == SND_PCM_STREAM_PLAYBACK) {
		rpbuf_post_eq_playback(argv[optind++]);
	} else {
		rpbuf_post_eq_capture(argv[optind++]);
	}


out:
	return ret;

}




