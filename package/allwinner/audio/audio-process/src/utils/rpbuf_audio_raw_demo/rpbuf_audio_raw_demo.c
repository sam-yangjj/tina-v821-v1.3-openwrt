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
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "rpbuf_audio_ctrl_raw.h"

#include "rpbuf_audio_raw.h"
#include "audio_common.h"
#include "thread_sem.h"

#define AUDIO_RPBUF_AUDIO_DEFAULT_SIZE 1024
#define DEFAULT_RPBUF_AUDIO_BUF_CNT 4

#define RPBUF_AUDIO_PCM_NAME_DEFAULT "default"

#define RPBUF_AUDIO_PLAYBACK_LENGTH_DEFAULT 1024//byte

#define RPBUF_AUDIO_OUT_CHANNEL 1

#define RPBUF_AUDIO_DEFAULT_FORMAT		SND_PCM_FORMAT_S16_LE
#define RPBUF_AUDIO_DEFAULT_SPEED 		16000
#define RPBUF_AUDIO_DEFAULT_CHANNEL 		2
#define RPBUF_AUDIO_DEFAULT_PERIOD_SIZE 	320
#define RPBUF_AUDIO_DEFAULT_BUFFER_SIZE 	1280

#define RPBUF_AUDIO_DEFAULT_MSEC 			64

#define RPBUF_AUDIO_FORMAT_DEFAULT	-1
#define RPBUF_AUDIO_FORMAT_RAW		0
#define RPBUF_AUDIO_FORMAT_VOC		1
#define RPBUF_AUDIO_FORMAT_WAVE		2


/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092

//#define RPBUF_DSP_AUDIO_DUMP_INPUT_DATA
#define SLEEP_MS 100

#define AUDIO_LOWPOWER_PROCESS_EXIT_BIT		(0x1<<0)
#define AUDIO_WAIT_MODE_PROCESS_EXIT_BIT	(0x2<<0)

#define AUDIO_BYTE_ALIGNMENT 32

#define AUDIO_ALIGNMENT_MASK 0x1f

struct hwparams{
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
	unsigned int period_size;
	unsigned int buffer_size;
	unsigned int msec;
};

static struct hwparams prehwparams, rprehwparams;

static snd_pcm_stream_t rpbuf_stream = SND_PCM_STREAM_CAPTURE;
static int rpbuf_file_type = RPBUF_AUDIO_FORMAT_DEFAULT;
static int rpbuf_mode = 0;
static long rpbuf_timelimit = 0;
static int time_sec = -1;
static int g_rpbuf_raw_index = 0;
static int g_rpbuf_card_num = 0;
static int repeat_cnt = 0;
static char *rpbuf_pcm_name = RPBUF_AUDIO_PCM_NAME_DEFAULT;
static volatile sig_atomic_t in_stopping = 0;
static uint64_t rpbuf_rec_count = LLONG_MAX;

#ifdef RPBUF_SPEED_TEST
static char PE5_output_low[32]  = "PE5 1 0 1 0";
static char PE5_output_high[32] = "PE5 1 1 1 0";
static char PE4_output_low[32]  = "PE4 1 0 1 0";
static char PE4_output_high[32] = "PE4 1 1 1 0";
static char sunix_pin[64] = "/sys/kernel/debug/sunxi_pinctrl/sunxi_pin_configure";
#endif


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
	char data_id[4];     /* 'd', 'a', 't', 'a' */
	uint32_t data_size;
};

struct wav_info {
	struct RIFF_HEADER_DEF header;
	struct FMT_BLOCK_DEF   fmt_block;
	struct DATA_BLOCK_DEF  data_block;
};
/*
typedef struct bufferheader
{
	void *recv_data;
	int use_tag;
	unsigned long recv_size;
} bufferheader;
*/

typedef struct rpbuf_audioraw_ctx {
	snd_pcm_t *pcm_dev;
	rpbuf_audio_raw_ctx_t *rpbuf_audio_ctx;
	rpbuf_audio_remote_ctrl_raw_t *rpbuf_ctl_arg;
	FILE *playfile;
	FILE *infile;
	int pin_fd;
	int rpbuf_raw_index;
	char raw_in_name[32];
	char raw_out_name[32];
	uint32_t lowpower_thread_exit;
	uint32_t mode;
	pthread_mutex_t mode_mutex;
	thread_sem_t thread_wait_normal_mode;
	thread_sem_t thread_wait_lowpower_mode;
	unsigned int rpbuf_audio_in_len;
	unsigned int rpbuf_audio_out_len;
	unsigned int rpbuf_len;
	struct wav_info audio_info;
}rpbuf_audioraw_ctx_t;

static long long get_currenttime()
{
	struct timeval time;

	gettimeofday(&time, NULL);

	return ((long long)time.tv_sec * 1000000 + (long long)time.tv_usec);
}

static int rpbuf_to_pcm_fmt(struct wav_info *wav_info)
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
			if (prehwparams.format != RPBUF_AUDIO_DEFAULT_FORMAT &&
				prehwparams.format != SND_PCM_FORMAT_U8)
				printf("Warning: format is changed to U8\n");
			prehwparams.format = SND_PCM_FORMAT_U8;
			break;
		case 16:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S16_BE;
			else
				native_format = SND_PCM_FORMAT_S16_LE;
			if (prehwparams.format != RPBUF_AUDIO_DEFAULT_FORMAT &&
				prehwparams.format != native_format)
				printf("Warning: format is changed to %s\n", \
					snd_pcm_format_name(native_format));
			prehwparams.format = native_format;
			break;
		case 24:
			switch (bit / prehwparams.channels) {
			case 3:
				if (big_endian)
					native_format = SND_PCM_FORMAT_S24_3BE;
				else
					native_format = SND_PCM_FORMAT_S24_3LE;
				if (prehwparams.format != RPBUF_AUDIO_DEFAULT_FORMAT &&
					prehwparams.format != native_format)
					printf("Warning: format is changed to %s\n", \
						snd_pcm_format_name(native_format));
				prehwparams.format = native_format;
				break;
			case 4:
				if (big_endian)
					native_format = SND_PCM_FORMAT_S24_BE;
				else
					native_format = SND_PCM_FORMAT_S24_LE;
				if (prehwparams.format != RPBUF_AUDIO_DEFAULT_FORMAT &&
					prehwparams.format != native_format)
					printf("Warning: format is changed to %s\n", \
						snd_pcm_format_name(native_format));
				prehwparams.format = native_format;
				break;
			default:
				printf(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)", \
					  wav_info->fmt_block.wav_format.bits_per_sample,
					  wav_info->fmt_block.wav_format.block_align,
					  prehwparams.channels);
				ret = -1;
			}
			break;
		case 32:
			if (format == WAV_FMT_PCM) {
				if (big_endian)
					native_format = SND_PCM_FORMAT_S32_BE;
				else
					native_format = SND_PCM_FORMAT_S32_LE;
							prehwparams.format = native_format;
			} else if (format == WAV_FMT_IEEE_FLOAT) {
				if (big_endian)
					native_format = SND_PCM_FORMAT_FLOAT_BE;
				else
					native_format = SND_PCM_FORMAT_FLOAT_LE;
				prehwparams.format = native_format;
			}
			break;
		default:
			printf(" can't play WAVE-files with sample %d bits wide", bit);
			ret = -1;
		}

	return ret;
}

