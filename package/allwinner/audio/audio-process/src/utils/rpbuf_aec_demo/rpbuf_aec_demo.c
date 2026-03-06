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

#include "rpbuf_audio_ctrl_aec.h"

#include "rpbuf_audio_aec.h"
#include "audio_common.h"
#include "thread_sem.h"
//#include "nfecnr_parser.h"

#define AUDIO_RPBUF_AEC_DEFAULT_SIZE 1024
#define DEFAULT_RPBUF_AEC_BUF_CNT 4

#define RPBUF_AEC_PCM_NAME_DEFAULT "default"

#define RPBUF_PLAYBACK_LENGTH_DEFAULT 1024//byte

#define RPBUF_AEC_OUT_CHANNEL 2

#define RPBUF_DEFAULT_FORMAT		SND_PCM_FORMAT_S16_LE
#define RPBUF_DEFAULT_SPEED 		16000
#define RPBUF_DEFAULT_CHANNEL 		2
#define RPBUF_DEFAULT_MSEC 			64
#define RPBUF_DEFAULT_LMS_MSEC 		0

#define RPBUF_FORMAT_DEFAULT		-1
#define RPBUF_FORMAT_RAW		0
#define RPBUF_FORMAT_VOC		1
#define RPBUF_FORMAT_WAVE		2


/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092

#define DSP_AEC_TEST
#define RPBUF_DSP_AEC_DUMP_INPUT_DATA

#define RPBUF_DSP_AEC_DOWNLINK_TEST
//#define RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
#define AEC_DOWNLINK_DUMP_FILE

struct hwparams
{
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int rate;
    unsigned int msec;
    unsigned int lms_msec;
};

static struct hwparams prehwparams, rprehwparams, rprehwparamspath;

static snd_pcm_stream_t rpbuf_stream = SND_PCM_STREAM_CAPTURE;
static int rpbuf_file_type = RPBUF_FORMAT_DEFAULT;
static int rpbuf_playback_file_type = RPBUF_FORMAT_DEFAULT;
static int rpbuf_mode = 0;
static long rpbuf_timelimit = 0;
static char *rpbuf_pcm_name = RPBUF_AEC_PCM_NAME_DEFAULT;
static char *rpbuf_pcm_playback_name = RPBUF_AEC_PCM_NAME_DEFAULT;
static char dumppath[32] = "/tmp/dump.wav";
static char dumplmspath[32] = "/tmp/test_aec_lms.pcm";
static char dumpaecdownlink[32] = "/tmp/aec_downlink.pcm";
static char aecuplink[32] = "/tmp/test_aec_out.pcm";
static char PE5_output_low[32]  = "PE5 1 0 1 0";
static char PE5_output_high[32] = "PE5 1 1 1 0";
static char PE4_output_low[32]  = "PE4 1 0 1 0";
static char PE4_output_high[32] = "PE4 1 1 1 0";
static char sunix_pin[64] = "/sys/kernel/debug/sunxi_pinctrl/sunxi_pin_configure";
static volatile sig_atomic_t in_aborting = 0;

static uint64_t rpbuf_rec_count = LLONG_MAX;


/* RIFF HEADER of wav file */
struct RIFF_HEADER_DEF
{
    char riff_id[4];     /* 'R', 'I', 'F', 'F' */
    uint32_t riff_size;
    char riff_format[4]; /* 'W', 'A', 'V', 'E' */
};

struct WAVE_FORMAT_DEF
{
    uint16_t format_tag;
    uint16_t channels; /* channels of the audio stream */
    uint32_t samples_per_sec; /* sample rate */
    uint32_t avg_bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample; /* bits per sample */
};

struct FMT_BLOCK_DEF
{
    char fmt_id[4];    /* 'f', 'm', 't', ' ' */
    uint32_t fmt_size;
    struct WAVE_FORMAT_DEF wav_format;
};

struct DATA_BLOCK_DEF
{
    char data_id[4];     /* 'd', 'a', 't', 'a' */
    uint32_t data_size;
};

struct wav_info
{
    struct RIFF_HEADER_DEF header;
    struct FMT_BLOCK_DEF   fmt_block;
    struct DATA_BLOCK_DEF  data_block;
};

