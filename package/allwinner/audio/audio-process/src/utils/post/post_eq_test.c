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
#include <pthread.h>
#include <time.h>
#include <sys/time.h>


#include "post_equal.h"
#include "audio_common.h"
#include "thread_sem.h"

#define AUDIO_POST_EQ_DEFAULT_SIZE 960
#define DEFAULT_POST_EQ_BUF_CNT 4

#define POST_EQ_PCM_NAME_DEFAULT "default"
#define POST_RPBUF_BUFFER_LENGTH_DEFAULT 1280  //byte
#define POST_TIME_MS_DEFAULT 20  //ms

#define POST_DEFAULT_FORMAT		SND_PCM_FORMAT_S16_LE
#define POST_DEFAULT_SPEED 		16000

#define POST_FORMAT_DEFAULT		-1
#define POST_FORMAT_RAW		0
#define POST_FORMAT_VOC		1
#define POST_FORMAT_WAVE		2


/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092

static struct {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
} posthwparams, rposthwparams;

static snd_pcm_stream_t post_stream = SND_PCM_STREAM_PLAYBACK;
static int post_file_type = POST_FORMAT_DEFAULT;
static int ctrl_id = 0;
static size_t g_eq_len = POST_RPBUF_BUFFER_LENGTH_DEFAULT;
static size_t g_eq_run_msec = POST_TIME_MS_DEFAULT;
static char *pcm_name = POST_EQ_PCM_NAME_DEFAULT;
static char filename[32] = "/data/test.pcm";

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
/*
typedef struct bufferheader
{
	void *recv_data;
	int use_tag;
	unsigned long recv_size;
} bufferheader;
*/

