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


#include "pre_aec.h"
#include "audio_common.h"

#define AUDIO_PRE_AEC_DEFAULT_SIZE 1024
#define DEFAULT_PRE_AEC_BUF_CNT 4

#define PRE_AEC_PCM_NAME_DEFAULT "default"

#define PRE_RPBUF_BUFFER_LENGTH_DEFAULT 2560//byte

#define PRE_RPBUF_BUFFER_AEC_OUT_LENGTH 320//byte

#define PRE_DEFAULT_FORMAT		SND_PCM_FORMAT_S16_LE
#define PRE_DEFAULT_SPEED 		16000
#define PRE_DEFAULT_CHANNEL 	2

#define PRE_FORMAT_DEFAULT		-1
#define PRE_FORMAT_RAW		0
#define PRE_FORMAT_VOC		1
#define PRE_FORMAT_WAVE		2


/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092

#define DSP_AEC_TEST

struct hwparams{
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
};

static struct hwparams prehwparams, rprehwparams;

static snd_pcm_stream_t pre_stream = SND_PCM_STREAM_CAPTURE;
static int pre_file_type = PRE_FORMAT_DEFAULT;
static int pre_ctrl_id = 0;
static int pre_mode = 0;
static long pre_timelimit = 0;
static size_t pre_len = PRE_RPBUF_BUFFER_LENGTH_DEFAULT;
static char *pre_pcm_name = PRE_AEC_PCM_NAME_DEFAULT;
static char dumppath[32] = "/data/dump.wav";
static off64_t pre_rec_count = LLONG_MAX;


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

typedef struct pre_aec_test {
	snd_pcm_t *pcm_dev;
	snd_pcm_t *pcm_daudio_dev;
	snd_pcm_pre_aec_t *pcm_preaec;

	FILE *playfile;
	FILE *infile;
	struct wav_info audio_info;
	rpbuf_arg_pre_aec aec_arg;
	int recv_thread_exit;
}pre_aec_test_t;

static long long get_currenttime()
{
	struct timeval time;

	gettimeofday(&time, NULL);

	return ((long long)time.tv_sec * 1000000 + (long long)time.tv_usec);
}