typedef struct rpbuf_aec_ctx
{
    snd_pcm_t *pcm_dev;
    rpbuf_audio_aec_ctx_t *rpbuf_audio_ctx;
    rpbuf_audio_remote_ctrl_aec_t *rpbuf_ctl_arg;
    FILE *playfile;
    FILE *infile;
    int pin_fd;
    char aec_in_name[32];
    char aec_out_name[32];
    char aec_downlink_in_name[32];
    char aec_downlink_out_name[32];
    unsigned int rpbuf_aec_in_len;
    unsigned int rpbuf_aec_out_len;
    unsigned int rpbuf_aec_downlink_in_len;
    unsigned int rpbuf_aec_downlink_out_len;
    struct wav_info audio_info;
} rpbuf_aec_ctx_t;

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
    {
        big_endian = 0;
    }
    else if (!strncmp(wav_info->header.riff_id, "RIFX", sizeof(wav_info->header.riff_id)))
    {
        big_endian = 1;
    }

    if (strncmp(wav_info->header.riff_format, "WAVE", sizeof(wav_info->header.riff_format)))
    {
        printf("riff_format %s is err\n", wav_info->header.riff_format);
        return -1;
    }

    switch (bit)
    {
        case 8:
            if (prehwparams.format != RPBUF_DEFAULT_FORMAT &&
                    prehwparams.format != SND_PCM_FORMAT_U8)
            {
                printf("Warning: format is changed to U8\n");
            }
            prehwparams.format = SND_PCM_FORMAT_U8;
            break;
        case 16:
            if (big_endian)
            {
                native_format = SND_PCM_FORMAT_S16_BE;
            }
            else
            {
                native_format = SND_PCM_FORMAT_S16_LE;
            }
            if (prehwparams.format != RPBUF_DEFAULT_FORMAT &&
                    prehwparams.format != native_format)
                printf("Warning: format is changed to %s\n", \
                       snd_pcm_format_name(native_format));
            prehwparams.format = native_format;
            break;
        case 24:
            switch (bit / prehwparams.channels)
            {
                case 3:
                    if (big_endian)
                    {
                        native_format = SND_PCM_FORMAT_S24_3BE;
                    }
                    else
                    {
                        native_format = SND_PCM_FORMAT_S24_3LE;
                    }
                    if (prehwparams.format != RPBUF_DEFAULT_FORMAT &&
                            prehwparams.format != native_format)
                        printf("Warning: format is changed to %s\n", \
                               snd_pcm_format_name(native_format));
                    prehwparams.format = native_format;
                    break;
                case 4:
                    if (big_endian)
                    {
                        native_format = SND_PCM_FORMAT_S24_BE;
                    }
                    else
                    {
                        native_format = SND_PCM_FORMAT_S24_LE;
                    }
                    if (prehwparams.format != RPBUF_DEFAULT_FORMAT &&
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
            if (format == WAV_FMT_PCM)
            {
                if (big_endian)
                {
                    native_format = SND_PCM_FORMAT_S32_BE;
                }
                else
                {
                    native_format = SND_PCM_FORMAT_S32_LE;
                }
                prehwparams.format = native_format;
            }
            else if (format == WAV_FMT_IEEE_FLOAT)
            {
                if (big_endian)
                {
                    native_format = SND_PCM_FORMAT_FLOAT_BE;
                }
                else
                {
                    native_format = SND_PCM_FORMAT_FLOAT_LE;
                }
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

    if (rpbuf_timelimit == 0)
    {
        count = rpbuf_rec_count;
    }
    else
    {
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

    while (offset < count)
    {
        written = fwrite((char *)buf + offset, 1, count - offset, file);
        if (written <= 0)
        {
            return written;
        }

        offset += written;
    }

    return offset;
}

/* write a WAVE-header */
static int start_wave_header(FILE *file, size_t cnt, struct hwparams *rprehwparams)
{
    struct RIFF_HEADER_DEF head;
    struct FMT_BLOCK_DEF f;
    struct DATA_BLOCK_DEF cd;
    int bits;
    u_int tmp;
    u_short tmp2;

    /* WAVE cannot handle greater than 32bit (signed?) int */
    if (cnt == (size_t) -2)
    {
        cnt = 0x7fffff00;
    }

    bits = 8;
    switch ((unsigned long) rprehwparams->format)
    {
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
    {
        f.wav_format.format_tag = WAV_FMT_IEEE_FLOAT;
    }
    else
    {
        f.wav_format.format_tag = WAV_FMT_PCM;
    }

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
            writefile(file, &cd, sizeof(struct DATA_BLOCK_DEF)) != sizeof(struct DATA_BLOCK_DEF))
    {
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
    {
        writefile(file, &rifflen, 4);
    }
    if (fseek(file, length_seek, SEEK_SET) == length_seek)
    {
        writefile(file, &cd, sizeof(struct DATA_BLOCK_DEF));
    }

    fseek(file, 0, SEEK_END);

}

#ifdef CONFIG_COMPONENT_NF_AEC
static void rpbuf_print_aec_pararms(tdspCfgCore *prms)
{
    tCfgAec *aec_params = &prms->aec;

    printf("enbale: %d, RefDelay: %d, InFgDelay: %d InBgDelay %d InGaindB %d RefGaindB %d OutGaindB %d\n", \
           aec_params->enable, aec_params->RefDelay, aec_params->InFgDelay, aec_params->InBgDelay, \
           aec_params->InGaindB, aec_params->RefGaindB, aec_params->OutGaindB);

}
#endif

int rpbuf_aec_config(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;
    int i = 0;

    /* config algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_CONFIG;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;

    priv->rpbuf_ctl_arg->channels = rprehwparams.channels;
    priv->rpbuf_ctl_arg->bits = snd_pcm_format_physical_width(rprehwparams.format);
    priv->rpbuf_ctl_arg->rate = rprehwparams.rate;
    priv->rpbuf_ctl_arg->msec = rprehwparams.msec;
    priv->rpbuf_ctl_arg->lms_msec = rprehwparams.lms_msec;
#ifdef RPBUF_DSP_AEC_DUMP_INPUT_DATA
    if (priv->rpbuf_ctl_arg->lms_msec)
        priv->rpbuf_ctl_arg->out_channels = RPBUF_AEC_OUT_CHANNEL;
    else
        priv->rpbuf_ctl_arg->out_channels = 1;
#else
        priv->rpbuf_ctl_arg->out_channels = 1;
#endif

#ifdef CONFIG_COMPONENT_NF_AEC
    tdspCfgCore *dspCfgCore = &priv->rpbuf_ctl_arg->dspCfgCore;

    /* aec config */
    ret = aec_params_parser(dspCfgCore, priv->rpbuf_ctl_arg->rate);
    if (ret != 0)
    {
        printf("aec_params_parser failed\n");
        return ret;
    }

    rpbuf_print_aec_pararms(dspCfgCore);
#endif
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_CONFIG);
        return ret;
    }


    return 0;
}

int rpbuf_aec_init(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* init rpbuf audio */
    priv->rpbuf_audio_ctx = rpbuf_audio_aec_init(0);
    if (priv->rpbuf_audio_ctx == NULL)
    {
        printf("rpbuf_audio_init failed\n");
        goto EXIT;
    }

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_ENABLE_ALG;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    priv->rpbuf_ctl_arg->value = 1;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_ENABLE_ALG);
        goto EXIT;
    }

    #ifdef RPBUF_DSP_AEC_DUMP_INPUT_DATA
    /* enable raw data dump */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_DUMP_MERGE_DATA;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    priv->rpbuf_ctl_arg->value = 1;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_DUMP_MERGE_DATA);
        goto EXIT;
    }
    #endif

    /* enable init */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_INIT;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_INIT);
        goto EXIT;
    }

    /* enable aec rpbuf transfer */
    ret = rpbuf_audio_aec_data_init(priv->rpbuf_audio_ctx, RPBUF_AEC_IN_LENGTH, RPBUF_AEC_OUT_LENGTH);
    if (ret != 0)
    {
        printf("%s %d rpbuf_audio_aec_data_init failed", __func__, __LINE__);
        goto EXIT;
    }

    ret = rpbuf_audio_aec_get_data_name(priv->rpbuf_audio_ctx, priv->aec_in_name, priv->aec_out_name);
    if (ret != 0)
    {
        printf("%s %d rpbuf_audio_aec_get_data_name failed", __func__, __LINE__);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_release(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_RELEASE;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_RELEASE);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_start(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_START;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_START);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_stop(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_STOP;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_aec_ctrl_send_cmd %d failed\n", RPBUF_CTL_STOP);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_downlink_config(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;
    int i = 0;

    /* config algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_CONFIG;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;

    priv->rpbuf_ctl_arg->channels = prehwparams.channels;
    priv->rpbuf_ctl_arg->out_channels = prehwparams.channels;
    priv->rpbuf_ctl_arg->bits = snd_pcm_format_physical_width(prehwparams.format);
    priv->rpbuf_ctl_arg->rate = prehwparams.rate;
    priv->rpbuf_ctl_arg->msec = prehwparams.msec;

#ifdef CONFIG_COMPONENT_NF_AEC
    tdspCfgCore *dspCfgCore = &priv->rpbuf_ctl_arg->dspCfgCore;

    /* aec config */
    ret = aec_params_parser(dspCfgCore, priv->rpbuf_ctl_arg->rate);
    if (ret != 0)
    {
        printf("aec_params_parser failed\n");
        return ret;
    }

    rpbuf_print_aec_pararms(dspCfgCore);
#endif

    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_CONFIG);
        return ret;
    }


    return 0;
}

int rpbuf_aec_downlink_init(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* init rpbuf audio */
    priv->rpbuf_audio_ctx = rpbuf_audio_aec_init(0);
    if (priv->rpbuf_audio_ctx == NULL)
    {
        printf("rpbuf_audio_init failed\n");
        goto EXIT;
    }

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_ENABLE_ALG;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    priv->rpbuf_ctl_arg->value = 1;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_ENABLE_ALG);
        goto EXIT;
    }

    #ifdef RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
    /* enable raw data dump */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_DUMP_MERGE_DATA;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    priv->rpbuf_ctl_arg->value = 1;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_DUMP_MERGE_DATA);
        goto EXIT;
    }
    #endif

    /* enable init */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_INIT;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_INIT);
        goto EXIT;
    }

    /* enable aec downlink rpbuf transfer */
    ret = rpbuf_audio_aec_downlink_data_init(priv->rpbuf_audio_ctx, RPBUF_AEC_DOWNLINK_IN_LENGTH, RPBUF_AEC_DOWNLINK_OUT_LENGTH);
    if (ret != 0)
    {
        printf("%s %d rpbuf_audio_aec_downlink_data_init failed", __func__, __LINE__);
        goto EXIT;
    }

    ret = rpbuf_audio_aec_downlink_get_data_name(priv->rpbuf_audio_ctx, priv->aec_downlink_in_name, priv->aec_downlink_out_name);
    if (ret != 0)
    {
        printf("%s %d rpbuf_audio_aec_downlink_get_data_name failed", __func__, __LINE__);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_downlink_release(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_RELEASE;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_RELEASE);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_downlink_start(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_START;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_START);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

int rpbuf_aec_downlink_stop(rpbuf_aec_ctx_t *priv)
{
    int ret = -1;

    /* enable whole algorithms */
    memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    priv->rpbuf_ctl_arg->cmd = RPBUF_CTL_STOP;
    priv->rpbuf_ctl_arg->type = RPBUF_ALGO_AEC_DOWNLINK;
    ret = rpbuf_audio_aec_ctrl_send_cmd(priv->rpbuf_audio_ctx, priv->rpbuf_ctl_arg, 0);
    if (ret != 0)
    {
        printf("rpbuf_audio_ctrl_send_cmd %d failed\n", RPBUF_CTL_STOP);
        goto EXIT;
    }

    return 0;

EXIT:

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }
    return ret;
}

