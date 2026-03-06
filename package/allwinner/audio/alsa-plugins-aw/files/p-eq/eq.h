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
#ifndef _EQ_H_
#define _EQ_H_
#include <stdlib.h>

#ifdef FIXED_POINT
#else
#include <math.h>
#endif
#include <math.h>
/* EQ filter type definition */

#define MAX_FILTER_N (10)

typedef enum
{
	/* low pass shelving filter */
	LOWPASS_SHELVING,
	/* band pass peak filter */
	BANDPASS_PEAK,
	/* high pass shelving filter */
	HIHPASS_SHELVING,
	LOWPASS,
	HIGHPASS
}eq_ftype_t;

/* equalizer parameters */
typedef struct
{
	/* boost/cut gain in dB */
	int G;
	/* cutoff/center frequency in Hz */
	int fc;
	/* quality factor */
	float Q;
	/* filter type */
	eq_ftype_t type;
}eq_core_prms_t;

typedef struct
{
	/*num of items(biquad)*/
	int biq_num;
	/* sampling rate */
	int sampling_rate;
	/* channel num */
	int chan;
	/* eq parameters for generate IIR coeff*/
	eq_core_prms_t core_prms[MAX_FILTER_N];
}eq_prms_t;


typedef struct
{
	/*num of items(biquad)*/
	int enable_[MAX_FILTER_N];	//enable == 1 ;  disable == 0

	/* sampling rate */
	int sampling_rate;
	/* channel num */
	int chan;
	/* eq parameters for generate IIR coeff*/
	eq_core_prms_t core_prms[MAX_FILTER_N];
}eq_prms_t_pc;



/*
	function eq_create
description:
	use this function to create the equalizer object
prms:
	eq_prms_t: [in], desired frequency response array
returns:
	the equalizer handle
*/
void* eq_create(eq_prms_t* prms);
/*
	function eq_process
description:
	equalizer processing function
prms:
	handle:[in,out] equalzier handle
	x:[in,out],	input signal
	len:[in], input length(in samples)
returns:
	none
*/
void eq_process(void* handle, short* x, int len);
/*
	function eq_destroy
description:
	use this function to destroy the equalizer object
prms:
	handle:[in], equalizer handle
returns:
	none
*/
void eq_destroy(void* handle);

void* eq_setpara_reset(eq_prms_t* prms, void * eq_handle);

void eq_avert_parms(eq_prms_t * eq_para, eq_prms_t_pc *eq_para_pc);

#endif