/* calculate the data count to read from/to dsp */
static uint64_t calc_data_count(void)
{
	uint64_t count;

	if (rpbuf_timelimit == 0) {
		count = rpbuf_rec_count;
	}
	else {
		count = snd_pcm_format_size(rprehwparams.format, rprehwparams.rate * rprehwparams.channels);
		count *= (uint64_t)rpbuf_timelimit;
	}
	return count < rpbuf_rec_count ? count : rpbuf_rec_count;
}

/*
 * make sure we write all bytes or return an error
 */
static ssize_t writefile(FILE *file, const void *buf, size_t count)
{
	ssize_t written;
	size_t offset = 0;

	while (offset < count) {
		written = fwrite((char *)buf + offset, 1, count - offset, file);
		if (written <= 0)
			return written;

		offset += written;
	}

	return offset;
}

/* write a WAVE-header */
static int start_wave_header(FILE *file, size_t cnt , struct hwparams *rprehwparams)
{
	struct RIFF_HEADER_DEF head;
	struct FMT_BLOCK_DEF f;
	struct DATA_BLOCK_DEF cd;
	int bits;
	u_int tmp;
	u_short tmp2;

	/* WAVE cannot handle greater than 32bit (signed?) int */
	if (cnt == (size_t)-2)
		cnt = 0x7fffff00;

	bits = 8;
	switch ((unsigned long) rprehwparams->format) {
	case SND_PCM_FORMAT_U8:
		bits = 8;
		break;
	case SND_PCM_FORMAT_S16_LE:
		bits = 16;
		break;
	case SND_PCM_FORMAT_S32_LE:
        case SND_PCM_FORMAT_FLOAT_LE:
		bits = 32;
		break;
	case SND_PCM_FORMAT_S24_LE:
	case SND_PCM_FORMAT_S24_3LE:
		bits = 24;
		break;
	default:
		printf("Wave doesn't support %s format...", snd_pcm_format_name(rprehwparams->format));
	}

	strncpy(head.riff_id, "RIFF", sizeof(head.riff_id));
	tmp = cnt + sizeof(struct RIFF_HEADER_DEF) + sizeof(struct FMT_BLOCK_DEF) + sizeof(struct DATA_BLOCK_DEF) - 8;
	head.riff_size = tmp;
	strncpy(head.riff_format, "WAVE", sizeof(head.riff_format));

	strncpy(f.fmt_id, "fmt ", sizeof(f.fmt_id));
	f.fmt_size = 16;

    if (rprehwparams->format == SND_PCM_FORMAT_FLOAT_LE)
        f.wav_format.format_tag = WAV_FMT_IEEE_FLOAT;
    else
        f.wav_format.format_tag = WAV_FMT_PCM;

	f.wav_format.channels = rprehwparams->channels;
	f.wav_format.samples_per_sec = rprehwparams->rate;

	tmp2 = rprehwparams->channels * snd_pcm_format_physical_width(rprehwparams->format) / 8;
	f.wav_format.block_align = tmp2;
	tmp = (u_int) tmp2 * rprehwparams->rate;

	f.wav_format.avg_bytes_per_sec = tmp;
	f.wav_format.bits_per_sample = bits;

	strncpy(cd.data_id, "data", sizeof(cd.data_id));
	cd.data_size = cnt;

	if (writefile(file, &head, sizeof(struct RIFF_HEADER_DEF)) != sizeof(struct RIFF_HEADER_DEF) ||
	    writefile(file, &f, sizeof(struct FMT_BLOCK_DEF)) != sizeof(struct FMT_BLOCK_DEF) ||
	    writefile(file, &cd, sizeof(struct DATA_BLOCK_DEF)) != sizeof(struct DATA_BLOCK_DEF)) {
		printf("write error\n");
		return -1;
	}
	return 0;
}

static void end_wave(FILE *file, uint64_t fdcount)
{
	struct DATA_BLOCK_DEF cd;
	uint64_t length_seek;
	uint64_t filelen;
	u_int rifflen;

	length_seek = sizeof(struct RIFF_HEADER_DEF) +
		      sizeof(struct FMT_BLOCK_DEF);

	strncpy(cd.data_id, "data", sizeof(cd.data_id));
	cd.data_size = fdcount > 0x7fffffff ? 0x7fffffff : fdcount;


	filelen = fdcount + sizeof(struct DATA_BLOCK_DEF) + sizeof(struct FMT_BLOCK_DEF) + 4;

	rifflen = filelen > 0x7fffffff ? 0x7fffffff : filelen;

	if (fseek(file, 4, SEEK_SET) == 4)
		writefile(file, &rifflen, 4);
	if (fseek(file, length_seek, SEEK_SET) == length_seek)
		writefile(file, &cd, sizeof(struct DATA_BLOCK_DEF));

	fseek(file, 0, SEEK_END);

}

int rpbuf_audio_record_config(rpbuf_audioraw_ctx_t *priv)
{
	int ret = -1;

	/* config algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_CONFIG;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;

	priv->rpbuf_ctl_arg->channels = rprehwparams.channels;
	priv->rpbuf_ctl_arg->bits = snd_pcm_format_physical_width(rprehwparams.format);
	priv->rpbuf_ctl_arg->rate = rprehwparams.rate;
	priv->rpbuf_ctl_arg->period_size = rprehwparams.period_size;
	priv->rpbuf_ctl_arg->buffer_size = rprehwparams.buffer_size;
	priv->rpbuf_ctl_arg->msec = rprehwparams.msec;
	priv->rpbuf_ctl_arg->value = g_rpbuf_card_num;  /* config card number */

	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_CONFIG);
		return ret;
	}

	printf("rpbuf audio raw config cmd %d chan:%d bit:%d rate:%d period_size:%d buffer_size:%d msec:%d\n", \
			priv->rpbuf_ctl_arg->cmd, priv->rpbuf_ctl_arg->channels, priv->rpbuf_ctl_arg->bits, \
			priv->rpbuf_ctl_arg->rate, priv->rpbuf_ctl_arg->period_size, priv->rpbuf_ctl_arg->buffer_size, priv->rpbuf_ctl_arg->msec);

	return 0;
}