static int pcm_s16le_split_to_plane(short *dst, short *src, struct hwparams *hw_params, unsigned int chan, unsigned int len)
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
    rate	 = hw_params->rate;

    if (channels == 0 || bitwidth == 0 || rate == 0)
    {
        printf("input param error, rate %d, ch %d, width %d\n", \
               rate, channels, bitwidth);
        return -1;
    }

    BytesPerSample = bitwidth * channels / 8;

    nSampleCount = len / BytesPerSample;

    for (i = 0; i < nSampleCount; ++i)
    {
        for (j = 0; j < channels; j++)
        {
            dst[i + nSampleCount * j] = src[channels * i + j];
        }
    }

    return 0;
}

static int pcm_s16le_plane_to_split(short *dst, short *lms_dst, short *src, struct hwparams *hw_params, unsigned int chan, unsigned int len)
{
    uint32_t rate = 0;
    uint32_t channels = 0;
    uint8_t bitwidth = 0;
    uint32_t BytesPerSample = 0;
    uint32_t LmsBytesPerSample = 0;
    short nSampleCount = 0;
    short nLmsSampleCount = 0;
    int i = 0;
    int j = 0;
    int in_len = 0;
    int out_len = 0;

    channels = chan;
    bitwidth = snd_pcm_format_physical_width(hw_params->format);
    rate     = hw_params->rate;

    if (channels == 0 || bitwidth == 0 || rate == 0)
    {
        printf("input param error, rate %d, ch %d, width %d\n", \
               rate, channels, bitwidth);
        return -1;
    }

    if (hw_params->lms_msec == 0)
    {
        BytesPerSample = bitwidth * channels / 8;
        nSampleCount = len / BytesPerSample;

        for (i = 0; i < nSampleCount; ++i)
        {
            for (j = 0; j < channels; j++)
            {
                dst[channels * i + j] = src[i + nSampleCount * j];
            }
        }
    }
    else
    {
        BytesPerSample = bitwidth * (channels - 1) / 8;
        LmsBytesPerSample = bitwidth * 1 / 8;

        in_len = (BytesPerSample * rate * hw_params->msec) / 1000;
        out_len = (LmsBytesPerSample * rate * hw_params->lms_msec) / 1000;

        if ((in_len + out_len) != len || lms_dst == NULL)
        {
            printf("input param error, in_len %d, out_len %d, len %d, lms_dst %p\n", \
                   in_len, out_len, len, lms_dst);
            return -1;
        }

        nSampleCount = in_len / BytesPerSample;
        nLmsSampleCount = out_len / LmsBytesPerSample;

        /* dst store 3ch(1mic + 1ref + 1aec) */
        for (i = 0; i < nSampleCount; ++i)
        {
            for (j = 0; j < channels - 1; j++)
            {
                dst[(channels - 1) * i + j] = src[i + nSampleCount * j];
            }
        }

        /* lms_dst store 1ch(lms) */
        for (i = 0; i < nLmsSampleCount; ++i)
        {
            lms_dst[i] = src[i + nSampleCount * (channels - 1)];
        }
    }
    return 0;
}