static int pre_to_pcm_fmt(struct wav_info *wav_info)
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
			if (prehwparams.format != PRE_DEFAULT_FORMAT &&
				prehwparams.format != SND_PCM_FORMAT_U8)
				printf("Warning: format is changed to U8\n");
			prehwparams.format = SND_PCM_FORMAT_U8;
			break;
		case 16:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S16_BE;
			else
				native_format = SND_PCM_FORMAT_S16_LE;
			if (prehwparams.format != PRE_DEFAULT_FORMAT &&
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
				if (prehwparams.format != PRE_DEFAULT_FORMAT &&
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
				if (prehwparams.format != PRE_DEFAULT_FORMAT &&
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
static off64_t calc_data_count(void)
{
	off64_t count;

	if (pre_timelimit == 0) {
		count = pre_rec_count;
	}
	else {
		count = snd_pcm_format_size(rprehwparams.format, rprehwparams.rate * rprehwparams.channels);
		count *= (off64_t)pre_timelimit;
	}
	return count < pre_rec_count ? count : pre_rec_count;
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

static void end_wave(FILE *file, off64_t fdcount)
{
	struct DATA_BLOCK_DEF cd;
	off64_t length_seek;
	off64_t filelen;
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

static void *aec_transmit_thread(void *params)
{
	pre_aec_test_t *priv = (pre_aec_test_t *)params;
	int ret = 0;
	void *data = NULL;
	void *recv_data = NULL;
	FILE *dumpfile = NULL;
	unsigned long size = pre_len;
	unsigned long recv_size = 0;
	unsigned long write_file_size = 0;
	unsigned long recv_data_offset = 0;
	long long start_usec, end_usec;
	float delta_usec;
	snd_pcm_sframes_t length;
	int timeout = 2000;
	off64_t count = 0;
	off64_t fdcount = 0;
	unsigned int frame_count = 0;
	long write_size = 0;

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

	dumpfile = fopen(dumppath, "wb+");
	if (!dumpfile) {
		printf("open file %s error!\n", dumppath);
		goto exit;
	}

	count = calc_data_count();
	if (count == 0)
		count = LLONG_MAX;

	if (count < LLONG_MAX)
			count += count % 2;
		else
			count -= count % 2;

	start_wave_header(dumpfile, count, &rprehwparams);


	while (count > 0) {


		frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, size);

		ret = audio_read(priv->pcm_dev, data, frame_count);
		if (ret < 0) {
			if (ret = -EPIPE)
				printf("audio read overrun, size %lu\n", size);
			else {
				printf("audio read err, size %lu\n", size);
				break;
			}
		}

		count -= size;

		write_file_size = writefile(dumpfile, data, size);

		if (write_file_size != size) {
			printf("write %lu size %lu error!\n", write_file_size, size);
			break;
		}

		fdcount += size;

		write_size += size;
		if (write_size == size * 50) {
			fflush(dumpfile);
			fsync(fileno(dumpfile));
			write_size = 0;
		}

		//printf("read size %lu frame_count %u delta usec %f time %lli\n", size, frame_count, delta_usec / 1000, end_usec);

#ifdef DSP_AEC_TEST

		start_usec = get_currenttime();

		/* transfer data to dsp */
		length = pre_aec_transfer(priv->pcm_preaec, priv->aec_arg.aec_in_name, data, 0, size);
		if (length <= 0)
		{
			printf("pre_aec_transfer error send_len %lu!\n", length);
			break;
		}

		memset(data, 0, size);

		ret = pre_aec_recv(priv->pcm_preaec, priv->aec_arg.aec_in_recv_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret != 0) {
			printf("pre aec recv size %lu\n", recv_size);
			break;
		}

		if ((recv_size != size) && memcmp(recv_data, data, recv_size)) {
			printf("recv data error recv size %ld size %ld\n", recv_size, size);
			break;
		}

		end_usec = get_currenttime();
		delta_usec = end_usec - start_usec;
		printf("aec in recv size %lu, delta usec %f time %lli\n", recv_size, delta_usec / 1000, end_usec);
#endif

		if (priv->recv_thread_exit) {
			printf("exit transmit loop!\n");
			break;
		}

	}
exit:

	priv->recv_thread_exit = 0;

	if (dumpfile) {
		end_wave(dumpfile, fdcount);
		fflush(dumpfile);
		fsync(fileno(dumpfile));
		fclose(dumpfile);
		dumpfile = NULL;
	}

	if (data)
		free(data);

	data = NULL;

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

	printf("end of transmit thread\n");
	return NULL;
}

static void *aec_recv_thread(void *params)
{
	pre_aec_test_t *priv = (pre_aec_test_t *)params;
	int ret = 0;
	long write_size = 0;
	off64_t count = 0;
	off64_t fdcount = 0;
	struct hwparams prehwparams;
	void *recv_data = NULL;
	unsigned long size = pre_len / 2;
	unsigned long recv_size = 0;
	unsigned long recv_data_offset = 0;
	int timeout = 5000;

	if (pre_file_type == PRE_FORMAT_WAVE) {
		/* aec in has two channels, left channel is mic in ,right channel is ref, but aec out only have one channel. */
		prehwparams.format = rprehwparams.format;
		prehwparams.channels  = 1;
		prehwparams.rate =  rprehwparams.rate;

		count = calc_data_count();
		if (count == 0)
			count = LLONG_MAX;

		if (count < LLONG_MAX)
				count += count % 2;
			else
				count -= count % 2;

		start_wave_header(priv->infile, count, &prehwparams);
	}

	recv_data = malloc(size);
	if (!recv_data) {
		printf("no memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	memset(recv_data, 0, size);

	while (1) {

		if (!pre_aec_rpbuf_is_available(priv->pcm_preaec, priv->aec_arg.aec_out_name)) {
			printf("exit recv loop!\n");
			break;
		}

		ret = pre_aec_recv(priv->pcm_preaec, priv->aec_arg.aec_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
		if (ret != 0) {
			printf("pre aec recv size %lu\n", recv_size);
			break;
		}

		if (recv_size != size) {
			printf("aec recv data error recv size %ld size %ld\n", recv_size, size);
			break;
		}

		if (writefile(priv->infile, recv_data, recv_size) != recv_size) {
			printf("write size %lu error!\n", recv_size);
			break;
		}

		memset(recv_data, 0, recv_size);

		fdcount += recv_size;

		write_size += recv_size;
		if (write_size == recv_size * 30) {
			fflush(priv->infile);
			fsync(fileno(priv->infile));
			write_size = 0;
		}

		//printf("write size %lu, delta usec %f time %lli\n", recv_size, delta_usec / 1000, end_usec);

	}

exit:

	priv->recv_thread_exit = 1;

	if (recv_data)
		free(recv_data);

	recv_data = NULL;

	if (pre_file_type == PRE_FORMAT_WAVE) {
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

int pre_aec_capture(char *filename, rpbuf_arg_pre_aec *arg)
{
	int ret = 0;
	int i = 0;
	pre_aec_test_t *priv = NULL;
	snd_pcm_pre_aec_t *pcm_preaec = NULL;
	pthread_t thread_recv = 0;
	pthread_t thread_tran = 0;
	struct WAVE_FORMAT_DEF *wave_fmt = NULL;
	route_type_t route_type;
	pcm_config_t pcm_config = {0};
	aec_prms_t aec_prms = { 0 };
	unsigned long  aec_prms_offset = 0;
	unsigned long  aec_prms_size = 0;
	char *aec_codec_name = "hw:audiocodec";
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
	char *aec_daudio_name = "hw:sndi2s1";
#else
	char *aec_daudio_name = "hw:snddaudio0";
#endif
	int mode = pre_mode;

	printf("pre aec capture enter\n");

	priv = malloc(sizeof(pre_aec_test_t));
	if (priv) {
		memset(priv, 0 , sizeof(pre_aec_test_t));
		memcpy(&priv->aec_arg, arg, sizeof(rpbuf_arg_pre_aec));
	} else {
		printf("malloc pre_aec_capture failed\n");
		ret = -ENOMEM;
		goto EXIT;
	}

	priv->infile = fopen(filename, "wb+");
	if (!priv->infile) {
		printf("open file %s error!\n", filename);
		goto EXIT;
	}

	printf("here\n");

	/* 1. open_audio */

	pcm_config.channels = rprehwparams.channels;
	pcm_config.format = rprehwparams.format;
	pcm_config.rate = rprehwparams.rate;
	pcm_config.period_size = AUDIO_PRE_AEC_DEFAULT_SIZE;
	pcm_config.period_count = DEFAULT_PRE_AEC_BUF_CNT;


	route_type = MicCapture;

	ret = audio_set_record_route(aec_codec_name, 1, route_type);
	if (ret != 0) {
		printf("audio_set_playback_route fail\n");
		goto EXIT;
	}

	if (mode == 2) {

		route_type = DaudioLoopBack;
		ret = audio_set_record_route(aec_daudio_name,  1, route_type);
		if (ret != 0) {
			printf("audio_set_playback_route fail\n");
			goto EXIT;
		}

	}

	priv->pcm_dev = open_audio(pre_pcm_name, pre_stream, &pcm_config);
	if (!priv->pcm_dev) {
		printf("open %s audio fail, stream %d!\n", pre_pcm_name, pre_stream);
		goto EXIT;
	}

#ifdef DSP_AEC_TEST
	/* 2. open pre aec */
	ret = pre_aec_open(&pcm_preaec, &priv->aec_arg, pre_ctrl_id, pre_len, pre_len / 2);
	if (ret != 0) {
		printf("config_name %s aec_in_name %s aec_in_recv_name %s aec_out_name %s!\n", \
				arg->config_name, arg->aec_in_name,arg->aec_in_recv_name, arg->aec_out_name);
		goto EXIT;
	}
	priv->pcm_preaec = pcm_preaec;

	/* 3. set pre aec param */
	aec_prms.chan = rprehwparams.channels;
	aec_prms.bits_per_sample = snd_pcm_format_physical_width(rprehwparams.format);
	aec_prms.sampling_rate = rprehwparams.rate;
	aec_prms.aec_config.nlpMode = kAecNlpAggressive;
	ret = pre_aec_hw_params(priv->pcm_preaec, &aec_prms, 1);
	if (ret != 0) {
		printf("pre aec hw params fail, channels %u rate %u !\n", rprehwparams.channels, rprehwparams.rate);
		goto EXIT;
	}
	memset(&aec_prms, 0, sizeof(aec_prms_t));

	ret = pre_aec_recv(priv->pcm_preaec, priv->aec_arg.config_name, &aec_prms, &aec_prms_offset, &aec_prms_size, -1);
	if (ret != 0) {
		printf("pre aec arg size %lu\n", aec_prms_size);
		goto EXIT;
	}

	if (aec_prms.chan != rprehwparams.channels || aec_prms.bits_per_sample != snd_pcm_format_physical_width(rprehwparams.format) ||
		aec_prms.sampling_rate != rprehwparams.rate || aec_prms_size != sizeof(aec_prms_t)) {
		printf("pre_aec_recv error size %lu\n", aec_prms_size);
		goto EXIT;
	}

#endif
	/* 4. start transmit thread*/

	pthread_create(&thread_tran,
				NULL,
				aec_transmit_thread,
				(void *)priv);

	pthread_setname_np(thread_tran, "aec_transmit_thread");

#ifdef DSP_AEC_TEST

	/* 5. start recv thread	*/
	pthread_create(&thread_recv,
				NULL,
				aec_recv_thread,
				(void *)priv);

	pthread_setname_np(thread_recv, "aec_recv_thread");

#endif

	/* end the wavread thread */

	if (thread_tran)
		pthread_join(thread_tran, NULL);

	if (thread_recv)
		pthread_join(thread_recv, NULL);

	thread_tran = 0;

	thread_recv = 0;


EXIT:

	printf("pre aec capture exit\n");
	if (thread_tran)
		pthread_join(thread_tran, NULL);

	if (thread_recv)
		pthread_join(thread_recv, NULL);

	if (priv->pcm_preaec) {
		pre_aec_close(priv->pcm_preaec);
		priv->pcm_preaec = NULL;
	}

	if (priv->infile) {
		fflush(priv->infile);
		fsync(fileno(priv->infile));
		fclose(priv->infile);
		priv->infile = NULL;
	}

	if (priv->pcm_dev) {
		route_type = MicCapture;
		ret = audio_set_record_route(aec_codec_name, 0, route_type);
		if (ret != 0) {
			printf("audio_set_record_route fail\n");
		}

		if (mode == 2) {
			route_type = DaudioLoopBack;
			ret = audio_set_record_route(aec_daudio_name, 0, route_type);
			if (ret != 0) {
				printf("audio_set_record_route fail\n");
			}

		}

		ret = close_audio(priv->pcm_dev, pre_stream);
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

int pre_aec_playback(char *filename, rpbuf_arg_pre_aec *arg)
{
	int ret = 0;
	pre_aec_test_t *priv = NULL;
	snd_pcm_pre_aec_t *pcm_preaec = NULL;
	struct WAVE_FORMAT_DEF *wave_fmt = NULL;
	route_type_t route_type;
	pcm_config_t pcm_config = {0};
	char *aec_codec_name = "hw:audiocodec";
#if AUDIO_PROCESS_SUNXI_AUDIO_TWO
	char *aec_daudio_name = "hw:sndi2s1";
#else
	char *aec_daudio_name = "hw:snddaudio0";
#endif
	void *data = NULL;
	unsigned long size = pre_len;
	long length = 0;
	unsigned int frame_count = 0;
	int mode = pre_mode;
	off64_t playback_count = 0;
	off64_t written = 0;
	off64_t c = 0;
	off64_t r = 0;

	printf("pre_aec_playback enter\n");

	priv = malloc(sizeof(pre_aec_test_t));
	if (priv) {
		memset(priv, 0 , sizeof(pre_aec_test_t));
		memcpy(&priv->aec_arg, arg, sizeof(rpbuf_arg_pre_aec));
	} else {
		printf("malloc pre_aec_playback failed\n");
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

	if (pre_file_type == PRE_FORMAT_WAVE) {
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
		ret = pre_to_pcm_fmt(&priv->audio_info);
		if (ret != 0) {
			printf("pre_to_pcm_fmt failed\n");
			goto EXIT;
		}
		prehwparams.channels = wave_fmt->channels;
		prehwparams.rate = wave_fmt->samples_per_sec;
	}

	/* 1. open playback audio */

	pcm_config.channels = prehwparams.channels;
	pcm_config.format = prehwparams.format;
	pcm_config.rate = prehwparams.rate;
	pcm_config.period_size = AUDIO_PRE_AEC_DEFAULT_SIZE;
	pcm_config.period_count = DEFAULT_PRE_AEC_BUF_CNT;

	route_type = HpSpeaker;

	ret = audio_set_playback_route(aec_codec_name,  1, route_type);
	if (ret != 0) {
		printf("audio_set_playback_route fail\n");
		goto EXIT;
	}


	if (mode == 1) {

		route_type = CodecHubMode;

		ret = audio_set_playback_route(aec_codec_name,  1, route_type);
		if (ret != 0) {
			printf("audio_set_playback_route fail\n");
			goto EXIT;
		}

		route_type = DaudioHubMode;

		ret = audio_set_playback_route(aec_daudio_name,  1, route_type);
		if (ret != 0) {
			printf("audio_set_playback_route fail\n");
			goto EXIT;
		}
	}

	priv->pcm_dev = open_audio(pre_pcm_name, pre_stream, &pcm_config);
	if (!priv->pcm_dev) {
		printf("open %s audio fail, stream %d!\n", pre_pcm_name, pre_stream);
		goto EXIT;
	}

	if (mode == 1) {
		priv->pcm_daudio_dev = open_audio(aec_daudio_name, pre_stream, &pcm_config);
		if (!priv->pcm_daudio_dev) {
			printf("open %s daudio fail, stream %d!\n", aec_daudio_name, pre_stream);
			goto EXIT;
		}
	}

	/* 3. start playback */

	while (written  < playback_count) {

		c = playback_count - written;

		if (c > size)
			c = size;

		length = fread(data, 1, c, priv->playfile);
		printf("read  len %ld\n", length);
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
			printf("write to pcm %lld!=%u\n", r, frame_count);
			break;
		}
		r = snd_pcm_frames_to_bytes(priv->pcm_dev, r);
		written += r;
	}

EXIT:

	printf("pre aec playback exit\n");

	if (priv->pcm_dev) {

		ret = close_audio(priv->pcm_dev, pre_stream);
		if (ret != 0) {
			printf("close_audio fail\n");
		}
		priv->pcm_dev = NULL;

		if (priv->pcm_daudio_dev) {
			ret = close_audio(priv->pcm_daudio_dev, pre_stream);
			if (ret != 0) {
				printf("pcm_daudio_dev fail\n");
			}
			priv->pcm_daudio_dev = NULL;
		}

		route_type = HpSpeaker;

		ret = audio_set_playback_route(aec_codec_name,  0, route_type);
		if (ret != 0) {
			printf("audio_set_playback_route fail\n");
		}

		if (mode == 1) {

			route_type = CodecHubMode;

			ret = audio_set_playback_route(aec_codec_name,  0, route_type);
			if (ret != 0) {
				printf("audio_set_playback_route fail\n");
				goto EXIT;
			}

			route_type = DaudioHubMode;

			ret = audio_set_playback_route(aec_daudio_name, 0, route_type);
			if (ret != 0) {
				printf("audio_set_playback_route fail\n");
				goto EXIT;
			}

		}

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

static void print_pre_aec_help_msg(void)
{
	printf("\n");
	printf("USAGE:\n");
	printf("  pre_aec_test [OPTIONS]\n");
	printf("\n");
	printf("OPTIONS:\n");
	printf("  -D          : select PCM by name\n");
	printf("  -t          : sfile type (voc, wav, raw or au)\n");
	printf("  -c          : channels\n");
	printf("  -f          : sample format (case insensitive)\n");
	printf("  -r          : sample rate\n");
	printf("  -d          : interrupt after # seconds\n");
	printf("  -m          : set mode (0: disable, 1: tx_hub (Synchronized playback) 2: loopback\n");
	printf("  -g          : rpbuf config_name (default: rpbuf_aec_config)\n");
	printf("  -i          : rpbuf aec_in_name (default: rpbuf_aec_in)\n");
	printf("  -o          : rpbuf aec_out_name (default: rpbuf_aec_out)\n");
	printf("  -I ID       : specify rpbuf ctrl ID (default: 0)\n");
	printf("  -L LENGTH   : specify buffer length (default: %d bytes)\n",
			PRE_RPBUF_BUFFER_LENGTH_DEFAULT);
	printf("  -P          : PlayBack\n");
	printf("  -C          : Capture\n");
	printf("\n");
	printf("e.g.\n");
	printf("  pre_aec_test -Dhw:audiocodec -L 1280 -t wav -m 1 -P /data/test.wav \n");
	printf("  pre_aec_test -DCaptureMulti1 -L 320 -f S16_LE -r 16000 -c 2 -t wav -d 15 -m 2 -C /data/test_aec_in.wav \n");
	printf("  pre_aec_test -DCaptureMulti1 -L 320 -f S16_LE -r 16000 -c 2 -m 2 -C /data/test_aec_in.pcm \n");
	printf("  pre_aec_test -DCaptureMulti1 -L 320 -g \"rp_aec_config\" -i \"rp_aec_ref_in\" -o \"rp_aec_out\" -f S16_LE -r 16000 -c 2 -t wav -m 2 -C /data/test_aec_in.wav\n");
	printf("\n");
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

	rpbuf_arg_pre_aec targ = {
		.config_name      = "rpbuf_aec_config",
		.aec_in_name      = "rpbuf_aec_in",
		.aec_in_recv_name = "rpbuf_aec_in_recv",
		.aec_out_name     = "rpbuf_aec_out",
	};

	pre_ctrl_id = 0;
	pre_mode = 0;
	pre_file_type = PRE_FORMAT_DEFAULT;
	rprehwparams.format = PRE_DEFAULT_FORMAT;
	rprehwparams.rate = PRE_DEFAULT_SPEED;
	rprehwparams.channels = PRE_DEFAULT_CHANNEL;
	pre_len = PRE_RPBUF_BUFFER_LENGTH_DEFAULT;

	while ((c = getopt(argc, argv, "hD:t:c:f:r:d:m:g:i:o:I:L:PC")) != -1) {
		switch (c) {
		case 'h':
			print_pre_aec_help_msg();
			ret = 0;
			goto out;
		case 'D':
			pre_pcm_name = optarg;
			break;
		case 't':
			if (strcasecmp(optarg, "raw") == 0)
				pre_file_type = PRE_FORMAT_RAW;
			else if (strcasecmp(optarg, "voc") == 0)
				pre_file_type = PRE_FORMAT_VOC;
			else if (strcasecmp(optarg, "wav") == 0)
				pre_file_type = PRE_FORMAT_WAVE;
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
		case 'd':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				ret = -EINVAL;
				goto out;
			}
			pre_timelimit = long_tmp;
			break;
		case 'm':
			pre_mode = strtol(optarg, NULL, 0);
			if (pre_mode < 0 || pre_mode == LONG_MAX) {
				printf("value %d for mode is invalid", pre_mode);
				return 1;
			}
			break;
		case 'g':
			strncpy(targ.config_name, optarg, sizeof(targ.config_name));
			break;
		case 'i':
			strncpy(targ.aec_in_name, optarg, sizeof(targ.aec_in_name));
			break;
		case 'o':
			strncpy(targ.aec_out_name, optarg, sizeof(targ.aec_out_name));
			break;
		case 'P':
			pre_stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'C':
			pre_stream = SND_PCM_STREAM_CAPTURE;
			break;
		case 'I':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				ret = -EINVAL;
				goto out;
			}
			pre_ctrl_id = long_tmp;
			break;
		case 'L':
			long_tmp = strtol(optarg, NULL, 0);
			if (long_tmp == LONG_MIN || long_tmp == LONG_MAX) {
				printf("Invalid input %s.\n", optarg);
				ret = -EINVAL;
				goto out;
			}
			pre_len = long_tmp;
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

	if (pre_stream == SND_PCM_STREAM_PLAYBACK) {
		pre_aec_playback(argv[optind++], &targ);
	} else {
		pre_aec_capture(argv[optind++], &targ);
	}

	return ret;


out:
	print_pre_aec_help_msg();
	return ret;

}