int rpbuf_audio_record_init(rpbuf_audioraw_ctx_t *priv)
{
	int ret = -1;

	/* init rpbuf audio */
	priv->rpbuf_audio_ctx = rpbuf_audio_raw_init(priv->rpbuf_raw_index);
	if (priv->rpbuf_audio_ctx == NULL) {
		printf("rpbuf_audio_init failed\n");
		goto EXIT;
	}

#ifdef RPBUF_DSP_AUDIO_DUMP_INPUT_DATA
	/* enable raw data dump */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_DUMP_MERGE_DATA;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	priv->rpbuf_ctl_arg->value = 1;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_DUMP_MERGE_DATA);
		goto EXIT;
	}
#endif

	//usleep(5000);

	/* enable init */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_INIT;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_INIT);
		goto EXIT;
	}
	printf("rpbuf_audio_init send cmd %u\n", priv->rpbuf_ctl_arg->cmd);

	//usleep(5000);

	ret = rpbuf_audio_raw_record_data_init(priv->rpbuf_audio_ctx, RPBUF_RAW_OUT_LENGTH);
	if (ret != 0) {
		printf("rpbuf_audio_raw_record_data_init failed\n");
		goto EXIT;
	}

	ret = rpbuf_audio_raw_get_data_name(priv->rpbuf_audio_ctx, NULL, priv->raw_out_name);
	if (ret != 0)
	{
		printf("%s %d rpbuf_audio_raw_get_data_name failed", __func__, __LINE__);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_audio_record_release(rpbuf_audioraw_ctx_t *priv)
{
	int ret = -1;

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_RELEASE;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_RELEASE);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_audio_record_start(rpbuf_audioraw_ctx_t *priv)
{
	int ret = -1;

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_START;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_START);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_audio_record_stop(rpbuf_audioraw_ctx_t *priv)
{
	int ret = -1;

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_STOP;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_STOP);
		goto EXIT;
	}

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

int rpbuf_audio_record_lowpower(rpbuf_audioraw_ctx_t *priv, uint32_t value)
{
	int ret = -1;

	/* enable whole algorithms */
	memset(priv->rpbuf_ctl_arg, 0 , sizeof(rpbuf_audio_remote_ctrl_raw_t));
	priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_LOW_POWER_MODE;
	priv->rpbuf_ctl_arg->type = RPBUF_ALGO_RECORD;
	priv->rpbuf_ctl_arg->value = value;
	ret = rpbuf_audio_raw_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
	if (ret != 0) {
		printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_LOW_POWER_MODE);
		goto EXIT;
	}

	pthread_mutex_lock(&priv->mode_mutex);
	priv->mode = RPBUF_AUDIO_DSP_RECORD_MODE_LOW_POWER;
	pthread_mutex_unlock(&priv->mode_mutex);

	return 0;

EXIT:

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}
	return ret;
}

static int pcm_s16le_plane_to_split(short *dst, short *src, struct hwparams *hw_params, unsigned int chan, unsigned int len)
{
	uint32_t rate = 0;
	uint32_t channels = 0;
	uint8_t bitwidth = 0;
	uint32_t BytesPerSample = 0;
	short nSampleCount = 0;
	int i = 0;
	int j = 0;

	channels = chan;
	bitwidth = snd_pcm_format_physical_width(hw_params->format);
	rate     = hw_params->rate;

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

static void *rpbuf_audio_record_recv_thread(void *params)
{
	rpbuf_audioraw_ctx_t *priv = (rpbuf_audioraw_ctx_t *)params;
	int ret = 0;
	//long write_size = 0;
	uint64_t count = 0;
	uint64_t fdcount = 0;
	struct hwparams prehwparams;
	void *recv_data = NULL;
	unsigned long size = priv->rpbuf_audio_out_len;
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	int timeout = 500;
	//long long start_usec, end_usec;
	//float delta_usec;
	int64_t capture_count = 0;
	rpbuf_audio_remote_ctrl_raw_t rpbuf_raw_ctrl_arg;

	count = calc_data_count();
	if (count == 0)
		count = LLONG_MAX;

	if (count < LLONG_MAX)
			count += count % 2;
		else
			count -= count % 2;

	capture_count = count;

	printf("count %lu\n", count);

	if (rpbuf_file_type == RPBUF_AUDIO_FORMAT_WAVE) {
		/* aec in has two channels, left channel is mic in ,right channel is ref, but aec out only have one channel. */
		prehwparams.format = rprehwparams.format;
#ifdef RPBUF_DSP_AUDIO_DUMP_INPUT_DATA
		prehwparams.channels = rprehwparams.channels + RPBUF_AUDIO_OUT_CHANNEL;
#else
		//prehwparams.channels = RPBUF_AUDIO_OUT_CHANNEL;
		prehwparams.channels = rprehwparams.channels;
#endif
		prehwparams.rate =  rprehwparams.rate;

		start_wave_header(priv->infile, count, &prehwparams);
	}

	recv_data = malloc(size);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, size);

	ret = rpbuf_audio_record_start(priv);
	if (ret != 0) {
		printf("rpbuf_audioraw_start %d error!\n", ret);
		goto exit;
	}

	do {
		memset(&rpbuf_raw_ctrl_arg, 0 ,sizeof(rpbuf_audio_remote_ctrl_raw_t));
		ret = rpbuf_audio_raw_ctrl_recv_cmd(priv->rpbuf_audio_ctx, &rpbuf_raw_ctrl_arg, &recv_data_offset, &recv_size, 1);
		if (ret != 0 || (recv_size > sizeof(rpbuf_audio_remote_ctrl_raw_t)))
		{
			if (ret == -ETIMEDOUT)
			{
				printf("wait remote ack timeout!\n");
				continue;
			}
			else
			{
				printf("rpbuf_raw_start ack %d error!\n", ret);
				goto exit;
			}
		}
	} while (ret);

	if (rpbuf_raw_ctrl_arg.cmd == RPBUF_CTL_START_ACK && rpbuf_raw_ctrl_arg.type == RPBUF_ALGO_RECORD && \
		rpbuf_raw_ctrl_arg.value == RPBUF_ERROR_NONE)
	{
		printf("remote record has been start finished in dsp\n");
	}
	else
	{
		printf("rpbuf remote record get start ack failed cmd %d type %d value %d exit thread\n", \
			rpbuf_raw_ctrl_arg.cmd, rpbuf_raw_ctrl_arg.type, rpbuf_raw_ctrl_arg.value);
		goto stop;
	}

	while (capture_count > 0 && !in_stopping) {

		if (!rpbuf_audio_raw_is_available(priv->rpbuf_audio_ctx, priv->raw_out_name)) {
			printf("exit audio record recv loop!\n");
			break;
		}

#ifdef RPBUF_SPEED_TEST
		ret = write(priv->pin_fd, PE5_output_low, strlen(PE5_output_low));
		if (ret != strlen(PE5_output_low)) {
			printf("cmd: %s\t error: %s", PE5_output_low, strerror(errno));
			break;
		}
#endif
		//start_usec = get_currenttime();
		//printf("rpbuf fdcount %llu\n", fdcount);

		ret = rpbuf_audio_raw_recv_data(priv->rpbuf_audio_ctx, priv->raw_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret != 0) {
			printf("rpbuf aec recv size %lu\n", recv_size);
			break;
		}

		if (recv_size != size) {
			printf("audio recv data error recv size %ld size %ld\n", recv_size, size);
			break;
		}


#ifdef RPBUF_SPEED_TEST
		ret = write(priv->pin_fd, PE5_output_high, strlen(PE5_output_high));
		if (ret != strlen(PE5_output_high)) {
			printf("cmd: %s\t error: %s", PE5_output_high, strerror(errno));
			break;
		}
#endif
		//end_usec = get_currenttime();
		//delta_usec = end_usec - start_usec;

		//pcm_s16le_plane_to_split(priv, out_data, recv_data, recv_size);

		if (writefile(priv->infile, recv_data, recv_size) != recv_size) {
			printf("write size %lu error!\n", recv_size);
			break;
		}

		memset(recv_data, 0, recv_size);

		fdcount += recv_size;
		capture_count -= recv_size;

/*
		write_size += recv_size;
		if (write_size == recv_size * 30) {
			fflush(priv->infile);
			fsync(fileno(priv->infile));
			write_size = 0;
		}
*/
		//printf("write size %lu, cnt %llu, delta msec %f\n", recv_size, fdcount ,delta_usec / 1000);

	}

stop:

	ret = rpbuf_audio_record_stop(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_stop %d error!\n", ret);
		goto exit;
	}

exit:

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

	if (rpbuf_file_type == RPBUF_AUDIO_FORMAT_WAVE) {
		printf("fdcount %lu\n", fdcount);
		end_wave(priv->infile, fdcount);
	}

	if (priv->infile) {
		fflush(priv->infile);
		fsync(fileno(priv->infile));
		fclose(priv->infile);
		priv->infile = NULL;
	}

	printf("end of recv thread\n");
	return NULL;
}

