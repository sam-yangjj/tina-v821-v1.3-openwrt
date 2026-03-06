/*
* Copyright (c) 2008-2020 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ispSimpleCode.h
* Description :
* History :
*   Author  :
*   Date    :
*   Comment :
*
*
*/

#ifndef ISPSIMPLECODE_H
#define ISPSIMPLECODE_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vencoder.h"

#define VENCODE_ROI_NUM 4

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vencode_command_e {
	VENCODE_CMD_HEAD_PPSSPS = 0x00,
	VENCODE_CMD_STREAM		= 0x01,
} vencode_command;

/****************************************************************************************************************
 * structure
 ***************************************************************************************************************/
typedef struct {
	VencHeaderData			sps_pps_data;
	VencH264Param			h264Param;
	VencMBModeCtrl			h264MBMode;
	VencMBInfo				MBInfo;
	VencFixQP				fixQP;
	VencSuperFrameConfig	sSuperFrameCfg;
	VencH264SVCSkip			SVCSkip; // set SVC and skip_frame
	VencH264AspectRatio		sAspectRatio;
	VencH264VideoSignal		sVideoSignal;
	VencCyclicIntraRefresh	sIntraRefresh;
	VencROIConfig			sRoiConfig[VENCODE_ROI_NUM];
	VeProcSet				sVeProcInfo;
	VencOverlayInfoS		sOverlayInfo;
	VencSmartFun			sH264Smart;
} h264_func_t;

typedef struct {
	VencH265Param			h265Param;
	VencH265GopStruct		h265Gop;
	VencHVS					h265Hvs;
	VencH265TendRatioCoef	h265Trc;
	VencSmartFun			h265Smart;
	VencMBModeCtrl			h265MBMode;
	VencMBInfo				MBInfo;
	VencFixQP				fixQP;
	VencSuperFrameConfig	sSuperFrameCfg;
	VencH264SVCSkip			SVCSkip; // set SVC and skip_frame
	VencH264AspectRatio		sAspectRatio;
	VencH264VideoSignal		sVideoSignal;
	VencCyclicIntraRefresh	sIntraRefresh;
	VencROIConfig			sRoiConfig[VENCODE_ROI_NUM];
	VencAlterFrameRateInfo	sAlterFrameRateInfo;
	int						h265_rc_frame_total;
	VeProcSet				sVeProcInfo;
	VencOverlayInfoS		sOverlayInfo;
}h265_func_t;

typedef struct {
    EXIFInfo                exifinfo;
    int                     quality;
    int                     jpeg_mode;
    VencJpegVideoSignal     vs;
    int                     jpeg_biteRate;
    int                     jpeg_frameRate;
    VencBitRateRange        bitRateRange;
    VencOverlayInfoS        sOverlayInfo;
    VencCopyROIConfig       pRoiConfig;
}jpeg_func_t;

typedef struct Fixqp {
	int mIQp;
	int mPQp;
} Fixqp;

typedef struct {
	unsigned int src_width;
	unsigned int src_height;
	unsigned int dst_width;
	unsigned int dst_height;
	unsigned int encode_frame_num;
	unsigned int encode_format;
	int bit_rate;
	int frame_rate;
	int SbmBufSize;
	int maxKeyFrame;
	unsigned int qp_min;
	unsigned int qp_max;
      unsigned int mInitQp;
	int                 bFastEncFlag;
	int                 mbPintraEnable;
	int                 n3DNR;
	int                 IQpOffset;

	int Rotate;
	int RCEnable;
	int ThrdI;
	int ThrdP;
	int MaxReEncodeTimes;
#ifdef INPUTSOURCE_FILE
	unsigned int src_size;
	unsigned int dts_size;
	FILE *in_file;
	FILE *out_file;
	char input_path[256];
	char output_path[256];
	VencAllocateBufferParam bufferParam;
#endif

      Fixqp fixqp;
	VencInputBuffer inputBuffer;
	VencOutputBuffer outputBuffer;
	VencHeaderData sps_pps_data;
	VENC_PIXEL_FMT picture_format;

      jpeg_func_t jpeg_func;
	h264_func_t h264_func;
	h265_func_t h265_func;
}encode_param_t;

int OpenEncoder(VENC_CODEC_TYPE type);

int PrepareEncoder(encode_param_t *encode_param);

int StartEncoder(encode_param_t *encode_param, VencInputBuffer *input, VencOutputBuffer *output, vencode_command type);

int FreeEncoderOutputBuffer(VencOutputBuffer *outputBuffer);

int CloseEncoder(encode_param_t *encode_param);
#ifdef __cplusplus
}
#endif

#endif

