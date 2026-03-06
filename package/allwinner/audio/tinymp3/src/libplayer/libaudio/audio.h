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
/*
	audio.h
 */

#ifndef _AUDIO_H__
#define _AUDIO_H__

/**
 * ao_format_t
 * @channels:
 * @bits:
 * @format:
 * @rate:
 */
typedef struct {
	int	channels;	/* number of audio channels */
	int	bytes;		/* bytes per sample */
	int	format;		/* pcm format */
	int	rate;		/* samples per second */
} ao_format_t;

/**
 * ao_device_t
 * @handler:
 * @hw_gain:
 * @fmt:
 */
typedef struct {
	void *handler;	/* number of audio channels */
	ao_format_t fmt;
	int hw_gain;
} ao_device_t;

/**
 * audio_output_t
 */
typedef struct {
	char	*name;

	int (*init)(ao_device_t *device);
	void (*deinit)(ao_device_t *device);

	int (*start)(ao_device_t *device);
	void (*play)(ao_device_t *device, short *buf, int samples);
	void (*stop)(ao_device_t *device);

	void (*help)(void);
	void (*volume)(ao_device_t *device, int vol);
	int (*dev_try)(ao_device_t *device);
	int (*get_space)(ao_device_t *device);
} audio_output_t;

/**
 * audio_get_output -
 */
extern audio_output_t *audio_get_output(char *name);

/**
 * audio_ls_outputs -
 */
extern void audio_ls_outputs(void);

#endif /* _AUDIO_H__ */