int rpbuf_audioraw_capture(char *filename)
{
	int ret = 0;
	rpbuf_audioraw_ctx_t *priv = NULL;
	pthread_t thread_recv = 0;
	int frame_bytes = 0;

	printf("audio raw capture enter\n");

	priv = malloc(sizeof(rpbuf_audioraw_ctx_t));
	if (priv) {
		memset(priv, 0, sizeof(rpbuf_audioraw_ctx_t));
	} else {
		printf("malloc rpbuf_audio_capture failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->rpbuf_ctl_arg = malloc(sizeof(rpbuf_audio_remote_ctrl_raw_t));
	if (priv->rpbuf_ctl_arg) {
		memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_raw_t));
	} else {
		printf("malloc rpbuf_audio_capture failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->infile = fopen(filename, "wb+");
	if (!priv->infile) {
		printf("open file %s error!\n", filename);
		goto EXIT;
	}

#ifdef RPBUF_SPEED_TEST
	priv->pin_fd = open(sunix_pin, O_RDWR);
	if (priv->pin_fd == -1) {
		printf("open file %s error!\n", sunix_pin);
		goto EXIT;
	}
#endif

	printf("here\n");

	frame_bytes = snd_pcm_format_physical_width(rprehwparams.format) / 8 * rprehwparams.channels;

	priv->rpbuf_audio_in_len = (frame_bytes * rprehwparams.rate * rprehwparams.msec) / 1000; /* 4 * 16 * 64 = 4096 bytes */

	if (priv->rpbuf_audio_in_len & AUDIO_ALIGNMENT_MASK) {
		if ((priv->rpbuf_audio_in_len + (AUDIO_BYTE_ALIGNMENT - (priv->rpbuf_audio_in_len & AUDIO_ALIGNMENT_MASK))) > priv->rpbuf_audio_in_len)
		{
			priv->rpbuf_audio_in_len += (AUDIO_BYTE_ALIGNMENT - (priv->rpbuf_audio_in_len & AUDIO_ALIGNMENT_MASK));
		} else {
			printf("%d is can't align %d \n", priv->rpbuf_audio_in_len, AUDIO_BYTE_ALIGNMENT);
			goto EXIT;
		}
	}

	priv->rpbuf_len = priv->rpbuf_audio_in_len;
	if (priv->rpbuf_len + sizeof(rpbuf_audio_remote_ctrl_raw_t) > RPBUF_LEN_MAX){
		printf("rpbuf_size %ld is larger than MAX %d\n", priv->rpbuf_len + sizeof(rpbuf_audio_remote_ctrl_raw_t), RPBUF_LEN_MAX);
		goto EXIT;
	}

#ifdef RPBUF_DSP_AUDIO_DUMP_INPUT_DATA
	priv->rpbuf_audio_out_len = (priv->rpbuf_audio_in_len / rprehwparams.channels) * \
						(rprehwparams.channels + RPBUF_AUDIO_OUT_CHANNEL); // (4096/ 2)  * 3 (1ch mic+ 1ch ref + 1ch aec) = 6144 byte
#else
	//priv->rpbuf_audio_out_len = (priv->rpbuf_audio_in_len / rprehwparams.channels) * (RPBUF_AUDIO_OUT_CHANNEL); // (4096/ 2)  * 1 (1ch aec)= 2048 byte
	priv->rpbuf_audio_out_len = priv->rpbuf_audio_in_len;
#endif

	if (priv->rpbuf_audio_out_len & AUDIO_ALIGNMENT_MASK) {
		if ((priv->rpbuf_audio_out_len + (AUDIO_BYTE_ALIGNMENT - (priv->rpbuf_audio_out_len & AUDIO_ALIGNMENT_MASK))) > priv->rpbuf_audio_out_len)
		{
			priv->rpbuf_audio_out_len += (AUDIO_BYTE_ALIGNMENT - (priv->rpbuf_audio_out_len & AUDIO_ALIGNMENT_MASK));
		} else {
			printf("%d is can't align %d \n", priv->rpbuf_audio_out_len, AUDIO_BYTE_ALIGNMENT);
			goto EXIT;
		}
	}

	printf("rpbuf_audio_in_len %u out %u rpbuf_len %u \n", priv->rpbuf_audio_in_len, priv->rpbuf_audio_out_len, priv->rpbuf_len);

	priv->rpbuf_raw_index = g_rpbuf_raw_index;

	/* init */
	ret = rpbuf_audio_record_init(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_init %d failed\n", ret);
		goto EXIT;
	}

	/* set record param */
	ret = rpbuf_audio_record_config(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_config %d failed\n", ret);
		goto EXIT;
	}

	//usleep(300 * 1000);

	/* start recv thread */
	pthread_create(&thread_recv,
				NULL,
				rpbuf_audio_record_recv_thread,
				(void *)priv);

	pthread_setname_np(thread_recv, "rpbuf_audio_record_recv_thread");

	/* end the recv thread */

	if (thread_recv)
		pthread_join(thread_recv, NULL);

	thread_recv = 0;

	ret = rpbuf_audio_record_release(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_release %d failed\n", ret);
		goto EXIT;
	}

EXIT:

	printf("rpbuf audio raw capture exit\n");

	if (thread_recv)
		pthread_join(thread_recv, NULL);

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}

#ifdef RPBUF_SPEED_TEST
	if (priv->pin_fd != -1) {
		close(priv->pin_fd);
		priv->pin_fd = 0;
	}
#endif

	if (priv->infile) {
		fflush(priv->infile);
		fsync(fileno(priv->infile));
		fclose(priv->infile);
		priv->infile = NULL;
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

static int rpbuf_audioraw_playback(char *filename)
{
	int ret = 0;
	rpbuf_audioraw_ctx_t *priv = NULL;
	struct WAVE_FORMAT_DEF *wave_fmt = NULL;
	pcm_config_t pcm_config = {0};
	void *data = NULL;
	unsigned long size = RPBUF_AUDIO_PLAYBACK_LENGTH_DEFAULT;
	long length = 0;
	unsigned long frame_count = 0;
	uint64_t playback_count = 0;
	uint64_t written = 0;
	uint64_t c = 0;
	uint64_t r = 0;

	printf("rpbuf_audio_playback enter\n");

	priv = malloc(sizeof(rpbuf_audioraw_ctx_t));
	if (priv) {
		memset(priv, 0 , sizeof(rpbuf_audioraw_ctx_t));
	} else {
		printf("malloc rpbuf_audio_playback failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	data = malloc(size);
	if (!data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto EXIT;
	}
	memset(data, 0, size);

	priv->playfile = fopen(filename, "rb");
	if (!priv->playfile) {
		printf("open file error!\n");
		goto EXIT;
	}

	if (rpbuf_file_type == RPBUF_AUDIO_FORMAT_WAVE) {
		if (fread(&(priv->audio_info.header),
			sizeof(struct RIFF_HEADER_DEF), 1, priv->playfile) != 1) {
			printf("read header info error");
			goto EXIT;
		}

		if (fread(&(priv->audio_info.fmt_block),
			sizeof(struct FMT_BLOCK_DEF), 1, priv->playfile) != 1) {
			printf("read fmt_block info error");
			goto EXIT;
		}

		if (fread(&(priv->audio_info.data_block),
			sizeof(struct DATA_BLOCK_DEF), 1, priv->playfile) != 1) {
			printf("read data_block info error");
			goto EXIT;
		}
		wave_fmt = &priv->audio_info.fmt_block.wav_format;

		if (!strncmp(priv->audio_info.data_block.data_id, "data", sizeof(priv->audio_info.data_block.data_id))) {
			playback_count = priv->audio_info.data_block.data_size;
		}
		else {
			printf("read data_block info error\n");
			goto EXIT;
		}
	}


	printf("here\n");

	if (wave_fmt) {
		ret = rpbuf_to_pcm_fmt(&priv->audio_info);
		if (ret != 0) {
			printf("rpbuf_to_pcm_fmt failed\n");
			goto EXIT;
		}
		prehwparams.channels = wave_fmt->channels;
		prehwparams.rate = wave_fmt->samples_per_sec;
	}

	/* 1. open playback audio */

	pcm_config.channels = prehwparams.channels;
	pcm_config.format = prehwparams.format;
	pcm_config.rate = prehwparams.rate;
	pcm_config.period_size = AUDIO_RPBUF_AUDIO_DEFAULT_SIZE;
	pcm_config.period_count = DEFAULT_RPBUF_AUDIO_BUF_CNT;


	priv->pcm_dev = open_audio(rpbuf_pcm_name, rpbuf_stream, &pcm_config);
	if (!priv->pcm_dev) {
		printf("open %s audio fail, stream %d!\n", rpbuf_pcm_name, rpbuf_stream);
		goto EXIT;
	}

	/* 3. start playback */

	while (written  < playback_count && !in_stopping) {

		c = playback_count - written;

		if (c > size)
			c = size;

		length = fread(data, 1, c, priv->playfile);
		//printf("read  len %ld\n", length);
		if (length < 0) {
			printf("read file error rlen %ld!\n", length);
			break;
		}

		frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, length);

		r = audio_write(priv->pcm_dev, data, frame_count);
		if (r < 0) {
			if (r = -EPIPE)
				printf("audio write underrun, size %ld\n", length);
			else {
				printf("audio write err, size %ld\n", length);
				break;
			}
		}
		else if (r != frame_count) {
			printf("write to pcm %ld!=%lu\n", r, frame_count);
			break;
		}
		r = snd_pcm_frames_to_bytes(priv->pcm_dev, r);
		written += r;
	}

EXIT:

	printf("rpbuf audio raw playback exit\n");

	if (priv->pcm_dev) {

		ret = close_audio(priv->pcm_dev, rpbuf_stream);
		if (ret != 0) {
			printf("close_audio fail\n");
		}
		priv->pcm_dev = NULL;

		printf("close_audio exit\n");
	}

	if (data) {
		free(data);
		data = NULL;
	}

	if (priv->playfile) {
		fclose(priv->playfile);
		priv->playfile = NULL;
	}

	if (priv) {
		free(priv);
		priv = NULL;
	}
	return ret;
}

static void *rpbuf_arecord_wait_mode_thread(void *params)
{
	rpbuf_audioraw_ctx_t *priv = (rpbuf_audioraw_ctx_t *)params;
	rpbuf_audio_remote_ctrl_raw_t *rpbuf_ctl_parm;
	int ret = 0;
	void *recv_data = NULL;
	unsigned long size = sizeof(rpbuf_audio_remote_ctrl_raw_t);
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	int timeout = 3000;
	int mode;

	recv_data = malloc(size);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, size);

	while (1) {

		if ((priv->lowpower_thread_exit & AUDIO_LOWPOWER_PROCESS_EXIT_BIT) || in_stopping) {
			break;
		}

		pthread_mutex_lock(&priv->mode_mutex);
		mode = priv->mode;
		pthread_mutex_unlock(&priv->mode_mutex);

		if (mode == RPBUF_AUDIO_DSP_RECORD_MODE_NORMAL) {
			ret = thread_sem_timed_down(&priv->thread_wait_normal_mode, 100);
			if (ret == ETIMEDOUT)
				continue;
			else if (ret != 0) {
				printf("thread_sem_timed_down error %d\n", ret);
				break;
			}
		}
		else if (mode == RPBUF_AUDIO_DSP_RECORD_MODE_LOW_POWER) {
			ret = rpbuf_audio_raw_recv_data(priv->rpbuf_audio_ctx, priv->raw_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
			if (ret == -ETIMEDOUT) {
				continue;
			}
			else if (ret != 0) {
				printf("rpbuf %s recv size %lu error %d\n", priv->raw_out_name, recv_size, ret);
				break;
			}

			if (recv_size > size) {
				printf("recv size %lu is larger than size %ld\n", recv_size, size);
				break;
			}

			rpbuf_ctl_parm = (rpbuf_audio_remote_ctrl_raw_t *)recv_data;
			pthread_mutex_lock(&priv->mode_mutex);
			if (rpbuf_ctl_parm->cmd == RPBUF_CTL_LOW_POWER_MODE && priv->mode != rpbuf_ctl_parm->value) {
				priv->mode = rpbuf_ctl_parm->value;
				printf("dsp remote call, it will be enter into %d mode\n", priv->mode);
				if (priv->mode == RPBUF_AUDIO_DSP_RECORD_MODE_NORMAL)
					thread_sem_signal(&priv->thread_wait_lowpower_mode);
			}
			pthread_mutex_unlock(&priv->mode_mutex);
		}
	}

exit:

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

	priv->lowpower_thread_exit |= AUDIO_WAIT_MODE_PROCESS_EXIT_BIT;

	printf("end of wait mode thread\n");
	return NULL;
}

static void *rpbuf_arecord_lowpower_recv_thread(void *params)
{
	rpbuf_audioraw_ctx_t *priv = (rpbuf_audioraw_ctx_t *)params;
	int ret = 0;
	//long write_size = 0;
	uint64_t count = 0;
	uint64_t fdcount = 0;
	struct hwparams prehwparams;
	void *recv_data = NULL;
	unsigned long rpbuf_len = priv->rpbuf_len;
	unsigned long arecord_period_size = priv->rpbuf_audio_out_len;
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	int timeout = 500;
	//long long start_usec, end_usec;
	//float delta_usec;
	int64_t capture_count = 0;
	int mode = 0;

	count = calc_data_count();
	if (count == 0)
		count = LLONG_MAX;

	if (count < LLONG_MAX)
			count += count % 2;
		else
			count -= count % 2;

	capture_count = count;

	printf("count %lu\n", count);

	if (rpbuf_file_type == RPBUF_AUDIO_FORMAT_WAVE) {
		prehwparams.format = rprehwparams.format;
		prehwparams.channels = rprehwparams.channels;
		prehwparams.rate =  rprehwparams.rate;
		start_wave_header(priv->infile, count, &prehwparams);
	}

	/* it will be malloc 2M ddr size */
	recv_data = malloc(rpbuf_len);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, rpbuf_len);

	ret = rpbuf_audio_record_start(priv);
	if (ret != 0) {
		printf("rpbuf_audioraw_start %d error!\n", ret);
		goto exit;
	}

	while (capture_count > 0 && !in_stopping) {

		pthread_mutex_lock(&priv->mode_mutex);
		mode = priv->mode;
		pthread_mutex_unlock(&priv->mode_mutex);

		if (mode == RPBUF_AUDIO_DSP_RECORD_MODE_LOW_POWER) {
			printf("enter low power mode, waiting dsp to wakeup!\n");
			thread_sem_wait(&priv->thread_wait_lowpower_mode);
			printf("enter normal mode, it will be recving data\n");
		}

		if (!rpbuf_audio_raw_is_available(priv->rpbuf_audio_ctx, priv->raw_out_name)) {
			printf("rpbuf %s is unavailable, exit audio record lowpower recv loop!\n", priv->raw_out_name);
			break;
		}

		//start_usec = get_currenttime();
		//printf("rpbuf fdcount %llu\n", fdcount);

		ret = rpbuf_audio_raw_recv_data(priv->rpbuf_audio_ctx, priv->raw_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret == -ETIMEDOUT) {
			continue;
		}
		else if (ret != 0) {
			printf("rpbuf %s recv size %lu error %d\n", priv->raw_out_name, recv_size, ret);
			break;
		}

		if (mode == RPBUF_AUDIO_DSP_RECORD_MODE_LOW_POWER && recv_size != rpbuf_len) {
			printf("audio recv data error recv size %ld size %ld\n", recv_size, rpbuf_len);
			break;
		} else if (mode == RPBUF_AUDIO_DSP_RECORD_MODE_NORMAL && recv_size != arecord_period_size) {
			printf("audio recv data error recv size %ld size %ld\n", recv_size, arecord_period_size);
			break;
		}

		//end_usec = get_currenttime();
		//delta_usec = end_usec - start_usec;

		if (writefile(priv->infile, recv_data, recv_size) != recv_size) {
			printf("write size %lu error!\n", recv_size);
			break;
		}

		memset(recv_data, 0, recv_size);

		fdcount += recv_size;
		capture_count -= recv_size;

/*
		write_size += recv_size;
		if (write_size == recv_size * 30) {
			fflush(priv->infile);
			fsync(fileno(priv->infile));
			write_size = 0;
		}
*/
		//printf("write size %lu, cnt %llu, delta msec %f\n", recv_size, fdcount ,delta_usec / 1000);

	}

	ret = rpbuf_audio_record_stop(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_stop %d error!\n", ret);
		goto exit;
	}

exit:

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

	if (rpbuf_file_type == RPBUF_AUDIO_FORMAT_WAVE) {
		printf("fdcount %lu\n", fdcount);
		end_wave(priv->infile, fdcount);
	}

	if (priv->infile) {
		fflush(priv->infile);
		fsync(fileno(priv->infile));
		fclose(priv->infile);
		priv->infile = NULL;
	}

	priv->lowpower_thread_exit |= AUDIO_LOWPOWER_PROCESS_EXIT_BIT;

	printf("end of low power recv thread\n");
	return NULL;
}

static int rpbuf_audio_lowpower_process(char *filename)
{
	int ret = 0;
	rpbuf_audioraw_ctx_t *priv = NULL;
	pthread_t thread_recv = 0;
	pthread_t thread_wait = 0;
	int frame_bytes = 0;
	int mode = 0;
	int record_time_sec = time_sec;
	long long record_time_msec = time_sec * 1000;
	int repeat = repeat_cnt;

	printf("audio low power process enter\n");

	priv = malloc(sizeof(rpbuf_audioraw_ctx_t));
	if (priv) {
		memset(priv, 0, sizeof(rpbuf_audioraw_ctx_t));
	} else {
		printf("malloc rpbuf_audio_capture failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->rpbuf_ctl_arg = malloc(sizeof(rpbuf_audio_remote_ctrl_raw_t));
	if (priv->rpbuf_ctl_arg) {
		memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_raw_t));
	} else {
		printf("malloc rpbuf_audio_capture failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->infile = fopen(filename, "wb+");
	if (!priv->infile) {
		printf("open file %s error!\n", filename);
		goto EXIT;
	}

	ret= pthread_mutex_init(&priv->mode_mutex, NULL);
	if (ret != 0) {
		goto EXIT;
	}

	ret = thread_sem_init(&priv->thread_wait_normal_mode, 0);
	if (ret != 0) {
		printf("thread_sem_init wait normal mode error!\n");
		goto EXIT;
	}

	ret = thread_sem_init(&priv->thread_wait_lowpower_mode, 0);
	if (ret != 0) {
		printf("thread_sem_init wait lowpower mode  error!\n");
		goto EXIT;
	}


	frame_bytes = snd_pcm_format_physical_width(rprehwparams.format) / 8 * rprehwparams.channels;

	priv->rpbuf_audio_in_len = (frame_bytes * rprehwparams.rate * rprehwparams.msec) / 1000; /* 4 * 16 * 64 = 4096 bytes */

	priv->rpbuf_audio_out_len = priv->rpbuf_audio_in_len;

	priv->rpbuf_len = priv->rpbuf_audio_in_len * 480;  //64ms * 48 = 30720ms (4* 480K = 1920K)
	if (priv->rpbuf_len + sizeof(rpbuf_audio_remote_ctrl_raw_t) > RPBUF_LEN_MAX){
		printf("rpbuf_size %ld is larger than MAX %d\n", priv->rpbuf_len + sizeof(rpbuf_audio_remote_ctrl_raw_t), RPBUF_LEN_MAX);
		goto EXIT;
	}

	printf("rpbuf_audio_in_len %u out %u rpbuf_len %u \n", priv->rpbuf_audio_in_len, priv->rpbuf_audio_out_len, priv->rpbuf_len);

	/* init */
	ret = rpbuf_audio_record_init(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_init %d failed\n", ret);
		goto EXIT;
	}

	/* set record param */
	ret = rpbuf_audio_record_config(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_config %d failed\n", ret);
		goto EXIT;
	}

	priv->mode = RPBUF_AUDIO_DSP_RECORD_MODE_NORMAL;
	priv->lowpower_thread_exit = 0;

	/* start recv thread */
	pthread_create(&thread_recv,
				NULL,
				rpbuf_arecord_lowpower_recv_thread,
				(void *)priv);

	pthread_setname_np(thread_recv, "rpbuf_arecord_lowpower_recv_thread");

	pthread_create(&thread_wait,
				NULL,
				rpbuf_arecord_wait_mode_thread,
				(void *)priv);

	pthread_setname_np(thread_wait, "rpbuf_arecord_wait_mode_thread");

	while(1) {

		if ((priv->lowpower_thread_exit & AUDIO_WAIT_MODE_PROCESS_EXIT_BIT) || in_stopping) {
			break;
		}

		/* normal mode will be only sleep */
		usleep(SLEEP_MS * 1000);

		pthread_mutex_lock(&priv->mode_mutex);
		mode = priv->mode;
		pthread_mutex_unlock(&priv->mode_mutex);

		if (record_time_sec >= 0 && mode == RPBUF_AUDIO_DSP_RECORD_MODE_NORMAL) {
			/* After recording record_time_msec, it will be enter low power mode */
			record_time_msec -= SLEEP_MS;
			if (record_time_msec <= 0) {
				ret = rpbuf_audio_record_lowpower(priv, RPBUF_AUDIO_DSP_RECORD_MODE_LOW_POWER);
				if (ret != 0) {
					printf("rpbuf_audio_record_lowpower %d failed\n", ret);
					break;
				}
				/* Enter low power mode */
				//system("echo mem > /sys/power/state");
				thread_sem_up(&priv->thread_wait_normal_mode);
				if (repeat) {
					record_time_msec = record_time_sec * 1000;
					repeat--;
				} else {
					record_time_sec = -1;
				}
			}
		}

	}

	/* end the recv thread */
	if (thread_recv)
		pthread_join(thread_recv, NULL);

	thread_recv = 0;

	if (thread_wait)
		pthread_join(thread_wait, NULL);

	thread_wait = 0;

	ret = rpbuf_audio_record_release(priv);
	if (ret != 0) {
		printf("rpbuf_audio_record_release %d failed\n", ret);
		goto EXIT;
	}

EXIT:

	printf("rpbuf audio raw capture exit\n");

	if (thread_recv)
		pthread_join(thread_recv, NULL);

	if (thread_wait)
		pthread_join(thread_wait, NULL);

	if (priv->rpbuf_audio_ctx) {
		rpbuf_audio_raw_deinit(priv->rpbuf_audio_ctx);
		priv->rpbuf_audio_ctx = NULL;
	}

	thread_sem_deinit(&priv->thread_wait_normal_mode);

	thread_sem_deinit(&priv->thread_wait_lowpower_mode);

	pthread_mutex_destroy(&priv->mode_mutex);

	if (priv->infile) {
		fflush(priv->infile);
		fsync(fileno(priv->infile));
		fclose(priv->infile);
		priv->infile = NULL;
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

static void print_rpbuf_audioraw_help_msg(void)
{
	printf("\n");
	printf("USAGE:\n");
	printf("  rpbuf_audio_demo [OPTIONS]\n");
	printf("\n");
	printf("OPTIONS:\n");
	printf("  -D          : select PCM by name\n");
	printf("  -t          : sfile type (voc, wav, raw or au)\n");
	printf("  -c          : channels\n");
	printf("  -f          : sample format (case insensitive)\n");
	printf("  -r          : sample rate\n");
	printf("  -p          : set period size (default is 320\n");
	printf("  -b          : set buffer size (default is 1280\n");
	printf("  -u          : set card number (0: card0, 1: card1, default is 0\n");
	printf("  -s          : audio record per mseconds(ms)\n");
	printf("  -d          : interrupt after # seconds\n");
	printf("  -m          : set mode (0: normal, 1: low power mode test, default is 0\n");
	printf("  -u          : set card number (0: card0, 1: card1, default is 0\n");
	printf("  -P          : PlayBack\n");
	printf("  -C          : Capture\n");
	printf("\n");
	printf("e.g.\n");
	printf("  rpbuf_audio_raw_demo -Dhw:audiocodec -t wav -P /tmp/test.wav \n");
	printf("  rpbuf_audio_raw_demo -f S16_LE -r 16000 -c 2 -s 64 -d 20 -t wav  -p 320 -b 1280 -C /tmp/test_arecord.wav \n");
	printf("  rpbuf_audio_raw_demo -f S16_LE -r 16000 -c 2 -s 64 -d 20 -C /tmp/test_arecord.pcm \n");
	printf("\n");
}

/*
 *	Subroutine to clean up before exit.
 */
static void prg_exit(int code)
{
	exit(code);
}

static void rpbuf_audio_signal_handler(int sig)
{

	if (in_stopping)
		return;

	switch (sig) {
		case SIGINT:
		case SIGTERM:
			printf("SIGINT or SIGTERM occurred, stop recording\n");
			in_stopping = 1;
			break;
		case SIGABRT:
			printf("SIGABRT occurred, aborting\n");
			in_stopping = 1;
			prg_exit(EXIT_FAILURE);
			break;
		default:
			printf("Signal %d occurred, do nothing\n", sig);
			break;
	}

	signal(sig, SIG_DFL);
}

int main(int argc, char **argv)
{
	int ret = 0;
	int c;
	int tmp;
	long long_tmp;

	if (argc <= 1) {
		ret = -1;
		goto out;
	}

	g_rpbuf_raw_index = 0;
	g_rpbuf_card_num = 0;
	rpbuf_file_type = RPBUF_AUDIO_FORMAT_DEFAULT;
	rprehwparams.format = RPBUF_AUDIO_DEFAULT_FORMAT;
	rprehwparams.rate = RPBUF_AUDIO_DEFAULT_SPEED;
	rprehwparams.channels = RPBUF_AUDIO_DEFAULT_CHANNEL;
	rprehwparams.msec = RPBUF_AUDIO_DEFAULT_MSEC;
	rprehwparams.period_size = RPBUF_AUDIO_DEFAULT_PERIOD_SIZE;
	rprehwparams.buffer_size = RPBUF_AUDIO_DEFAULT_BUFFER_SIZE;


	while ((c = getopt(argc, argv, "hD:t:c:f:r:s:d:m:i:n:k:u:p:b:PC")) != -1) {
		switch (c) {
		case 'h':
			print_rpbuf_audioraw_help_msg();
			ret = 0;
			goto out;
		case 'D':
			rpbuf_pcm_name = optarg;
			break;
		case 't':
			if (strcasecmp(optarg, "raw") == 0)
				rpbuf_file_type = RPBUF_AUDIO_FORMAT_RAW;
			else if (strcasecmp(optarg, "voc") == 0)
				rpbuf_file_type = RPBUF_AUDIO_FORMAT_VOC;
			else if (strcasecmp(optarg, "wav") == 0)
				rpbuf_file_type = RPBUF_AUDIO_FORMAT_WAVE;
			else {
				printf("unrecognized file format %s", optarg);
				return 1;
			}
			break;
		case 'c':
			rprehwparams.channels = strtol(optarg, NULL, 0);
			if (rprehwparams.channels < 1 || rprehwparams.channels > 256) {
				printf("value %u for channels is invalid", rprehwparams.channels);
				return 1;
			}
			break;
		case 'f':
			rprehwparams.format = snd_pcm_format_value(optarg);
			if (rprehwparams.format == SND_PCM_FORMAT_UNKNOWN) {
				printf("wrong extended format '%s'", optarg);
				return 1;
			}
			break;
		case 'r':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 300)
				tmp *= 1000;
			rprehwparams.rate = tmp;
			if (tmp < 2000 || tmp > 192000) {
				printf("bad speed value %d", tmp);
				return 1;
			}
			break;
		case 's':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0) {
				printf("bad msec %d", tmp);
				return 1;
			}
			rprehwparams.msec = tmp;
			break;
		case 'd':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				ret = -EINVAL;
				goto out;
			}
			rpbuf_timelimit = long_tmp;
			break;
		case 'm':
			rpbuf_mode = strtol(optarg, NULL, 0);
			if (rpbuf_mode < 0 || rpbuf_mode == LONG_MAX) {
				printf("value %d for mode is invalid", rpbuf_mode);
				return 1;
			}
			break;
		case 'i':
			time_sec = strtol(optarg, NULL, 0);
			if (time_sec < 0 || time_sec == LONG_MAX) {
				printf("value %d for mode is invalid", time_sec);
				return 1;
			}
			break;
		case 'n':
			repeat_cnt = strtol(optarg, NULL, 0);
			if (repeat_cnt < 0 || repeat_cnt == LONG_MAX) {
				printf("value %d for mode is invalid", repeat_cnt);
				return 1;
			}
			break;
		case 'k':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			g_rpbuf_raw_index = tmp;
			break;
		case 'u':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			g_rpbuf_card_num = tmp;
			break;
		case 'p':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			rprehwparams.period_size = tmp;
			break;
		case 'b':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			rprehwparams.buffer_size = tmp;
			break;
		case 'P':
			rpbuf_stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'C':
			rpbuf_stream = SND_PCM_STREAM_CAPTURE;
			break;
		default:
			printf("Invalid option: -%c\n", c);
			ret = -1;
			goto out;
		}
	}

	if (optind > argc - 1) {
		printf("Please enter the filename\n");
		ret = -1;
		goto out;
	}
	prehwparams = rprehwparams;

	signal(SIGINT, rpbuf_audio_signal_handler);
	signal(SIGTERM, rpbuf_audio_signal_handler);
	signal(SIGABRT, rpbuf_audio_signal_handler);

	switch (rpbuf_mode) {
		case 0:
		{
			if (rpbuf_stream == SND_PCM_STREAM_PLAYBACK) {
				rpbuf_audioraw_playback(argv[optind++]);
			} else {
				rpbuf_audioraw_capture(argv[optind++]);
			}
			break;
		}

		case 1:
		{
			rpbuf_audio_lowpower_process(argv[optind++]);
			break;
		}

		default:
			printf("unknown mode %d command\n", rpbuf_mode);
		break;
	}
	return ret;


out:
	print_rpbuf_audioraw_help_msg();
	return ret;

}