typedef struct apost_eq_test {
	snd_pcm_t *pcm_dev;
	snd_pcm_postequal_t *pcm_postequal;
	uint32_t data_len;
	FILE *infile;
	struct wav_info audio_info;
	rpbuf_arg_post_eq eq_arg;
}apost_eq_test_t;

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
			if (posthwparams.format != POST_DEFAULT_FORMAT &&
				posthwparams.format != SND_PCM_FORMAT_U8)
				printf("Warning: format is changed to U8\n");
			posthwparams.format = SND_PCM_FORMAT_U8;
			break;
		case 16:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S16_BE;
			else
				native_format = SND_PCM_FORMAT_S16_LE;
			if (posthwparams.format != POST_DEFAULT_FORMAT &&
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
				if (posthwparams.format != POST_DEFAULT_FORMAT &&
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
				if (posthwparams.format != POST_DEFAULT_FORMAT &&
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

static void *transmit_thread(void *params)
{
	apost_eq_test_t *priv = (apost_eq_test_t *)params;
	int ret = 0;
	void *data = NULL;
	void *recv_data = NULL;
	unsigned long size = priv->data_len;
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	long long start_usec, end_usec;
	float delta_usec;
	snd_pcm_sframes_t length;
	int timeout = 2000;
	unsigned int frame_count = 0;

	data = malloc(size);
	if (!data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(data, 0, size);

	recv_data = malloc(size);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, size);

#ifdef DUMP_FILE
	FILE *dumpfile = NULL;
	long write_size = 0;

	dumpfile = fopen(filename, "wb+");
	if (!dumpfile) {
		printf("open file %s error!\n", filename);
		goto exit;
	}
#endif

	while (1) {


		length = fread(data, 1, size, priv->infile);
		//printf("read  len %ld\n", length);
		if (length <= 0) {
			printf("read file error rlen %ld!\n", length);
			break;
		}

		/* transfer data to dsp */
		length = post_equal_transfer(priv->pcm_postequal, priv->eq_arg.aeq_in_name, data, 0, size);
		if (length <= 0)
		{
			printf("post_equal_transfer error send_len %lu!\n", length);
			break;
		}

		start_usec = get_currenttime();

		memset(data, 0, size);

		ret = post_equal_recv(priv->pcm_postequal, priv->eq_arg.aeq_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret != 0) {
			printf("post equal recv size %lu\n", recv_size);
			break;
		}

		if (recv_size != size) {
			printf("eq recv data error recv size %ld size %ld\n", recv_size, size);
			break;
		}

#ifdef DUMP_FILE
		if (dumpfile) {
			ret = fwrite(recv_data, 1, size, dumpfile);
			if (ret != size)
				printf("[%s] line:%d fwrite err ! ret=%d, data_len=%ld\n", __func__, __LINE__, ret, size);
			write_size += size;
			if (write_size == size * 100) {
				fflush(dumpfile);
				fsync(fileno(dumpfile));
				write_size = 0;
			}
		}
#endif

		frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, recv_size);

		ret = audio_write(priv->pcm_dev, recv_data, frame_count);
		if (ret < 0) {
			if (ret = -EPIPE)
				printf("audio write underrun, size %lu\n", recv_size);
			else {
				printf("audio write err, size %lu\n", recv_size);
				break;
			}
		}

		end_usec = get_currenttime();

		delta_usec = end_usec - start_usec;
		//printf("recv size %lu, delta usec %f time %lli\n", recv_size, delta_usec / 1000, end_usec);


	}
exit:


	if (data)
		free(data);

	data = NULL;

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

#ifdef DUMP_FILE
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

int post_eq_capture(char *filename, rpbuf_arg_post_eq *arg)
{
	printf("post_eq_record\n");
	return 0;
}

int post_eq_playback(char *filename, rpbuf_arg_post_eq *arg)
{
	int ret = 0;
	int i = 0;
	apost_eq_test_t *priv = NULL;
	snd_pcm_postequal_t *pcm_postequal = NULL;
	pthread_t thread_tran = 0;
	struct WAVE_FORMAT_DEF *wave_fmt = NULL;
	route_type_t route_type;
	pcm_config_t pcm_config = {0};
	eq_remote_prms_t aeq_prms = { 0 };
	unsigned long  aeq_prms_offset = 0;
	unsigned long  aeq_prms_size = 0;
	char *eq_name = POST_EQ_PCM_NAME_DEFAULT;
	int frame_bytes = 0;

	printf("post_eq_playback enter\n");

	priv = malloc(sizeof(apost_eq_test_t));
	if (priv) {
		memset(priv, 0 , sizeof(apost_eq_test_t));
		memcpy(&priv->eq_arg, arg, sizeof(rpbuf_arg_post_eq));
	} else {
		printf("malloc apost_eq_test failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->infile = fopen(filename, "rb");
	if (!priv->infile) {
		printf("open file error!\n");
		goto EXIT;
	}

	if (post_file_type == POST_FORMAT_WAVE) {
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

	/* 1. open_audio */

	pcm_config.channels = posthwparams.channels;
	pcm_config.format = posthwparams.format;
	pcm_config.rate = posthwparams.rate;
	pcm_config.period_size = AUDIO_POST_EQ_DEFAULT_SIZE;
	pcm_config.period_count = DEFAULT_POST_EQ_BUF_CNT;

	priv->pcm_dev = open_audio(eq_name, post_stream, &pcm_config);
	if (!priv->pcm_dev) {
		printf("open %s audio fail, stream %d!\n", eq_name, post_stream);
		goto EXIT;
	}

	route_type = HpSpeaker;

	ret = audio_set_playback_route(eq_name,  1, route_type);
	if (ret != 0) {
		printf("audio_set_playback_route fail\n");
		goto EXIT;
	}

	frame_bytes = (snd_pcm_format_physical_width(posthwparams.format) / 8) *  posthwparams.channels;

	priv->data_len = (frame_bytes * posthwparams.rate * g_eq_run_msec) / 1000;

	if (g_eq_len < priv->data_len) {
		printf("input param error, aeq_len %u is less than data_len %u. Enlarge input len %u or decrease input time %u\n", \
				g_eq_len, priv->data_len, g_eq_len, g_eq_run_msec);
		goto EXIT;
	}

	/* 2. open post eq */
	ret = post_equal_open(&pcm_postequal, pcm_name, &priv->eq_arg, ctrl_id, 1, g_eq_len);
	if (ret != 0) {
		printf("post equal open %s fail, aeq_config_name %s aeq_reset_config_name %s aeq_in_name %s aeq_out_name %s !\n", \
			pcm_name, arg->aeq_config_name, arg->aeq_reset_config_name, arg->aeq_in_name, arg->aeq_out_name);
		goto EXIT;
	}
	priv->pcm_postequal = pcm_postequal;

	/* 3. set post eq param */
	ret = post_equal_hw_params(priv->pcm_postequal, posthwparams.channels, posthwparams.rate, snd_pcm_format_physical_width(posthwparams.format));
	if (ret != 0) {
		printf("post equal hw params fail, channels %u rate %u !\n", posthwparams.channels, posthwparams.rate);
		goto EXIT;
	}

	memset(&aeq_prms, 0, sizeof(eq_remote_prms_t));

	ret = post_equal_recv(priv->pcm_postequal, priv->eq_arg.aeq_config_name, &aeq_prms, &aeq_prms_offset, &aeq_prms_size, -1);
	if (ret != 0) {
		printf("post aeq arg size %lu\n", aeq_prms_size);
		goto EXIT;
	}

	if (aeq_prms.chan != posthwparams.channels || aeq_prms.bits_per_sample != snd_pcm_format_physical_width(posthwparams.format) ||
		aeq_prms.sampling_rate != posthwparams.rate || aeq_prms_size != sizeof(eq_remote_prms_t)) {
		printf("post_equal_recv error size %lu\n", aeq_prms_size);
		goto EXIT;
	}

	/* 4. start transmit thread*/

	pthread_create(&thread_tran,
				NULL,
				transmit_thread,
				(void *)priv);

	pthread_setname_np(thread_tran, "transmit_thread");

	/* end the wavread thread */
	pthread_join(thread_tran, NULL);
	thread_tran = 0;


EXIT:

	printf("post equal playback exit\n");
	if (thread_tran)
		pthread_join(thread_tran, NULL);

	if (priv->pcm_postequal) {
		post_equal_hw_free(priv->pcm_postequal);
		post_equal_close(priv->pcm_postequal);
		priv->pcm_postequal = NULL;
	}

	if (priv->infile) {
		fclose(priv->infile);
		priv->infile = NULL;
	}

	if (priv->pcm_dev) {
		route_type = HpSpeaker;
		ret = audio_set_playback_route(eq_name,	0, route_type);
		if (ret != 0) {
			printf("audio_set_playback_route fail\n");
		}
		printf("audio_set_playback_route exit\n");

		ret = close_audio(priv->pcm_dev, post_stream);
		if (ret != 0) {
			printf("close_audio fail\n");
		}
		priv->pcm_dev = NULL;

		printf("close_audio exit\n");
	}

	if (priv) {
		free(priv);
		priv = NULL;
	}
	return ret;
}

static void print_help_msg(void)
{
	printf("\n");
	printf("USAGE:\n");
	printf("  post_eq_test [OPTIONS]\n");
	printf("\n");
	printf("OPTIONS:\n");
	printf("  -D          : select PCM by name\n");
	printf("  -t          : sfile type (voc, wav, raw or au)\n");
	printf("  -c          : channels\n");
	printf("  -f          : sample format (case insensitive)\n");
	printf("  -r          : sample rate\n");
	printf("  -s          : rpbuf start name (default: rpbuf_eq_start)\n");
	printf("  -d          : rpbuf reset name (default: rpbuf_eq_reset)\n");
	printf("  -p          : rpbuf send name (default: rpbuf_eq_send)\n");
	printf("  -g          : rpbuf recv name (default: rpbuf_eq_recv)\n");
	printf("  -I ID       : specify rpbuf ctrl ID (default: 0)\n");
	printf("  -L LENGTH   : specify buffer length (default: %d bytes)\n",
			POST_RPBUF_BUFFER_LENGTH_DEFAULT);
	printf("  -T MS   : specify time (default: %d MS)\n",
			POST_TIME_MS_DEFAULT);
	printf("  -P          : PlayBack\n");
	printf("  -C          : Capture\n");
	printf("\n");
	printf("e.g.\n");
	printf("  post_eq_test -DPlaybackEQ —L 4096 -T 20 -t wav -P /data/test.wav \n");
	printf("  post_eq_test -DPlaybackEQ —L 4096 -T 20 -c 2 -r 16000 -f S16_LE -P /data/test.pcm \n");
	printf("  post_eq_test -DPlaybackEQ —L 4096 -T 20 -t wav -s \"rp_eq_start\" -d \"rp_eq_reset\" -p \"rp_eq_send\" -g \"rp_eq_recv\" -P /data/test.wav\n");
	printf("\n");
}


int main(int argc, char **argv)
{
	int ret = 0;
	int c;
	int tmp;
	long long_tmp;

	if (argc <= 1) {
		print_help_msg();
		ret = -1;
		goto out;
	}

	rpbuf_arg_post_eq targ = {
		.aeq_config_name =       "rpbuf_eq_config",
		.aeq_reset_config_name = "rpbuf_eq_reset",
		.aeq_in_name  =          "rpbuf_eq_in",
		.aeq_out_name =          "rpbuf_eq_out",
	};

	ctrl_id = 0;
	post_file_type = POST_FORMAT_DEFAULT;
	rposthwparams.format = POST_DEFAULT_FORMAT;
	rposthwparams.rate = POST_DEFAULT_SPEED;
	rposthwparams.channels = 2;
	g_eq_len = POST_RPBUF_BUFFER_LENGTH_DEFAULT;
	g_eq_run_msec = POST_TIME_MS_DEFAULT;

	while ((c = getopt(argc, argv, "hD:t:c:f:r:s:d:p:g:I:L:T:PC")) != -1) {
		switch (c) {
		case 'h':
			print_help_msg();
			ret = 0;
			goto out;
		case 'D':
			pcm_name = optarg;
			break;
		case 't':
			if (strcasecmp(optarg, "raw") == 0)
				post_file_type = POST_FORMAT_RAW;
			else if (strcasecmp(optarg, "voc") == 0)
				post_file_type = POST_FORMAT_VOC;
			else if (strcasecmp(optarg, "wav") == 0)
				post_file_type = POST_FORMAT_WAVE;
			else {
				printf("unrecognized file format %s", optarg);
				return 1;
			}
			break;
		case 'c':
			rposthwparams.channels = strtol(optarg, NULL, 0);
			if (rposthwparams.channels < 1 || rposthwparams.channels > 256) {
				printf("value %u for channels is invalid", rposthwparams.channels);
				return 1;
			}
			break;
		case 'f':
			rposthwparams.format = snd_pcm_format_value(optarg);
			if (rposthwparams.format == SND_PCM_FORMAT_UNKNOWN) {
				printf("wrong extended format '%s'", optarg);
				return 1;
			}
			break;
		case 'r':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 300)
				tmp *= 1000;
			rposthwparams.rate = tmp;
			if (tmp < 2000 || tmp > 192000) {
				printf("bad speed value %d", tmp);
				return 1;
			}
			break;
		case 's':
			strncpy(targ.aeq_config_name, optarg, sizeof(targ.aeq_config_name));
			break;
		case 'd':
			strncpy(targ.aeq_reset_config_name, optarg, sizeof(targ.aeq_reset_config_name));
			break;
		case 'p':
			strncpy(targ.aeq_in_name, optarg, sizeof(targ.aeq_in_name));
			break;
		case 'g':
			strncpy(targ.aeq_out_name, optarg, sizeof(targ.aeq_out_name));
			break;
		case 'P':
			post_stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'C':
			post_stream = SND_PCM_STREAM_CAPTURE;
			break;
		case 'I':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				print_help_msg();
				ret = -EINVAL;
				goto out;
			}
			ctrl_id = long_tmp;
			break;
		case 'L':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				print_help_msg();
				ret = -EINVAL;
				goto out;
			}
			g_eq_len = long_tmp;
			break;
		case 'T':
			tmp = strtol(optarg, NULL, 0);
			if (tmp < 0 || tmp > 10000) {
				printf("bad time value %d", tmp);
				return 1;
			}
			g_eq_run_msec = tmp;
			break;
		default:
			printf("Invalid option: -%c\n", c);
			print_help_msg();
			ret = -1;
			goto out;
		}
	}

	if (optind > argc - 1) {
		printf("Please enter the filename\n");
		ret = -1;
		goto out;
	}
	posthwparams = rposthwparams;

	if (post_stream == SND_PCM_STREAM_PLAYBACK) {
		post_eq_playback(argv[optind++], &targ);
	} else {
		post_eq_capture(argv[optind++], &targ);
	}


out:
	return ret;

}