static void *rpbuf_aec_transmit_thread(void *params)
{
    rpbuf_aec_ctx_t *priv = (rpbuf_aec_ctx_t *)params;
    int ret = 0;
    void *data = NULL;
    void *recv_data = NULL;
    void *out_data = NULL;
    void *lms_data = NULL;
    FILE *dumpfile = NULL;
    FILE *dumplmsfile = NULL;
    unsigned long size = priv->rpbuf_aec_in_len;
    unsigned long out_size = priv->rpbuf_aec_out_len;
    unsigned long recv_size = 0;
    unsigned long write_file_size = 0;
    unsigned long recv_data_offset = 0;
    //long long start_usec, end_usec;
    //float delta_usec;
    snd_pcm_sframes_t length;
    int timeout = 300;
    int64_t count = 0;
    uint64_t fdcount = 0;
    uint64_t outfdcount = 0;
    uint64_t outlmsfdcount = 0;
    unsigned int frame_count = 0;
    int cnt = 0;
    int aec_out_channels = 0;
    int aec_tmp_size = 0;
    int aec_lms_tmp_size = 0;
    rpbuf_audio_remote_ctrl_aec_t rpbuf_aec_ctrl_arg;

#ifdef RPBUF_DSP_AEC_DUMP_INPUT_DATA
    if (rprehwparams.lms_msec)
        aec_out_channels = RPBUF_AEC_OUT_CHANNEL; /* if chan > 2, lms will be dump, its len will be calc by lmsmsec */
    else
        aec_out_channels = 1;
#else
        aec_out_channels = 1;
#endif

    data = malloc(size);
    if (!data)
    {
        printf("no memory\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(data, 0, size);

    recv_data = malloc(out_size);
    if (!recv_data)
    {
        printf("no memory\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(recv_data, 0, out_size);

    if (aec_out_channels > 1)
    {
        aec_tmp_size = size / rprehwparams.channels * (rprehwparams.channels + aec_out_channels - 1);
        out_data = malloc(aec_tmp_size);
        if (!out_data)
        {
            printf("no memory\n");
            ret = -ENOMEM;
            goto exit;
        }
        memset(out_data, 0, aec_tmp_size);

        aec_lms_tmp_size = out_size - aec_tmp_size;
        lms_data = malloc(aec_lms_tmp_size);
        if (!lms_data)
        {
            printf("no memory\n");
            ret = -ENOMEM;
            goto exit;
        }
        memset(lms_data, 0, aec_lms_tmp_size);

        dumplmsfile = fopen(dumplmspath, "wb+");
        if (!dumplmsfile)
        {
            printf("open file %s error!\n", dumplmspath);
            goto exit;
        }
    }
    else
    {
        out_data = malloc(out_size);
        if (!out_data)
        {
            printf("no memory\n");
            ret = -ENOMEM;
            goto exit;
        }
        memset(out_data, 0, out_size);
    }

    dumpfile = fopen(dumppath, "wb+");
    if (!dumpfile)
    {
        printf("open file %s error!\n", dumppath);
        goto exit;
    }

    if (rpbuf_file_type == RPBUF_FORMAT_WAVE)
    {
        /* aec in has two channels, left channel is mic in ,right channel is ref, but aec out only have one channel. */
        rprehwparamspath.format = rprehwparams.format;
        rprehwparamspath.rate =  rprehwparams.rate;

        if (aec_out_channels > 1)
            rprehwparamspath.channels = rprehwparams.channels + aec_out_channels - 1;
        else
            rprehwparamspath.channels = rprehwparams.channels + aec_out_channels;

        count = calc_data_count();
        if (count == 0)
        {
            count = LLONG_MAX;
        }

        if (count < LLONG_MAX)
        {
            count += count % 2;
        }
        else
        {
            count -= count % 2;
        }

        printf("count %ld\n", count);

        start_wave_header(priv->infile, count, &rprehwparamspath);

        if (aec_out_channels > 1) {
            rprehwparamspath.channels = 1;
            start_wave_header(dumplmsfile, count, &rprehwparamspath);
        }
    }

    count = calc_data_count();
    if (count == 0)
    {
        count = LLONG_MAX;
    }

    if (count < LLONG_MAX)
    {
        count += count % 2;
    }
    else
    {
        count -= count % 2;
    }

    start_wave_header(dumpfile, count, &rprehwparams);

	printf("transmit count %ld\n", count);

    ret = rpbuf_aec_start(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_start %d error!\n", ret );
        goto exit;
    }

    do {
        memset(&rpbuf_aec_ctrl_arg, 0 ,sizeof(rpbuf_audio_remote_ctrl_aec_t));
        ret = rpbuf_audio_aec_ctrl_recv_cmd(priv->rpbuf_audio_ctx, &rpbuf_aec_ctrl_arg, &recv_data_offset, &recv_size, 1);
        if (ret != 0 || (recv_size > sizeof(rpbuf_audio_remote_ctrl_aec_t)))
        {
            if (ret == -ETIMEDOUT)
            {
                printf("wait remote ack timeout!\n");
                continue;
            }
            else
            {
                printf("rpbuf_aec_start ack %d error!\n", ret);
                goto exit;
            }
        }
    } while (ret);

    if (rpbuf_aec_ctrl_arg.cmd == RPBUF_CTL_START_ACK && rpbuf_aec_ctrl_arg.type == RPBUF_ALGO_AEC && \
        rpbuf_aec_ctrl_arg.value == RPBUF_ERROR_NONE)
    {
        printf("dsp has start finished\n");
    }
    else
    {
        printf("rpbuf aec get start ack failed cmd %d type %d value %d exit thread\n", \
              rpbuf_aec_ctrl_arg.cmd, rpbuf_aec_ctrl_arg.type, rpbuf_aec_ctrl_arg.value);
        goto stop;
    }


    while (count > 0 && !in_aborting)
    {

        frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, size);

        ret = audio_read(priv->pcm_dev, data, frame_count);
        if (ret < 0)
        {
            if (ret = -EPIPE)
            {
                printf("audio read overrun, size %lu\n", size);
            }
            else
            {
                printf("audio read err, size %lu\n", size);
                break;
            }
        }
        //printf("audio_read %u\n", frame_count);

        count -= size;

        write_file_size = writefile(dumpfile, data, size);

        if (write_file_size != size)
        {
            printf("write %lu size %lu error!\n", write_file_size, size);
            break;
        }

        fdcount += size;
        /*
        write_size += size;
        if (write_size == size * 50) {
            fflush(dumpfile);
            fsync(fileno(dumpfile));
            write_size = 0;
        }
        */
        //printf("read size %lu frame_count %u delta usec %f time %lli\n", size, frame_count, delta_usec / 1000, end_usec);

        #ifdef DSP_AEC_TEST

        //start_usec = get_currenttime();
        #ifdef RPBUF_SPEED_TEST
        ret = write(priv->pin_fd, PE4_output_low, strlen(PE4_output_low));
        if (ret != strlen(PE4_output_low))
        {
            printf("cmd: %s\t error: %s", PE4_output_low, strerror(errno));
            break;
        }
        #endif

        /* transfer data to dsp */
        length = rpbuf_audio_aec_transfer_data(priv->rpbuf_audio_ctx, priv->aec_in_name, data, 0, size);
        if (length < 0)
        {
            printf("rpbuf_aec_transfer error send_len %lu!\n", length);
            break;
        }

        #ifdef RPBUF_SPEED_TEST
        ret = write(priv->pin_fd, PE4_output_high, strlen(PE4_output_high));
        if (ret != strlen(PE4_output_high))
        {
            printf("cmd: %s\t error: %s", PE4_output_high, strerror(errno));
            break;
        }
        #endif

        #ifdef RPBUF_SPEED_TEST
        ret = write(priv->pin_fd, PE5_output_low, strlen(PE5_output_low));
        if (ret != strlen(PE5_output_low))
        {
            printf("cmd: %s\t error: %s", PE5_output_low, strerror(errno));
            break;
        }
        #endif

        cnt = 0;
        do {
            ret = rpbuf_audio_aec_recv_data(priv->rpbuf_audio_ctx, priv->aec_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
            if (ret != 0)
            {
                if (ret ==  -ETIMEDOUT) {
                    printf("rpbuf aec uplink recv timeout %d\n", timeout);
                    cnt++;
                    if (cnt > 3) {
                        cnt = 0;
                        break;
                    }
                    continue;
                }
                else {
                    printf("rpbuf aec recv size %lu\n", recv_size);
                    break;
                }
            }
        } while(0);

        #ifdef RPBUF_SPEED_TEST
        ret = write(priv->pin_fd, PE5_output_high, strlen(PE5_output_high));
        if (ret != strlen(PE5_output_high))
        {
            printf("cmd: %s\t error: %s", PE5_output_high, strerror(errno));
            break;
        }
        #endif

        if (recv_size != 0xFFFFFFFF && recv_size > out_size)
        {
            printf("aec recv data error recv size %ld size %ld\n", recv_size, out_size);
            break;
        }

        if (recv_size != 0xFFFFFFFF) {
            if (lms_data == NULL) {
                pcm_s16le_plane_to_split(out_data, NULL, recv_data, &rprehwparams, rprehwparams.channels + aec_out_channels, recv_size);
                if (writefile(priv->infile, out_data, recv_size) != recv_size)
                {
                    printf("write size %lu error!\n", recv_size);
                    break;
                }
                outfdcount += recv_size;
            }
            else
            {
                pcm_s16le_plane_to_split(out_data, lms_data, recv_data, &rprehwparams, rprehwparams.channels + aec_out_channels, recv_size);
                if (writefile(priv->infile, out_data, aec_tmp_size) != aec_tmp_size)
                {
                    printf("write size %d error!\n", aec_tmp_size);
                    break;
                }
                outfdcount += aec_tmp_size;

                if (writefile(dumplmsfile, lms_data, aec_lms_tmp_size) != aec_lms_tmp_size)
                {
                    printf("lms write size %d error!\n", aec_lms_tmp_size);
                    break;
                }
                outlmsfdcount += aec_lms_tmp_size;
            }
        }
        //end_usec = get_currenttime();
        //delta_usec = end_usec - start_usec;
        //printf("aec in size %lu, delta msec %f\n", recv_size, delta_usec / 1000);
        #endif
    }

stop:

    ret = rpbuf_aec_stop(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_stop %d error!\n", ret);
        goto exit;
    }

exit:

    if (dumpfile)
    {
        printf("fdcount %lu\n", fdcount);
        end_wave(dumpfile, fdcount);
        fflush(dumpfile);
        fsync(fileno(dumpfile));
        fclose(dumpfile);
        dumpfile = NULL;
    }

    if (recv_data)
    {
        free(recv_data);
        recv_data = NULL;
    }

    if (out_data)
    {
        free(out_data);
        out_data = NULL;
    }

    if (lms_data)
    {
        free(lms_data);
        lms_data = NULL;
    }

    if (rpbuf_file_type == RPBUF_FORMAT_WAVE)
    {
        printf("outfdcount %lu outlmsfdcount %lu\n", outfdcount, outlmsfdcount);
        end_wave(priv->infile, outfdcount);
		end_wave(dumplmsfile, outlmsfdcount);
    }

    if (priv->infile)
    {
        fflush(priv->infile);
        fsync(fileno(priv->infile));
        fclose(priv->infile);
        priv->infile = NULL;
    }

    if (dumplmsfile)
    {
        fflush(dumplmsfile);
        fsync(fileno(dumplmsfile));
        fclose(dumplmsfile);
        dumplmsfile = NULL;
    }

    if (data)
    {
        free(data);
        data = NULL;
    }

    printf("end of transmit thread\n");
    return NULL;
}

int rpbuf_aec_capture(void *params)
{
    int ret = 0;
    rpbuf_aec_ctx_t *priv = (rpbuf_aec_ctx_t *)params;;
    pthread_t thread_tran = 0;
    struct WAVE_FORMAT_DEF *wave_fmt = NULL;
    route_type_t route_type;
    pcm_config_t pcm_config = {0};
    char *aec_codec_name = "hw:audiocodec";
    char *aec_daudio_name = "hw:snddaudio2";
    int frame_bytes = 0;
    int lms_frame_sample = 0;
    int aec_out_channels = 0;

    printf("pre aec capture enter\n");

    #ifdef RPBUF_SPEED_TEST
    priv->pin_fd = open(sunix_pin, O_RDWR);
    if (priv->pin_fd == -1)
    {
        printf("open file %s error!\n", sunix_pin);
        goto EXIT;
    }
    #endif

    printf("here\n");

    frame_bytes = snd_pcm_format_physical_width(rprehwparams.format) / 8 * rprehwparams.channels;

    lms_frame_sample = (rprehwparams.rate * rprehwparams.lms_msec) / 1000;

    priv->rpbuf_aec_in_len = (frame_bytes * rprehwparams.rate * rprehwparams.msec) / 1000; /* 4 * 16 * 64 = 4096 bytes */

    #ifdef RPBUF_DSP_AEC_DUMP_INPUT_DATA
    if (lms_frame_sample && RPBUF_AEC_OUT_CHANNEL > 1)
        aec_out_channels = RPBUF_AEC_OUT_CHANNEL - 1; /* if chan > 2, lms will be dump, its len will be calc by lmsmsec */
    else
        aec_out_channels = 1;
    #else
        aec_out_channels = 1;
    #endif

    #ifdef RPBUF_DSP_AEC_DUMP_INPUT_DATA
    if (lms_frame_sample)
        priv->rpbuf_aec_out_len = (priv->rpbuf_aec_in_len / rprehwparams.channels) * \
            (rprehwparams.channels + aec_out_channels) + (lms_frame_sample * snd_pcm_format_physical_width(rprehwparams.format) / 8);
    else
        priv->rpbuf_aec_out_len = (priv->rpbuf_aec_in_len / rprehwparams.channels) * \
            (rprehwparams.channels + aec_out_channels);

    #else
    priv->rpbuf_aec_out_len = (priv->rpbuf_aec_in_len / rprehwparams.channels) * (aec_out_channels); // (4096/ 2)  * 1 (1ch aec)= 2048 byte
    #endif

    printf("rpbuf_aec_in_len %d out %d\n", priv->rpbuf_aec_in_len, priv->rpbuf_aec_out_len);


    /* 1. open_audio */

    pcm_config.channels = rprehwparams.channels;
    pcm_config.format = rprehwparams.format;
    pcm_config.rate = rprehwparams.rate;
    pcm_config.period_size = AUDIO_RPBUF_AEC_DEFAULT_SIZE;
    pcm_config.period_count = DEFAULT_RPBUF_AEC_BUF_CNT;

    route_type = MicCapture;

    ret = audio_set_record_route(aec_codec_name, 1, route_type);
    if (ret != 0)
    {
        printf("audio_set_playback_route fail\n");
        goto EXIT;
    }

    route_type = DaudioLoopBack;
    ret = audio_set_record_route(aec_daudio_name,  1, route_type);
    if (ret != 0)
    {
        printf("audio_set_playback_route fail\n");
        goto EXIT;
    }

    priv->pcm_dev = open_audio(rpbuf_pcm_name, SND_PCM_STREAM_CAPTURE, &pcm_config);
    if (!priv->pcm_dev)
    {
        printf("open %s audio fail, stream %d!\n", rpbuf_pcm_name, SND_PCM_STREAM_CAPTURE);
        goto EXIT;
    }

    #ifdef DSP_AEC_TEST

    /* set aec param */
    ret = rpbuf_aec_config(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_config %d failed\n", ret);
        goto EXIT;
    }

    //usleep(300 * 1000);

    #endif
    /* start transmit thread */

    pthread_create(&thread_tran,
                   NULL,
                   rpbuf_aec_transmit_thread,
                   (void *)priv);

    pthread_setname_np(thread_tran, "rpbuf_aec_transmit");

    /* end the wavread thread */

    if (thread_tran)
    {
        pthread_join(thread_tran, NULL);
    }

    thread_tran = 0;

    #ifdef DSP_AEC_TEST
    ret = rpbuf_aec_release(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_release %d failed\n", ret);
        goto EXIT;
    }
   #endif

EXIT:

    printf("rpbuf aec capture exit\n");
    if (thread_tran)
    {
        pthread_join(thread_tran, NULL);
    }

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_data_deinit(priv->rpbuf_audio_ctx);
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }

    #ifdef RPBUF_SPEED_TEST
    if (priv->pin_fd != -1)
    {
        close(priv->pin_fd);
        priv->pin_fd = 0;
    }
    #endif

    if (priv->infile)
    {
        fflush(priv->infile);
        fsync(fileno(priv->infile));
        fclose(priv->infile);
        priv->infile = NULL;
    }

    if (priv->pcm_dev)
    {
        route_type = MicCapture;
        ret = audio_set_record_route(aec_codec_name, 0, route_type);
        if (ret != 0)
        {
            printf("audio_set_record_route fail\n");
        }

        route_type = DaudioLoopBack;
        ret = audio_set_record_route(aec_daudio_name, 0, route_type);
        if (ret != 0)
        {
            printf("audio_set_record_route fail\n");
        }

        ret = close_audio(priv->pcm_dev, SND_PCM_STREAM_CAPTURE);
        if (ret != 0)
        {
            printf("close_audio fail\n");
        }
        priv->pcm_dev = NULL;

        printf("close_audio exit\n");
    }

    if (priv->rpbuf_ctl_arg)
    {
        free(priv->rpbuf_ctl_arg);
        priv->rpbuf_ctl_arg = NULL;
    }

    if (priv)
    {
        free(priv);
        priv = NULL;
    }
    return ret;
}

static void *rpbuf_aec_downlink_transmit_thread(void *params)
{
    rpbuf_aec_ctx_t *priv = (rpbuf_aec_ctx_t *)params;
    int ret = 0;
    void *data = NULL;
    void *recv_data = NULL;
    void *output_data = NULL;
    void *out_data = NULL;
    void *dump_data = NULL;
    unsigned long output_size = 0;
    unsigned long size = priv->rpbuf_aec_downlink_in_len;
    unsigned long out_size = priv->rpbuf_aec_downlink_out_len;
    unsigned long recv_size = 0;
    unsigned long recv_data_offset = 0;
    //long long start_usec, end_usec;
    //float delta_usec;
    snd_pcm_sframes_t length;
    int timeout = 300;
    unsigned int frame_count = 0;
    int pin_fd = -1;
    int cnt = 0;
    rpbuf_audio_remote_ctrl_aec_t rpbuf_aec_ctrl_arg;

    data = malloc(size);
    if (!data)
    {
        printf("no memory\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(data, 0, size);

    recv_data = malloc(out_size);
    if (!recv_data)
    {
        printf("no memory\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(recv_data, 0, out_size);

    #ifdef RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
    out_data = malloc(out_size);
    if (!out_data)
    {
        printf("no memory\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(out_data, 0, out_size);
    #endif

    #ifdef AEC_DOWNLINK_DUMP_FILE
    FILE *dumpfile = NULL;
    long write_size = 0;

    dumpfile = fopen(dumpaecdownlink, "wb+");
    if (!dumpfile)
    {
        printf("open file %s error!\n", dumpaecdownlink);
        goto exit;
    }
    #endif

    #ifdef RPBUF_SPEED_TEST

    pin_fd = open(sunix_pin, O_RDWR);
    if (pin_fd == -1)
    {
        printf("open file %s error!\n", sunix_pin);
        goto exit;
    }
    #endif

    #ifdef RPBUF_DSP_AEC_DOWNLINK_TEST
    ret = rpbuf_aec_downlink_start(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_downlink_start %d error!\n", ret );
        goto exit;
    }

    do {
        memset(&rpbuf_aec_ctrl_arg, 0 ,sizeof(rpbuf_audio_remote_ctrl_aec_t));
        ret = rpbuf_audio_aec_downlink_ctrl_recv_cmd(priv->rpbuf_audio_ctx, &rpbuf_aec_ctrl_arg, &recv_data_offset, &recv_size, 1);
        if (ret != 0 || (recv_size > sizeof(rpbuf_audio_remote_ctrl_aec_t)))
        {
            if (ret == -ETIMEDOUT)
            {
                printf("wait remote ack timeout!\n");
                continue;
            }
            else
            {
                printf("rpbuf_aec_downlink_start ack %d error!\n", ret);
                goto exit;
            }
        }
    } while (ret);

    if (rpbuf_aec_ctrl_arg.cmd == RPBUF_CTL_START_ACK && rpbuf_aec_ctrl_arg.type == RPBUF_ALGO_AEC_DOWNLINK && \
        rpbuf_aec_ctrl_arg.value == RPBUF_ERROR_NONE)
    {
        printf("downlink dsp has start finished\n");
    }
    else
    {
        printf("rpbuf aec get start ack failed cmd %d type %d  value %d exit downlink thread\n", rpbuf_aec_ctrl_arg.cmd, rpbuf_aec_ctrl_arg.type, rpbuf_aec_ctrl_arg.value);
        goto stop;
    }

    #endif

    while (!in_aborting)
    {

        length = fread(data, 1, size, priv->playfile);
        //printf("read  len %ld\n", length);
        if (length <= 0)
        {
            printf("read file error rlen %ld!\n", length);
            break;
        }

        #ifdef RPBUF_SPEED_TEST
        ret = write(pin_fd, PE4_output_low, strlen(PE4_output_low));
        if (ret != strlen(PE4_output_low))
        {
            printf("cmd: %s\t error: %s", PE4_output_low, strerror(errno));
            break;
        }
        #endif

        #ifdef RPBUF_DSP_AEC_DOWNLINK_TEST

        /* transfer data to dsp */
        ret = rpbuf_audio_aec_transfer_data(priv->rpbuf_audio_ctx, priv->aec_downlink_in_name, data, 0, size);
        if (ret < 0)
        {
            printf("rpbuf_audio_transfer_data error ret %d!\n", ret);
            break;
        }

        //start_usec = get_currenttime();

        memset(data, 0, size);

        cnt = 0;
        do {
            ret = rpbuf_audio_aec_recv_data(priv->rpbuf_audio_ctx, priv->aec_downlink_out_name, recv_data, &recv_data_offset, &recv_size, timeout);
            if (ret != 0)
            {
                if (ret ==  -ETIMEDOUT) {
                    printf("rpbuf aec downlink recv timeout %d\n", timeout);
                    cnt++;
                    if (cnt > 3) {
                        cnt = 0;
                        break;
                    }
                    continue;
                }
                else {
                    printf("rpbuf aec recv size %lu\n", recv_size);
                    break;
                }
            }
        } while(0);

        if (recv_size != 0xFFFFFFFF && recv_size != out_size)
        {
            printf("aec downlink recv data error recv size %ld size %ld\n", recv_size, out_size);
            break;
        }

        //end_usec = get_currenttime();
        //delta_usec = end_usec - start_usec;
        //printf("size %lu, delta ms %f\n", recv_size, delta_usec / 1000);
        #endif

        #ifdef RPBUF_SPEED_TEST
        ret = write(pin_fd, PE4_output_high, strlen(PE4_output_high));
        if (ret != strlen(PE4_output_high))
        {
            printf("cmd: %s\t error: %s", PE4_output_high, strerror(errno));
            break;
        }
        #endif

        #ifdef RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
        if (recv_size != 0xFFFFFFFF) {
            if (prehwparams.channels >= 2)
            {
                output_data = recv_data + size;
                output_size = size;
                /* 2ch change to ch0ch0 ...ch0 ch1ch1 ...ch1 ch2ch2 ...ch2 ch3ch3 ...ch3 */
                pcm_s16le_split_to_plane(out_data, recv_data, &prehwparams, prehwparams.channels, size);
                pcm_s16le_split_to_plane(out_data + size, recv_data + size, &prehwparams, prehwparams.channels, size);
                /* 4ch change to ch0ch1ch2ch3 ... ch0ch1ch2ch3 ... ch0ch1ch2ch3 */
                pcm_s16le_plane_to_split(recv_data, NULL, out_data, &prehwparams, prehwparams.channels * 2, recv_size);
                dump_data = recv_data;
            }
            else
            {
                pcm_s16le_plane_to_split(out_data, NULL, recv_data, &prehwparams, prehwparams.channels * 2, recv_size);
                output_data = recv_data + size;
                output_size = size;
                dump_data = out_data;
            }
        }
        #else
        if(recv_size != 0xFFFFFFFF) {
            output_data = recv_data;
            output_size = recv_size;
            dump_data = recv_data;
        }
        #endif

        #ifdef AEC_DOWNLINK_DUMP_FILE
        if (recv_size != 0xFFFFFFFF && dumpfile)
        {
            ret = fwrite(dump_data, 1, recv_size, dumpfile);
            if (ret != recv_size)
            {
                printf("[%s] line:%d fwrite err ! ret=%d, data_len=%ld\n", __func__, __LINE__, ret, recv_size);
            }
            write_size += recv_size;
            if (write_size == recv_size * 100)
            {
                fflush(dumpfile);
                fsync(fileno(dumpfile));
                write_size = 0;
            }
        }
        #endif

        if (recv_size != 0xFFFFFFFF) {
            frame_count = snd_pcm_bytes_to_frames(priv->pcm_dev, output_size);
            ret = audio_write(priv->pcm_dev, output_data, frame_count);
            if (ret < 0)
            {
                if (ret = -EPIPE)
                {
                    printf("audio write underrun, size %lu\n", output_size);
                }
                else
                {
                    printf("audio write err, size %lu\n", output_size);
                    break;
                }
            }
            else if (ret != frame_count)
            {
                printf("write to pcm %d!=%u\n", ret, frame_count);
                break;
            }
        }
    }

stop:

    #ifdef RPBUF_DSP_AEC_DOWNLINK_TEST
    ret = rpbuf_aec_downlink_stop(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_downlink_stop %d error!\n", ret);
        goto exit;
    }
    #endif

exit:

    #ifdef RPBUF_SPEED_TEST
    if (pin_fd != -1)
    {
        close(pin_fd);
    }
    #endif

    if (data)
    {
        free(data);
    }

    data = NULL;

    if (recv_data)
    {
        free(recv_data);
    }

    recv_data = NULL;

    #ifdef RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
    if (out_data)
    {
        free(out_data);
    }

    out_data = NULL;
    #endif

    #ifdef AEC_DOWNLINK_DUMP_FILE
    if (dumpfile)
    {
        fflush(dumpfile);
        fsync(fileno(dumpfile));
        fclose(dumpfile);
        dumpfile = NULL;
    }
    #endif

    printf("end of downlink transmit thread\n");
    return NULL;
}

static int rpbuf_aec_playback(void *params)
{
    int ret = 0;
    rpbuf_aec_ctx_t *priv = (rpbuf_aec_ctx_t *)params;
    struct WAVE_FORMAT_DEF *wave_fmt = NULL;
    pcm_config_t pcm_config = {0};
    pthread_t thread_tran = 0;
    unsigned int frame_count = 0;
    uint64_t playback_count = 0;
    int frame_bytes = 0;

    printf("rpbuf_aec_playback enter\n");

    if (rpbuf_playback_file_type == RPBUF_FORMAT_WAVE)
    {
        if (fread(&(priv->audio_info.header),
                  sizeof(struct RIFF_HEADER_DEF), 1, priv->playfile) != 1)
        {
            printf("read header info error");
            goto EXIT;
        }

        if (fread(&(priv->audio_info.fmt_block),
                  sizeof(struct FMT_BLOCK_DEF), 1, priv->playfile) != 1)
        {
            printf("read fmt_block info error");
            goto EXIT;
        }

        if (fread(&(priv->audio_info.data_block),
                  sizeof(struct DATA_BLOCK_DEF), 1, priv->playfile) != 1)
        {
            printf("read data_block info error");
            goto EXIT;
        }
        wave_fmt = &priv->audio_info.fmt_block.wav_format;

        if (!strncmp(priv->audio_info.data_block.data_id, "data", sizeof(priv->audio_info.data_block.data_id)))
        {
            playback_count = priv->audio_info.data_block.data_size;
        }
        else
        {
            printf("read data_block info error\n");
            goto EXIT;
        }
    }

    printf("here\n");

    if (wave_fmt)
    {
        ret = rpbuf_to_pcm_fmt(&priv->audio_info);
        if (ret != 0)
        {
            printf("rpbuf_to_pcm_fmt failed\n");
            goto EXIT;
        }
        prehwparams.channels = wave_fmt->channels;
        prehwparams.rate = wave_fmt->samples_per_sec;
    }

    frame_bytes = snd_pcm_format_physical_width(prehwparams.format) / 8 * prehwparams.channels;

    priv->rpbuf_aec_downlink_in_len = (frame_bytes * prehwparams.rate * prehwparams.msec) / 1000; /* 2 * 16 * 64 = 2048 bytes */

    #ifdef RPBUF_DSP_AEC_DOWNLINK_DUMP_INPUT_DATA
    priv->rpbuf_aec_downlink_out_len = (priv->rpbuf_aec_downlink_in_len / prehwparams.channels) * \
                                       (prehwparams.channels + prehwparams.channels); // (2048/ 1)  * 2 (1ch + 1ch) = 4096 byte
    #else
    priv->rpbuf_aec_downlink_out_len = (priv->rpbuf_aec_downlink_in_len / prehwparams.channels) * (prehwparams.channels); // (2048/ 1) * 1 (1ch spk out)= 2048 byte
    #endif

    printf("rpbuf_aec_downlink_in_len %d out %d\n", priv->rpbuf_aec_downlink_in_len, priv->rpbuf_aec_downlink_out_len);

    /* open playback audio */

    pcm_config.channels = prehwparams.channels;
    pcm_config.format = prehwparams.format;
    pcm_config.rate = prehwparams.rate;
    pcm_config.period_size = AUDIO_RPBUF_AEC_DEFAULT_SIZE;
    pcm_config.period_count = DEFAULT_RPBUF_AEC_BUF_CNT;

    priv->pcm_dev = open_audio(rpbuf_pcm_playback_name, SND_PCM_STREAM_PLAYBACK, &pcm_config);
    if (!priv->pcm_dev)
    {
        printf("open %s audio fail, stream %d!\n", rpbuf_pcm_playback_name, SND_PCM_STREAM_PLAYBACK);
        goto EXIT;
    }

    #ifdef RPBUF_DSP_AEC_DOWNLINK_TEST

    /* set eq param */
    ret = rpbuf_aec_downlink_config(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_downlink_config %d failed\n", ret);
        goto EXIT;
    }

    //usleep(300 * 1000);

    #endif

    /* start transmit thread*/

    pthread_create(&thread_tran,
                   NULL,
                   rpbuf_aec_downlink_transmit_thread,
                   (void *)priv);

    pthread_setname_np(thread_tran, "rpbuf_aec_downlink_transmit_thread");

    /* end the wavread thread */
    pthread_join(thread_tran, NULL);

    thread_tran = 0;


    #ifdef RPBUF_DSP_AEC_DOWNLINK_TEST
    ret = rpbuf_aec_downlink_release(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_downlink_release %d failed\n", ret);
        goto EXIT;
    }
    #endif

EXIT:

    printf("rpbuf aec playback exit\n");

    if (thread_tran)
    {
        pthread_join(thread_tran, NULL);
    }

    if (priv->rpbuf_audio_ctx)
    {
        rpbuf_audio_aec_downlink_data_deinit(priv->rpbuf_audio_ctx);
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }

    if (priv->pcm_dev)
    {
        ret = close_audio(priv->pcm_dev, SND_PCM_STREAM_PLAYBACK);
        if (ret != 0)
        {
            printf("close_audio fail\n");
        }
        priv->pcm_dev = NULL;
        printf("close_audio exit\n");
    }

    if (priv->playfile)
    {
        fclose(priv->playfile);
        priv->playfile = NULL;
    }

    if (priv->rpbuf_ctl_arg)
    {
        free(priv->rpbuf_ctl_arg);
        priv->rpbuf_ctl_arg = NULL;
    }

    if (priv)
    {
        free(priv);
        priv = NULL;
    }
    return ret;
}

static void print_rpbuf_audio_aec_help_msg(void)
{
    printf("\n");
    printf("USAGE:\n");
    printf("  rpbuf_aec_demo [OPTIONS]\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -D          : select PCM by name\n");
    printf("  -d          : select playback pcm by name\n");
    printf("  -T          : sfile type (voc, wav, raw or au)\n");
    printf("  -c          : channels\n");
    printf("  -f          : sample format (case insensitive)\n");
    printf("  -r          : sample rate\n");
    printf("  -s          : aec uplink process per mseconds(ms)\n");
    printf("  -S          : aec downlink process per mseconds(ms)\n");
    printf("  -m          : set mode (0: disable, 1: test aec downlink and uplink 2: test aec uplink and downlink \n");
    printf("  -n          : capture test mseconds\n");
    printf("  -l          : lms mseconds\n");
    printf("  -P          : PlayBack\n");
    printf("  -N          : Capture\n");
    printf("\n");
    printf("e.g.\n");
    printf("  rpbuf_aec_demo -dPlaybackAec -S 64 -T wav -P /tmp/16000.wav \n");
    printf("  rpbuf_aec_demo -DCaptureMulti1 -f S16_LE -r 16000 -c 2 -s 62 -l 62 -n 20 -N /tmp/test_aec_out.pcm \n");
    printf("  rpbuf_aec_demo -DCaptureMulti1 -dhw:snddaudio2 -f S16_LE -r 16000 -c 2 -s 62 -l 62 -n 20 -S 64 -m 1 -T wav /tmp/s16le_16000_mono.wav \n");
    printf("\n");
}

static void *rpbuf_aec_downlink_thread(void *params)
{
    char *filename = (char *)params;
    int ret = 0;
    rpbuf_aec_ctx_t *priv = NULL;

    if (filename == NULL || strlen(filename) == 0) {
        printf("path is null\n");
        goto EXIT;
    }

    priv = malloc(sizeof(rpbuf_aec_ctx_t));
    if (priv)
    {
        memset(priv, 0, sizeof(rpbuf_aec_ctx_t));
    }
    else
    {
        printf("malloc rpbuf_aec_playback failed\n");
        ret = -ENOMEM;
        goto EXIT;
    }

    priv->rpbuf_ctl_arg = malloc(sizeof(rpbuf_audio_remote_ctrl_aec_t));
    if (priv->rpbuf_ctl_arg)
    {
        memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    }
    else
    {
        printf("malloc rpbuf_aec_playback failed\n");
        ret = -ENOMEM;
        goto EXIT;
    }

    priv->playfile = fopen(filename, "rb");
    if (!priv->playfile)
    {
        printf("open file error!\n");
        goto EXIT;
    }

#ifdef RPBUF_DSP_AEC_DOWNLINK_TEST
    /* init */
    ret = rpbuf_aec_downlink_init(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_downlink_init %d failed\n", ret);
        goto EXIT;
    }
#endif

    ret = rpbuf_aec_playback(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_playback failed\n");
        goto EXIT;
    }
    else
    {
        goto end;
    }

EXIT:
    if (priv->rpbuf_audio_ctx) {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }

    if (priv->playfile) {
        fclose(priv->playfile);
        priv->playfile = NULL;
    }

    if (priv->rpbuf_ctl_arg) {
        free(priv->rpbuf_ctl_arg);
        priv->rpbuf_ctl_arg = NULL;
    }

    if (priv) {
        free(priv);
        priv = NULL;
    }
end:
    printf("end of rpbuf_aec_downlink_thread \n");
    return NULL;

}


static void *rpbuf_aec_uplink_thread(void *params)
{
    int ret = 0;
    char *filename = (char *)params;
    rpbuf_aec_ctx_t *priv = NULL;

    if (filename == NULL || strlen(filename) == 0) {
        printf("path is null\n");
        goto EXIT;
    }

    priv = malloc(sizeof(rpbuf_aec_ctx_t));
    if (priv)
    {
        memset(priv, 0, sizeof(rpbuf_aec_ctx_t));
    }
    else
    {
        printf("malloc rpbuf_aec_capture failed\n");
        ret = -ENOMEM;
        goto EXIT;
    }

    priv->rpbuf_ctl_arg = malloc(sizeof(rpbuf_audio_remote_ctrl_aec_t));
    if (priv->rpbuf_ctl_arg)
    {
        memset(priv->rpbuf_ctl_arg, 0, sizeof(rpbuf_audio_remote_ctrl_aec_t));
    }
    else
    {
        printf("malloc rpbuf_aec_capture failed\n");
        ret = -ENOMEM;
        goto EXIT;
    }

    priv->infile = fopen(filename, "wb+");
    if (!priv->infile)
    {
        printf("open file %s error!\n", filename);
        goto EXIT;
    }

#ifdef DSP_AEC_TEST
    /* init */
    ret = rpbuf_aec_init(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_init %d failed\n", ret);
        goto EXIT;
    }
#endif

    ret = rpbuf_aec_capture(priv);
    if (ret != 0)
    {
        printf("rpbuf_aec_capture failed\n");
    }
    else
    {
        goto end;
    }

EXIT:
    if (priv->rpbuf_audio_ctx) {
        rpbuf_audio_aec_deinit(priv->rpbuf_audio_ctx);
        priv->rpbuf_audio_ctx = NULL;
    }

    if (priv->infile) {
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
end:
    printf("end of rpbuf_aec_uplink_thread \n");
    return NULL;
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
    pthread_t thread_uplink = 0;
    pthread_t thread_downlink = 0;

    if (argc <= 1)
    {
        ret = -1;
        goto out;
    }

    rpbuf_mode = 0;
    rpbuf_file_type = RPBUF_FORMAT_DEFAULT;
    rpbuf_playback_file_type = RPBUF_FORMAT_DEFAULT;
    rprehwparams.format = RPBUF_DEFAULT_FORMAT;
    rprehwparams.rate = RPBUF_DEFAULT_SPEED;
    rprehwparams.channels = RPBUF_DEFAULT_CHANNEL;
    rprehwparams.msec = RPBUF_DEFAULT_MSEC;
    rprehwparams.lms_msec = RPBUF_DEFAULT_LMS_MSEC;

    prehwparams = rprehwparams;

    while ((c = getopt(argc, argv, "hD:d:t:T:c:C:f:F:r:R:s:S:l:n:m:PN")) != -1)
    {
        switch (c)
        {
            case 'h':
                print_rpbuf_audio_aec_help_msg();
                ret = 0;
                goto out;
            case 'D':
                rpbuf_pcm_name = optarg;
                break;
            case 'd':
                rpbuf_pcm_playback_name = optarg;
                break;
            case 't':
                if (strcasecmp(optarg, "raw") == 0)
                {
                    rpbuf_file_type = RPBUF_FORMAT_RAW;
                }
                else if (strcasecmp(optarg, "voc") == 0)
                {
                    rpbuf_file_type = RPBUF_FORMAT_VOC;
                }
                else if (strcasecmp(optarg, "wav") == 0)
                {
                    rpbuf_file_type = RPBUF_FORMAT_WAVE;
                }
                else
                {
                    printf("unrecognized file format %s", optarg);
                    return 1;
                }
                break;
            case 'T':
                if (strcasecmp(optarg, "raw") == 0)
                {
                    rpbuf_playback_file_type = RPBUF_FORMAT_RAW;
                }
                else if (strcasecmp(optarg, "voc") == 0)
                {
                    rpbuf_playback_file_type = RPBUF_FORMAT_VOC;
                }
                else if (strcasecmp(optarg, "wav") == 0)
                {
                    rpbuf_playback_file_type = RPBUF_FORMAT_WAVE;
                }
                else
                {
                    printf("unrecognized file format %s", optarg);
                    return 1;
                }
                break;
            case 'c':
                rprehwparams.channels = strtol(optarg, NULL, 0);
                if (rprehwparams.channels < 1 || rprehwparams.channels > 256)
                {
                    printf("value %u for channels is invalid", rprehwparams.channels);
                    return 1;
                }
                break;
            case 'C':
                prehwparams.channels = strtol(optarg, NULL, 0);
                if (prehwparams.channels < 1 || prehwparams.channels > 256)
                {
                    printf("value %u for channels is invalid", prehwparams.channels);
                    return 1;
                }
                break;
            case 'f':
                rprehwparams.format = snd_pcm_format_value(optarg);
                if (rprehwparams.format == SND_PCM_FORMAT_UNKNOWN)
                {
                    printf("wrong extended format '%s'", optarg);
                    return 1;
                }
                break;
            case 'F':
                prehwparams.format = snd_pcm_format_value(optarg);
                if (prehwparams.format == SND_PCM_FORMAT_UNKNOWN)
                {
                    printf("wrong extended format '%s'", optarg);
                    return 1;
                }
                break;
            case 'r':
                tmp = strtol(optarg, NULL, 0);
                if (tmp < 300)
                {
                    tmp *= 1000;
                }
                rprehwparams.rate = tmp;
                if (tmp < 2000 || tmp > 192000)
                {
                    printf("bad speed value %d", tmp);
                    return 1;
                }
                break;
            case 'R':
                tmp = strtol(optarg, NULL, 0);
                if (tmp < 300)
                {
                    tmp *= 1000;
                }
                prehwparams.rate = tmp;
                if (tmp < 2000 || tmp > 192000)
                {
                    printf("bad speed value %d", tmp);
                    return 1;
                }
                break;
            case 's':
                tmp = strtol(optarg, NULL, 0);
                if (tmp < 0)
                {
                    printf("bad msec %d", tmp);
                    return 1;
                }
                rprehwparams.msec = tmp;
                break;
            case 'S':
                tmp = strtol(optarg, NULL, 0);
                if (tmp < 0)
                {
                    printf("bad msec %d", tmp);
                    return 1;
                }
                prehwparams.msec = tmp;
                break;
            case 'l':
                tmp = strtol(optarg, NULL, 0);
                if (tmp < 0)
                {
                    printf("bad msec %d", tmp);
                    return 1;
                }
                rprehwparams.lms_msec = tmp;
                break;
            case 'n':
                long_tmp = strtol(optarg, NULL, 0);
                if (long_tmp == LONG_MIN || long_tmp == LONG_MAX)
                {
                    printf("Invalid input %s.\n", optarg);
                    ret = -EINVAL;
                    goto out;
                }
                rpbuf_timelimit = long_tmp;
                break;
            case 'm':
                rpbuf_mode = strtol(optarg, NULL, 0);
                if (rpbuf_mode < 0 || rpbuf_mode == LONG_MAX)
                {
                    printf("value %d for mode is invalid", rpbuf_mode);
                    return 1;
                }
                break;
            case 'P':
                rpbuf_stream = SND_PCM_STREAM_PLAYBACK;
                break;
            case 'N':
                rpbuf_stream = SND_PCM_STREAM_CAPTURE;
                break;
            default:
                printf("Invalid option: -%c\n", c);
                ret = -1;
                goto out;
        }
    }

    if (optind > argc - 1)
    {
        printf("Please enter the filename\n");
        ret = -1;
        goto out;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

    switch (rpbuf_mode)
    {
        case 0:
        {
            if (rpbuf_stream == SND_PCM_STREAM_PLAYBACK)
            {
                pthread_create(&thread_downlink,
                           NULL,
                           rpbuf_aec_downlink_thread,
                           (void *)argv[optind++]);
                pthread_join(thread_downlink, NULL);
            }
            else
            {
                 pthread_create(&thread_uplink,
                           NULL,
                           rpbuf_aec_uplink_thread,
                           (void *)argv[optind++]);
                pthread_join(thread_uplink, NULL);
            }
            break;
        }
        case 1:
        {
            /* playback first */
            pthread_create(&thread_downlink,
                           NULL,
                           rpbuf_aec_downlink_thread,
                           (void *)argv[optind++]);

            //usleep(20000);

            pthread_create(&thread_uplink,
                           NULL,
                           rpbuf_aec_uplink_thread,
                           aecuplink);

            pthread_join(thread_downlink, NULL);

            pthread_join(thread_uplink, NULL);

            break;
        }
        case 2:
        {
            /* capture first */
            pthread_create(&thread_uplink,
                           NULL,
                           rpbuf_aec_uplink_thread,
                           aecuplink);

            //usleep(20000);

            pthread_create(&thread_downlink,
                           NULL,
                           rpbuf_aec_downlink_thread,
                           (void *)argv[optind++]);

            pthread_join(thread_uplink, NULL);

            pthread_join(thread_downlink, NULL);

            break;
        }
        default:
            printf("unknown 's' command\n");
            break;
    }

    return ret;


out:
    print_rpbuf_audio_aec_help_msg();
    return ret;

}




