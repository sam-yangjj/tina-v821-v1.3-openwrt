/*
* Copyright (c) 2008-2020 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ispSimpleCode.c
* Description :
* History :
*   Author  :
*   Date    :
*   Comment :
*
*
*/
#include "include/ispSimpleCode.h"
#include "../tuning_app/server/isp_handle.h"
#include <dlfcn.h>
#include <sys/time.h>
#include <string.h>
#include "../isp_version.h"

/****************************************************************************************************************
 * define
 ***************************************************************************************************************/
#define logt(fmt, arg...)
#define loge(fmt, arg...)	printf("%s: " fmt " lineNum-%d\n", __FUNCTION__, ##arg, __LINE__);
#define VIDEOCODEC_OK		(0)
#define VIDEOCODEC_FAIL		(-1)
#define ALIGN_XXB(y, x)		(((x) + ((y)-1)) & ~((y)-1))
#define VERSION_STRING		"20201230-V1.0"
#define VBV_TOTAL_SIZE		(12 * 1024 * 1024)

#if (ISP_VERSION == 521)
	#define VENC_STATIC_LIBRARY
#elif (ISP_VERSION == 522)
	#define VENC_DYNAMIC_LIBRARY
#endif
/****************************************************************************************************************
 * vencoder library function define
 ***************************************************************************************************************/
typedef VideoEncoder* (* VideoEncCreateFunc)(VENC_CODEC_TYPE eCodecType);
typedef void (* VideoEncDestroyFunc)(VideoEncoder* pEncoder);
typedef int (* VideoEncInitFunc)(VideoEncoder* pEncoder, VencBaseConfig* pConfig);
typedef int (* AllocInputBufferFunc)(VideoEncoder* pEncoder, VencAllocateBufferParam *pBufferParam);
typedef int (* GetOneAllocInputBufferFunc)(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
typedef int (* FlushCacheAllocInputBufferFunc)(VideoEncoder* pEncoder,	VencInputBuffer *pInputbuffer);
typedef int (* ReturnOneAllocInputBufferFunc)(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer);
typedef int (* AddOneInputBufferFunc)(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
typedef int (* VideoEncodeOneFrameFunc)(VideoEncoder* pEncoder);
typedef int (* AlreadyUsedInputBufferFunc)(VideoEncoder* pEncoder, VencInputBuffer* pBuffer);
typedef int (* GetOneBitstreamFrameFunc)(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);
typedef int (* FreeOneBitStreamFrameFunc)(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);
typedef int (* VideoEncGetParameterFunc)(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);
typedef int (* VideoEncSetParameterFunc)(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);
typedef int (* VideoEncoderSetFreqFunc)(VideoEncoder* pEncoder, int nVeFreq);

static VideoEncCreateFunc VideoEncCreateDl = NULL;
static VideoEncDestroyFunc VideoEncDestroyDl = NULL;
static VideoEncInitFunc VideoEncInitDl = NULL;
static AllocInputBufferFunc AllocInputBufferDl = NULL;
static GetOneAllocInputBufferFunc GetOneAllocInputBufferDl = NULL;
static FlushCacheAllocInputBufferFunc FlushCacheAllocInputBufferDl = NULL;
static ReturnOneAllocInputBufferFunc ReturnOneAllocInputBufferDl = NULL;
static AddOneInputBufferFunc AddOneInputBufferDl = NULL;
static VideoEncodeOneFrameFunc VideoEncodeOneFrameDl = NULL;
static AlreadyUsedInputBufferFunc AlreadyUsedInputBufferDl = NULL;
static GetOneBitstreamFrameFunc GetOneBitstreamFrameDl = NULL;
static FreeOneBitStreamFrameFunc FreeOneBitStreamFrameDl = NULL;
static VideoEncGetParameterFunc VideoEncGetParameterDl = NULL;
static VideoEncSetParameterFunc VideoEncSetParameterDl = NULL;
static VideoEncoderSetFreqFunc VideoEncoderSetFreqDl = NULL;

/****************************************************************************************************************
 * global variable
 ***************************************************************************************************************/
static void *vencoderLib = NULL;
// Create in openEnc() and destory in closeEnc()
static VideoEncoder* pVideoEnc = NULL;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int width_aligh16;
    unsigned int height_aligh16;
    unsigned char* argb_addr;
    unsigned int size;
}BitMapInfoS;
BitMapInfoS bit_map_info[13];//= {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};

static long long GetNowUs(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000ll + tv.tv_usec;
}

void init_jpeg_exif(EXIFInfo *exifinfo)
{
    exifinfo->ThumbWidth = 640;
    exifinfo->ThumbHeight = 480;

    strcpy((char*)exifinfo->CameraMake,        "allwinner make test");
    strcpy((char*)exifinfo->CameraModel,        "allwinner model test");
    strcpy((char*)exifinfo->DateTime,         "2014:02:21 10:54:05");
    strcpy((char*)exifinfo->gpsProcessingMethod,  "allwinner gps");

    exifinfo->Orientation = 0;

    exifinfo->ExposureTime.num = 2;
    exifinfo->ExposureTime.den = 1000;

    exifinfo->FNumber.num = 20;
    exifinfo->FNumber.den = 10;
    exifinfo->ISOSpeed = 50;

    exifinfo->ExposureBiasValue.num= -4;
    exifinfo->ExposureBiasValue.den= 1;

    exifinfo->MeteringMode = 1;
    exifinfo->FlashUsed = 0;

    exifinfo->FocalLength.num = 1400;
    exifinfo->FocalLength.den = 100;

    exifinfo->DigitalZoomRatio.num = 4;
    exifinfo->DigitalZoomRatio.den = 1;

    exifinfo->WhiteBalance = 1;
    exifinfo->ExposureMode = 1;

    exifinfo->enableGpsInfo = 1;

    exifinfo->gps_latitude = 23.2368;
    exifinfo->gps_longitude = 24.3244;
    exifinfo->gps_altitude = 1234.5;

    exifinfo->gps_timestamp = (long)time(NULL);

    strcpy((char*)exifinfo->CameraSerialNum,  "123456789");
    strcpy((char*)exifinfo->ImageName,  "exif-name-test");
    strcpy((char*)exifinfo->ImageDescription,  "exif-descriptor-test");
}

void init_jpeg_rate_ctrl(jpeg_func_t *jpeg_func)
{
    jpeg_func->jpeg_biteRate = 12*1024*1024;
    jpeg_func->jpeg_frameRate = 30;
    jpeg_func->bitRateRange.bitRateMax = 14*1024*1024;
    jpeg_func->bitRateRange.bitRateMin = 10*1024*1024;
}

#if 0
void init_overlay_info(VencOverlayInfoS *pOverlayInfo, encode_param_t *encode_param)
{
	int i;
	unsigned char num_bitMap = 2;
	BitMapInfoS* pBitMapInfo;
	unsigned int time_id_list[19];
	unsigned int start_mb_x;
	unsigned int start_mb_y;

	memset(pOverlayInfo, 0, sizeof(VencOverlayInfoS));

#if 0
	char filename[64];
	int ret;
	for(i = 0; i < num_bitMap; i++)
	{
		FILE* icon_hdle = NULL;
		int width  = 0;
		int height = 0;

		sprintf(filename, "%s%d.bmp", "/mnt/libcedarc/bitmap/icon_720p_",i);

		icon_hdle	= fopen(filename, "r");
		if (icon_hdle == NULL) {
			printf("get wartermark %s error\n", filename);
			return;
		}

		//get watermark picture size
		fseek(icon_hdle, 18, SEEK_SET);
		fread(&width, 1, 4, icon_hdle);
		fread(&height, 1, 4, icon_hdle);

		fseek(icon_hdle, 54, SEEK_SET);

		bit_map_info[i].argb_addr = NULL;
		bit_map_info[i].width = 0;
		bit_map_info[i].height = 0;

		bit_map_info[i].width = width;
		bit_map_info[i].height = height*(-1);

		bit_map_info[i].width_aligh16 = ALIGN_XXB(16, bit_map_info[i].width);
		bit_map_info[i].height_aligh16 = ALIGN_XXB(16, bit_map_info[i].height);
		if(bit_map_info[i].argb_addr == NULL) {
			bit_map_info[i].argb_addr =
			(unsigned char*)malloc(bit_map_info[i].width_aligh16*bit_map_info[i].height_aligh16*4);

			if(bit_map_info[i].argb_addr == NULL)
			{
				loge("malloc bit_map_info[%d].argb_addr fail\n", i);
				return;
			}
		}
		logd("bitMap[%d] size[%d,%d], size_align16[%d, %d], argb_addr:%p\n", i,
									bit_map_info[i].width,
									bit_map_info[i].height,
									bit_map_info[i].width_aligh16,
									bit_map_info[i].height_aligh16,
									bit_map_info[i].argb_addr);

		ret = fread(bit_map_info[i].argb_addr, 1,
			bit_map_info[i].width*bit_map_info[i].height*4, icon_hdle);
		if(ret != bit_map_info[i].width*bit_map_info[i].height*4)
			loge("read bitMap[%d] error, ret value:%d\n", i, ret);

		bit_map_info[i].size = bit_map_info[i].width_aligh16 * bit_map_info[i].height_aligh16 * 4;

		if (icon_hdle) {
			fclose(icon_hdle);
			icon_hdle = NULL;
		}
	}

	//time 2017-04-27 18:28:26
	time_id_list[0] = 2;
	time_id_list[1] = 0;
	time_id_list[2] = 1;
	time_id_list[3] = 7;
	time_id_list[4] = 11;
	time_id_list[5] = 0;
	time_id_list[6] = 4;
	time_id_list[7] = 11;
	time_id_list[8] = 2;
	time_id_list[9] = 7;
	time_id_list[10] = 10;
	time_id_list[11] = 1;
	time_id_list[12] = 8;
	time_id_list[13] = 12;
	time_id_list[14] = 2;
	time_id_list[15] = 8;
	time_id_list[16] = 12;
	time_id_list[17] = 2;
	time_id_list[18] = 6;

	logd("pOverlayInfo:%p\n", pOverlayInfo);
	pOverlayInfo->blk_num = 19;
#else
		FILE* icon_hdle = NULL;
		int width  = 464;
		int height = 32;
		memset(time_id_list, 0 ,sizeof(time_id_list));

		icon_hdle = fopen(encode_param->overlay_file, "r");
		logd("icon_hdle = %p",icon_hdle);
		if (icon_hdle == NULL) {
			printf("get icon_hdle error\n");
			return;
		}

		for(i = 0; i < num_bitMap; i++)
		{
			bit_map_info[i].argb_addr = NULL;
			bit_map_info[i].width = width;
			bit_map_info[i].height = height;

			bit_map_info[i].width_aligh16 = ALIGN_XXB(16, bit_map_info[i].width);
			bit_map_info[i].height_aligh16 = ALIGN_XXB(16, bit_map_info[i].height);
			if(bit_map_info[i].argb_addr == NULL) {
				bit_map_info[i].argb_addr =
			(unsigned char*)malloc(bit_map_info[i].width_aligh16*bit_map_info[i].height_aligh16*4);

				if(bit_map_info[i].argb_addr == NULL)
				{
					loge("malloc bit_map_info[%d].argb_addr fail\n", i);
					if (icon_hdle) {
						fclose(icon_hdle);
						icon_hdle = NULL;
					}

					return;
				}
			}
			logd("bitMap[%d] size[%d,%d], size_align16[%d, %d], argb_addr:%p\n", i,
														bit_map_info[i].width,
														bit_map_info[i].height,
														bit_map_info[i].width_aligh16,
														bit_map_info[i].height_aligh16,
														bit_map_info[i].argb_addr);

			int ret;
			ret = fread(bit_map_info[i].argb_addr, 1,
						bit_map_info[i].width*bit_map_info[i].height*4, icon_hdle);
			if(ret != (int)(bit_map_info[i].width*bit_map_info[i].height*4))
			loge("read bitMap[%d] error, ret value:%d\n", i, ret);

			bit_map_info[i].size
				= bit_map_info[i].width_aligh16 * bit_map_info[i].height_aligh16 * 4;
			fseek(icon_hdle, 0, SEEK_SET);
		}
		if (icon_hdle) {
			fclose(icon_hdle);
			icon_hdle = NULL;
		}
#endif
		pOverlayInfo->argb_type = VENC_OVERLAY_ARGB8888;
		pOverlayInfo->blk_num = num_bitMap;
		logd("blk_num:%d, argb_type:%d\n", pOverlayInfo->blk_num, pOverlayInfo->argb_type);
		//pOverlayInfo->invert_threshold = 200;
		//pOverlayInfo->invert_mode = 3;

		start_mb_x = 0;
		start_mb_y = 0;
		for(i=0; i<pOverlayInfo->blk_num; i++)
		{
			//id = time_id_list[i];
			//pBitMapInfo = &bit_map_info[id];
			pBitMapInfo = &bit_map_info[i];

			pOverlayInfo->overlayHeaderList[i].start_mb_x = start_mb_x;
			pOverlayInfo->overlayHeaderList[i].start_mb_y = start_mb_y;
			pOverlayInfo->overlayHeaderList[i].end_mb_x = start_mb_x
										+ (pBitMapInfo->width_aligh16 / 16 - 1);
			pOverlayInfo->overlayHeaderList[i].end_mb_y = start_mb_y
										+ (pBitMapInfo->height_aligh16 / 16 -1);

			pOverlayInfo->overlayHeaderList[i].extra_alpha_flag = 0;
			pOverlayInfo->overlayHeaderList[i].extra_alpha = 8;
			if(i%3 == 0)
				pOverlayInfo->overlayHeaderList[i].overlay_type = LUMA_REVERSE_OVERLAY;
			else if(i%2 == 0 && i!=0)
				pOverlayInfo->overlayHeaderList[i].overlay_type = COVER_OVERLAY;
			else
				pOverlayInfo->overlayHeaderList[i].overlay_type = NORMAL_OVERLAY;


			if(pOverlayInfo->overlayHeaderList[i].overlay_type == COVER_OVERLAY)
			{
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_y = 0xff;
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_u = 0xff;
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_v = 0xff;
			}

			//pOverlayInfo->overlayHeaderList[i].bforce_reverse_flag = 1;

			pOverlayInfo->overlayHeaderList[i].overlay_blk_addr = pBitMapInfo->argb_addr;
			pOverlayInfo->overlayHeaderList[i].bitmap_size = pBitMapInfo->size;

			loge("blk_%d[%d], start_mb[%d,%d], end_mb[%d,%d],extra_alpha_flag:%d, extra_alpha:%d\n",
								i,
								time_id_list[i],
								pOverlayInfo->overlayHeaderList[i].start_mb_x,
								pOverlayInfo->overlayHeaderList[i].start_mb_y,
								pOverlayInfo->overlayHeaderList[i].end_mb_x,
								pOverlayInfo->overlayHeaderList[i].end_mb_y,
								pOverlayInfo->overlayHeaderList[i].extra_alpha_flag,
								pOverlayInfo->overlayHeaderList[i].extra_alpha);
			loge("overlay_type:%d, cover_yuv[%d,%d,%d], overlay_blk_addr:%p, bitmap_size:%d\n",
								pOverlayInfo->overlayHeaderList[i].overlay_type,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_y,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_u,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_v,
								pOverlayInfo->overlayHeaderList[i].overlay_blk_addr,
								pOverlayInfo->overlayHeaderList[i].bitmap_size);
			//if(i != 5)
			{
				start_mb_x += pBitMapInfo->width_aligh16 / 16;
				start_mb_y += pBitMapInfo->height_aligh16 / 16;
			}
		}

	return;
}
#endif

static void init_h265_gop(VencH265GopStruct *h265Gop)
{
	h265Gop->gop_size = 8;
	h265Gop->intra_period = 16;

	h265Gop->use_lt_ref_flag = 1;
	if(h265Gop->use_lt_ref_flag) {
		h265Gop->max_num_ref_pics = 2;
		h265Gop->num_ref_idx_l0_default_active = 2;
		h265Gop->num_ref_idx_l1_default_active = 2;
		h265Gop->use_sps_rps_flag = 0;
	} else {
		h265Gop->max_num_ref_pics = 1;
		h265Gop->num_ref_idx_l0_default_active = 1;
		h265Gop->num_ref_idx_l1_default_active = 1;
		h265Gop->use_sps_rps_flag = 1;
	}
	//1:user config the reference info; 0:encoder config the reference info
	h265Gop->custom_rps_flag = 0;
}

static void init_mb_mode(VencMBModeCtrl *pMBMode, encode_param_t *encode_param)
{
	unsigned int mb_num;

	mb_num = (ALIGN_XXB(16, encode_param->dst_width) >> 4) * (ALIGN_XXB(16, encode_param->dst_height) >> 4);
	pMBMode->p_info = malloc(sizeof(VencMBModeCtrlInfo) * mb_num);
	pMBMode->mode_ctrl_en = 1;
}

static void init_mb_info(VencMBInfo *MBInfo, encode_param_t *encode_param)
{
	if(encode_param->encode_format == VENC_CODEC_H265)
	{
		MBInfo->num_mb = (ALIGN_XXB(32, encode_param->dst_width) *
							ALIGN_XXB(32, encode_param->dst_height)) >> 10;
	}
	else
	{
		MBInfo->num_mb = (ALIGN_XXB(16, encode_param->dst_width) *
							ALIGN_XXB(16, encode_param->dst_height)) >> 8;
	}
	MBInfo->p_para = (VencMBInfoPara *)malloc(sizeof(VencMBInfoPara) * MBInfo->num_mb);
	if(MBInfo->p_para == NULL)
	{
		loge("malloc MBInfo->p_para error");
		return;
	}
}

static void init_fix_qp(VencFixQP *fixQP)
{
	fixQP->bEnable = 1;
	fixQP->nIQp = vencoder_tuning_param->fixqp_cfg.mIQp;
	fixQP->nPQp = vencoder_tuning_param->fixqp_cfg.mPQp;

	loge("----------H264_para:fix-qp, I=%d, P=%d", \
		vencoder_tuning_param->fixqp_cfg.mIQp,\
		vencoder_tuning_param->fixqp_cfg.mPQp);
}

static void init_super_frame_cfg(VencSuperFrameConfig *sSuperFrameCfg, encode_param_t *encode_param)
{
      if(encode_param->MaxReEncodeTimes > 0)
      	{
      	     sSuperFrameCfg->eSuperFrameMode = VENC_SUPERFRAME_REENCODE;
	     sSuperFrameCfg->nMaxIFrameBits = vencoder_tuning_param->common_cfg.ThrdI;
	     sSuperFrameCfg->nMaxPFrameBits = vencoder_tuning_param->common_cfg.ThrdP;
      	}
	 else
	 {
	     sSuperFrameCfg->eSuperFrameMode = VENC_SUPERFRAME_NONE;
	     sSuperFrameCfg->nMaxIFrameBits = 30000*8;
	     sSuperFrameCfg->nMaxPFrameBits = 15000*8;
	 }
}

static void init_svc_skip(VencH264SVCSkip *SVCSkip)
{
	SVCSkip->nTemporalSVC = T_LAYER_4;
	switch(SVCSkip->nTemporalSVC)
	{
		case T_LAYER_4:
			SVCSkip->nSkipFrame = SKIP_8;
			break;
		case T_LAYER_3:
			SVCSkip->nSkipFrame = SKIP_4;
			break;
		case T_LAYER_2:
			SVCSkip->nSkipFrame = SKIP_2;
			break;
		default:
			SVCSkip->nSkipFrame = NO_SKIP;
			break;
	}
}

static void init_aspect_ratio(VencH264AspectRatio *sAspectRatio)
{
	sAspectRatio->aspect_ratio_idc = 255;
	sAspectRatio->sar_width = 4;
	sAspectRatio->sar_height = 3;
}

static void init_video_signal(VencH264VideoSignal *sVideoSignal)
{
	sVideoSignal->video_format = 5;
	sVideoSignal->src_colour_primaries = 0;
	sVideoSignal->dst_colour_primaries = 1;
}

static void init_intra_refresh(VencCyclicIntraRefresh *sIntraRefresh)
{
	sIntraRefresh->bEnable = 1;
	sIntraRefresh->nBlockNumber = 10;
}

static void init_roi(VencROIConfig *sRoiConfig)
{
	sRoiConfig[0].bEnable = 1;
	sRoiConfig[0].index = 0;
	sRoiConfig[0].nQPoffset = 10;
	sRoiConfig[0].sRect.nLeft = 0;
	sRoiConfig[0].sRect.nTop = 0;
	sRoiConfig[0].sRect.nWidth = 1280;
	sRoiConfig[0].sRect.nHeight = 320;

	sRoiConfig[1].bEnable = 1;
	sRoiConfig[1].index = 1;
	sRoiConfig[1].nQPoffset = 10;
	sRoiConfig[1].sRect.nLeft = 320;
	sRoiConfig[1].sRect.nTop = 180;
	sRoiConfig[1].sRect.nWidth = 320;
	sRoiConfig[1].sRect.nHeight = 180;

	sRoiConfig[2].bEnable = 1;
	sRoiConfig[2].index = 2;
	sRoiConfig[2].nQPoffset = 10;
	sRoiConfig[2].sRect.nLeft = 320;
	sRoiConfig[2].sRect.nTop = 180;
	sRoiConfig[2].sRect.nWidth = 320;
	sRoiConfig[2].sRect.nHeight = 180;

	sRoiConfig[3].bEnable = 1;
	sRoiConfig[3].index = 3;
	sRoiConfig[3].nQPoffset = 10;
	sRoiConfig[3].sRect.nLeft = 320;
	sRoiConfig[3].sRect.nTop = 180;
	sRoiConfig[3].sRect.nWidth = 320;
	sRoiConfig[3].sRect.nHeight = 180;
}

static void init_alter_frame_rate_info(VencAlterFrameRateInfo *pAlterFrameRateInfo)
{
	memset(pAlterFrameRateInfo, 0 , sizeof(VencAlterFrameRateInfo));
	pAlterFrameRateInfo->bEnable = 1;
	pAlterFrameRateInfo->bUseUserSetRoiInfo = 1;
	pAlterFrameRateInfo->sRoiBgFrameRate.nSrcFrameRate = 25;
	pAlterFrameRateInfo->sRoiBgFrameRate.nDstFrameRate = 5;

	pAlterFrameRateInfo->roi_param[0].bEnable = 1;
	pAlterFrameRateInfo->roi_param[0].index = 0;
	pAlterFrameRateInfo->roi_param[0].nQPoffset = 10;
	pAlterFrameRateInfo->roi_param[0].roi_abs_flag = 1;
	pAlterFrameRateInfo->roi_param[0].sRect.nLeft = 0;
	pAlterFrameRateInfo->roi_param[0].sRect.nTop = 0;
	pAlterFrameRateInfo->roi_param[0].sRect.nWidth = 320;
	pAlterFrameRateInfo->roi_param[0].sRect.nHeight = 320;

	pAlterFrameRateInfo->roi_param[1].bEnable = 1;
	pAlterFrameRateInfo->roi_param[1].index = 0;
	pAlterFrameRateInfo->roi_param[1].nQPoffset = 10;
	pAlterFrameRateInfo->roi_param[1].roi_abs_flag = 1;
	pAlterFrameRateInfo->roi_param[1].sRect.nLeft = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nTop = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nWidth = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nHeight = 320;
}

static void init_enc_proc_info(VeProcSet *ve_proc_set)
{
	ve_proc_set->bProcEnable = vencoder_tuning_param->proc_cfg.ProcEnable;
	ve_proc_set->nProcFreq = vencoder_tuning_param->proc_cfg.ProcFreq;
}

static int initH264Func(h264_func_t *h264_func, encode_param_t *encode_param)
{
	memset(h264_func, 0, sizeof(h264_func_t));

    loge("H264_para:bit_rate=%d, dst_height=%u, dst_width=%u, encode_format=%u, \
            encode_frame_num=%u, frame_rate=%d, maxKeyFrame=%d, picture_format=%d, qp_max=%u, \
            qp_min=%u, src_height=%u, src_width=%u",\
		encode_param->bit_rate, encode_param->dst_height,\
		encode_param->dst_width, encode_param->encode_format, encode_param->encode_frame_num, \
		encode_param->frame_rate, encode_param->maxKeyFrame, encode_param->picture_format,\
		encode_param->qp_max, encode_param->qp_min, encode_param->src_height, \
		encode_param->src_width);

	loge("---------ThrdI=%d, ThrdP=%d\n---------", \
		vencoder_tuning_param->common_cfg.ThrdI, vencoder_tuning_param->common_cfg.ThrdP);

	//init h264Param
	h264_func->h264Param.bEntropyCodingCABAC = 1;
	h264_func->h264Param.nBitrate = encode_param->bit_rate;
	h264_func->h264Param.nFramerate = encode_param->frame_rate;
	h264_func->h264Param.nCodingMode = VENC_FRAME_CODING;
	h264_func->h264Param.nMaxKeyInterval = encode_param->maxKeyFrame;
	h264_func->h264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
	h264_func->h264Param.sProfileLevel.nLevel = VENC_H264Level51;

	if(vencoder_tuning_param->vbr_cfg.mMaxBitRate > 0 &&
	     vencoder_tuning_param->vbr_cfg.mMinBitRate > 0)
	{
	    loge("----------------VBR\n");
	    h264_func->h264Param.sRcParam.eRcMode = AW_VBR;
#if 0
	    h264_func->h264Param.sRcParam.sVbrParam.uMaxBitRate =
			vencoder_tuning_param->vbr_cfg.mMaxBitRate;
	    h264_func->h264Param.sRcParam.sVbrParam.nMovingTh = 25;
	    h264_func->h264Param.sRcParam.sVbrParam.nQuality = 8;
#endif
	}

	if(vencoder_tuning_param->avbr_cfg.mMaxBitRate > 0 &&\
	     vencoder_tuning_param->avbr_cfg.mMinBitRate > 0 &&\
	     vencoder_tuning_param->avbr_cfg.mQuality > 0)
	{
	    loge("----------------AVBR\n");
	    h264_func->h264Param.sRcParam.eRcMode = AW_AVBR;
#if 0
	    h264_func->h264Param.sRcParam.sVbrParam.uMaxBitRate =
			vencoder_tuning_param->avbr_cfg.mMaxBitRate;
	    h264_func->h264Param.sRcParam.sVbrParam.nQuality =
			vencoder_tuning_param->avbr_cfg.mQuality;
#endif
	}


#if 0  //default is CBR
	h264_func->h264Param.sRcParam.eRcMode = AW_VBR;
#endif
	if (encode_param->qp_min > 0 && encode_param->qp_max && encode_param->qp_max > encode_param->qp_min) {
		h264_func->h264Param.sQPRange.nMinqp = encode_param->qp_min;
		h264_func->h264Param.sQPRange.nMaxqp = encode_param->qp_max;
	} else {
		h264_func->h264Param.sQPRange.nMinqp = 10;
		h264_func->h264Param.sQPRange.nMaxqp = 50;
	}

	//h264_func->h264Param.bLongRefEnable = 1;
	//h264_func->h264Param.nLongRefPoc = 0;

	h264_func->sH264Smart.img_bin_en = 1;
	h264_func->sH264Smart.img_bin_th = 27;
	h264_func->sH264Smart.shift_bits = 2;
	h264_func->sH264Smart.smart_fun_en = 1;

	//以下函数都是在填充h264_func里面的结构体成员
	//init VencMBModeCtrl
	init_mb_mode(&h264_func->h264MBMode, encode_param);

	//init VencMBInfo
	init_mb_info(&h264_func->MBInfo, encode_param);

	//init VencH264FixQP
	init_fix_qp(&h264_func->fixQP);

	//init VencSuperFrameConfig
	init_super_frame_cfg(&h264_func->sSuperFrameCfg, encode_param);

	//init VencH264SVCSkip
	init_svc_skip(&h264_func->SVCSkip);

	//init VencH264AspectRatio
	init_aspect_ratio(&h264_func->sAspectRatio);

	//init VencH264AspectRatio
	init_video_signal(&h264_func->sVideoSignal);

	//init CyclicIntraRefresh
	init_intra_refresh(&h264_func->sIntraRefresh);

	//init VencROIConfig
	init_roi(h264_func->sRoiConfig);

	//init proc info
	init_enc_proc_info(&h264_func->sVeProcInfo);

	//init VencOverlayConfig
    //init_overlay_info(&h264_func->sOverlayInfo, encode_param);

	return VIDEOCODEC_OK;
}

int initH265Func(h265_func_t *h265_func, encode_param_t *encode_param)
{
	memset(h265_func, 0, sizeof(h264_func_t));

	//init h265Param
	h265_func->h265Param.nBitrate = encode_param->bit_rate;
	h265_func->h265Param.nFramerate = encode_param->frame_rate;
	h265_func->h265Param.sProfileLevel.nProfile = VENC_H265ProfileMain;
	h265_func->h265Param.sProfileLevel.nLevel = VENC_H265Level41;
	h265_func->h265Param.nQPInit = 30;
	h265_func->h265Param.idr_period = 30;
	h265_func->h265Param.nGopSize = h265_func->h265Param.idr_period;
	h265_func->h265Param.nIntraPeriod = h265_func->h265Param.idr_period;

#if 0 //default is CBR
	h265_func->h265Param.sRcParam.eRcMode = AW_VBR;
#endif

	if (encode_param->qp_min > 0 && encode_param->qp_max && encode_param->qp_max > encode_param->qp_min) {
		h265_func->h265Param.sQPRange.nMaxqp = encode_param->qp_min;
		h265_func->h265Param.sQPRange.nMaxqp = encode_param->qp_max;
	} else {
		h265_func->h265Param.sQPRange.nMinqp = 10;
		h265_func->h265Param.sQPRange.nMaxqp = 50;
	}

	//h265_func->h265Param.bLongTermRef = 1;
	h265_func->h265Hvs.hvs_en = 1;
	h265_func->h265Hvs.th_dir = 24;
	h265_func->h265Hvs.th_coef_shift = 4;

	h265_func->h265Trc.inter_tend = 63;
	h265_func->h265Trc.skip_tend = 3;
	h265_func->h265Trc.merge_tend = 0;

	h265_func->h265Smart.img_bin_en = 1;
	h265_func->h265Smart.img_bin_th = 27;
	h265_func->h265Smart.shift_bits = 2;
	h265_func->h265Smart.smart_fun_en = 1;

	h265_func->h265_rc_frame_total = 20*h265_func->h265Param.nGopSize;

	//init H265Gop
	init_h265_gop(&h265_func->h265Gop);

	//init VencMBModeCtrl
	init_mb_mode(&h265_func->h265MBMode, encode_param);

	//init VencMBInfo
	init_mb_info(&h265_func->MBInfo, encode_param);

	//init VencH264FixQP
	init_fix_qp(&h265_func->fixQP);

	//init VencSuperFrameConfig
	init_super_frame_cfg(&h265_func->sSuperFrameCfg, encode_param);

	//init VencH264SVCSkip
	init_svc_skip(&h265_func->SVCSkip);

	//init VencH264AspectRatio
	init_aspect_ratio(&h265_func->sAspectRatio);

	//init VencH264AspectRatio
	init_video_signal(&h265_func->sVideoSignal);

	//init CyclicIntraRefresh
	init_intra_refresh(&h265_func->sIntraRefresh);

	//init VencROIConfig
	init_roi(h265_func->sRoiConfig);

	//init alter frameRate info
	init_alter_frame_rate_info(&h265_func->sAlterFrameRateInfo);

	//init proc info
	init_enc_proc_info(&h265_func->sVeProcInfo);

	//init VencOverlayConfig
	//init_overlay_info(&h265_func->sOverlayInfo, encode_param);

	return VIDEOCODEC_OK;
}

int initJpegFunc(jpeg_func_t *jpeg_func, encode_param_t *encode_param)
{
    memset(jpeg_func, 0, sizeof(jpeg_func_t));

    jpeg_func->quality = 95;
    encode_param->encode_frame_num = 1000;
    if(encode_param->encode_frame_num > 1)
        jpeg_func->jpeg_mode = 1;
    else
        jpeg_func->jpeg_mode = 0;

    if(0 == jpeg_func->jpeg_mode)
        init_jpeg_exif(&jpeg_func->exifinfo);
    else if(1 == jpeg_func->jpeg_mode)
        init_jpeg_rate_ctrl(jpeg_func);
    else
    {
        loge("encoder do not support the jpeg_mode:%d\n", jpeg_func->jpeg_mode);
        return -1;
    }

     //init VencOverlayConfig
    //init_overlay_info(&jpeg_func->sOverlayInfo, encode_param);

    return 0;

}

static int setEncParam(VideoEncoder *pVideoEnc ,encode_param_t *encode_param)
{
	int result = 0;
	VeProcSet mProcSet;
	//mProcSet.bProcEnable = 1;
	//mProcSet.nProcFreq = 30;
	//mProcSet.nStatisBitRateTime = 1000;
	//mProcSet.nStatisFrRateTime  = 1000;

       vencoder_common_cfg_t *p = &vencoder_tuning_param->common_cfg;
       loge("+++++--------+++++DstPicHeight=%d, DstPicWidth=%d, EncodeFormat=%d, enGopMode=%d, FastEncFlag=%d, \
	   	Flag_3DNR=%d, InputPixelFormat=%d, IQpOffset=%d, Level=%d, MaxKeyInterval=%d, \
	   	MaxReEncodeTimes=%d, mBitRate=%d, mbPintraEnable=%d, mInitQp=%d, mMaxQp=%d, \
	   	mMixQp=%d, RCEnable=%d, Rotate=%d, SbmBufSize=%d, SrcPicHeight=%d, SrcPicWidth=%d, \
	   	ThrdI=%d, ThrdP=%d, Profile=%d\n", \
       p->DstPicHeight, p->DstPicWidth, p->EncodeFormat, p->enGopMode, p->FastEncFlag, \
       p->Flag_3DNR, p->InputPixelFormat, p->IQpOffset, p->Level, p->MaxKeyInterval, \
       p->MaxReEncodeTimes, p->mBitRate, p->mbPintraEnable, p->mInitQp, p->mMaxQp, \
       p->mMixQp, p->RCEnable, p->Rotate, p->SbmBufSize, p->SrcPicHeight, p->SrcPicWidth, \
       p->ThrdI, p->ThrdP, p->Profile);

	mProcSet.bProcEnable = vencoder_tuning_param->proc_cfg.ProcEnable;
	mProcSet.nProcFreq = vencoder_tuning_param->proc_cfg.ProcFreq;
	mProcSet.nStatisBitRateTime = vencoder_tuning_param->proc_cfg.StatisBitRateTime;
	mProcSet.nStatisFrRateTime  = 1000;
	unsigned int vbv_size, value;

	VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamProcSet, &mProcSet);

	if(encode_param->encode_format == VENC_CODEC_JPEG)
	{
        result = initJpegFunc(&encode_param->jpeg_func, encode_param);
        if(result)
        {
            loge("initJpegFunc error, return \n");
            return -1;
        }

        if(1 == encode_param->jpeg_func.jpeg_mode)
        {
            unsigned int vbv_size = 4*1024*1024;
		if(p->SbmBufSize > 0)
		{
		    vbv_size = p->SbmBufSize;
		}
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &encode_param->jpeg_func.jpeg_mode);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamBitrate, &encode_param->jpeg_func.jpeg_biteRate);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamFramerate, &encode_param->jpeg_func.jpeg_frameRate);
            VideoEncSetParameter(pVideoEnc,
                                    VENC_IndexParamSetBitRateRange, &encode_param->jpeg_func.bitRateRange);

        }
        else
        {
            unsigned int vbv_size = 4*1024*1024;
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &encode_param->jpeg_func.quality);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &encode_param->jpeg_func.exifinfo);
        }

        if(encode_param->jpeg_func.pRoiConfig.bEnable == 1)
        {
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamRoi, &encode_param->jpeg_func.pRoiConfig);
        }
#if 0
        if(encode_param->test_overlay_flag == 1)
        {
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetOverlay, &encode_param->jpeg_func.sOverlayInfo);
        }
#endif
    }
	else if(encode_param->encode_format == VENC_CODEC_H264)
	{
		result = initH264Func(&encode_param->h264_func, encode_param);
		if(result)
		{
			loge("initH264Func error, return \n");
			return VIDEOCODEC_FAIL;
		}

		if(encode_param->SbmBufSize > 0 )
		{
			vbv_size = encode_param->SbmBufSize;
		}
		else
		{
			vbv_size = VBV_TOTAL_SIZE;
		}

		VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamH264Param, &encode_param->h264_func.h264Param);
		VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);

		if(encode_param->bFastEncFlag > 0)
		{
		    loge("++++++++++++++fastenc\n");
		    value = 1;
		    VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamFastEnc, &value);
		}

             if(encode_param->Rotate > 0)
             {
		    loge("++++++++++++++Rotate=%d\n", encode_param->Rotate);
		    value = (encode_param->Rotate%4)*90;
                 VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamRotation, &value);
             }

             if(vencoder_tuning_param->savebsfile_cfg.Save_bsfile_flag > 0 &&
				memcmp(vencoder_tuning_param->savebsfile_cfg.Filename, "\0", 1) != 0)//vencoder_tuning_param->savebsfile_cfg.Filename != "\0"
             {
                 //sprintf(vencoder_tuning_param->savebsfile_cfg.Filename, "/tmp/test_1.dat");
                 VencSaveBSFile bs_file;
		     memcpy(bs_file.filename, vencoder_tuning_param->savebsfile_cfg.Filename, VENCODER_FILENAME_LEN);
		     bs_file.save_bsfile_flag = 1;
                  VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamSaveBSFile, &bs_file);
             }

             if(encode_param->fixqp.mIQp > 0 && encode_param->fixqp.mPQp > 0)
             {
                 VencFixQP fixQp;
		    fixQp.bEnable = 1;
		    fixQp.nIQp = encode_param->fixqp.mIQp;
		    fixQp.nPQp = encode_param->fixqp.mPQp;
		    //loge("++++++++++++++p=%d, q=%d\n", fixQp.nIQp, fixQp.nPQp);
                 //VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamH264FixQP, &fixQp);
             }

		if(encode_param->mbPintraEnable > 0)
		{
		    loge("++++++++++++++mbPintraEnable\n");
		     value = 1;
                  VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamPFrameIntraEn, &value);
		}

		if(encode_param->IQpOffset > 0)
		{
		    loge("++++++++++++++IQpOffset=%d\n", encode_param->IQpOffset);
		     value = encode_param->IQpOffset;
                  VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamIQpOffset, &value);
		}

		if(encode_param->n3DNR > 0)
		{
		    loge("++++++++++++++n3DNR=%d\n", encode_param->n3DNR);
		     value = encode_param->n3DNR;
                  VideoEncSetParameterDl(pVideoEnc, VENC_IndexParam3DFilter, &value);
		}

		VideoEncSetParameterDl(pVideoEnc, VENC_IndexParamProcSet, &encode_param->h264_func.sVeProcInfo);
	}
	else if(encode_param->encode_format == VENC_CODEC_H265)
	{
		result = initH265Func(&encode_param->h265_func, encode_param);
		if(result)
		{
			loge("initH265Func error, return \n");
			return VIDEOCODEC_FAIL;
		}
		if(encode_param->SbmBufSize > 0 )
		{
		    vbv_size = encode_param->SbmBufSize;
		}
		else
		{
		    vbv_size = VBV_TOTAL_SIZE;
		}
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH265Param, &encode_param->h265_func.h265Param);

		value = 1;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamChannelNum, &value);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamProcSet, &encode_param->h265_func.sVeProcInfo);
	}

	return VIDEOCODEC_OK;
}

static void releaseMb(encode_param_t *encode_param)
{
	VencMBInfo *pMBInfo;
	VencMBModeCtrl *pMBMode;
	if (encode_param->encode_format == VENC_CODEC_H264 && encode_param->h264_func.h264MBMode.mode_ctrl_en) {
		pMBInfo = &encode_param->h264_func.MBInfo;
		pMBMode = &encode_param->h264_func.h264MBMode;
	} else if (encode_param->encode_format == VENC_CODEC_H265 && encode_param->h265_func.h265MBMode.mode_ctrl_en) {
		pMBInfo = &encode_param->h265_func.MBInfo;
		pMBMode = &encode_param->h265_func.h265MBMode;
	} else {
		return;
	}

	if(pMBInfo->p_para)
	{
		free(pMBInfo->p_para);
		pMBInfo->p_para = NULL;
	}
	if(pMBMode->p_info)
	{
		free(pMBMode->p_info);
		pMBMode->p_info = NULL;
	}
}

static int encoder_doProcessData(encode_param_t *encode_param,
                                       VencInputBuffer *inputBuffer,
                                       VencOutputBuffer *outputBuffer)
{
	unsigned int done_Number = 0;
	long long time1, time2;
	int ret;

#ifdef INPUTSOURCE_FILE
	AllocInputBufferDl(pVideoEnc, &encode_param->bufferParam);
	inputBuffer = &encode_param->inputBuffer;
#endif

	//while(done_Number < encode_param->encode_frame_num)
	{
#ifdef INPUTSOURCE_FILE
		GetOneAllocInputBufferDl(pVideoEnc, inputBuffer);
		unsigned int size1, size2;
		size1 = fread(inputBuffer->pAddrVirY, 1,
		encode_param->src_size, encode_param->in_file);
		size2 = fread(inputBuffer->pAddrVirC, 1,
		encode_param->src_size/2, encode_param->in_file);
		if((size1!= encode_param->src_size) || (size2!= encode_param->src_size/2))
		{
			fseek(encode_param->in_file, 0L, SEEK_SET);
			size1 = fread(inputBuffer->pAddrVirY, 1,
							encode_param->src_size, encode_param->in_file);
			size2 = fread(inputBuffer->pAddrVirC, 1,
							encode_param->src_size/2, encode_param->in_file);
		}
		inputBuffer->bEnableCorp = 0;
		FlushCacheAllocInputBufferDl(pVideoEnc, inputBuffer);

		inputBuffer->nPts += 1*1000/encode_param->frame_rate;

		AddOneInputBufferDl(pVideoEnc, inputBuffer);
		time1 = GetNowUs();
		ret = VideoEncodeOneFrameDl(pVideoEnc);
		if(ret < 0)
		{
			loge("encoder error, goto out");
			return VIDEOCODEC_FAIL;
		}

		time2 = GetNowUs();
		printf("encode frame %u use time is %lldus..\n\n", done_Number,(time2-time1));
		AlreadyUsedInputBufferDl(pVideoEnc, inputBuffer);

		ReturnOneAllocInputBufferDl(pVideoEnc, inputBuffer);

		ret = GetOneBitstreamFrameDl(pVideoEnc, &encode_param->outputBuffer);
		if(ret == -1)
		{
			loge("Error - Get one stream frame fail!");
			return VIDEOCODEC_FAIL;
		}

		fwrite(encode_param->outputBuffer.pData0, 1, encode_param->outputBuffer.nSize0, encode_param->out_file);
		if(encode_param->outputBuffer.nSize1)
		{
			fwrite(encode_param->outputBuffer.pData1, 1, encode_param->outputBuffer.nSize1, encode_param->out_file);
		}

		FreeOneBitStreamFrameDl(pVideoEnc, &encode_param->outputBuffer);
		done_Number++;
#else
		inputBuffer->bEnableCorp = 0;
		inputBuffer->sCropInfo.nLeft =  240;
		inputBuffer->sCropInfo.nTop  =  240;
		inputBuffer->sCropInfo.nWidth  =  240;
		inputBuffer->sCropInfo.nHeight =  240;
		ret = FlushCacheAllocInputBufferDl(pVideoEnc, inputBuffer);
		if(ret)
		{
			loge("FlushCacheAllocInputBuffer error, ret=%d", ret);
			return VIDEOCODEC_FAIL;
		}

		inputBuffer->nPts += 1*1000/encode_param->frame_rate;

		ret = AddOneInputBufferDl(pVideoEnc, inputBuffer);
		if(ret)
		{
			loge("AddOneInputBuffer error, ret=%d", ret);
			return VIDEOCODEC_FAIL;
		}

		time1 = GetNowUs();
		ret = VideoEncodeOneFrameDl(pVideoEnc);
		if(ret < 0)
		{
			loge("encoder error, goto out， ret=%d", ret);
			return VIDEOCODEC_FAIL;
		}
		time2 = GetNowUs();
		logt("encode frame %u use time is %lldus..", done_Number,(time2-time1));

		AlreadyUsedInputBufferDl(pVideoEnc, inputBuffer);
		//ReturnOneAllocInputBufferDl(pVideoEnc, inputBuffer);

		ret = GetOneBitstreamFrameDl(pVideoEnc, outputBuffer);
		if(ret == -1)
		{
			loge("Error - Get one stream frame fail!");
			return VIDEOCODEC_FAIL;
		}
		done_Number++;
#endif
	}

	return VIDEOCODEC_OK;
}

int FreeEncoderOutputBuffer(VencOutputBuffer *outputBuffer)
{
	int ret;
	ret = FreeOneBitStreamFrameDl(pVideoEnc, outputBuffer);
	return ret;
}

int OpenEncoder(VENC_CODEC_TYPE type)
{
	loge("isp preview vencode version:%s", VERSION_STRING);

	if(type > VENC_CODEC_VP8 || type < VENC_CODEC_H264)
	{
		loge("Error - typs is illegal, please check the video codec tpye!");
		return VIDEOCODEC_FAIL;
	}
#ifdef VENC_DYNAMIC_LIBRARY
	loge("Link vencode function with dynamic library");
	//********load dynamic library********
	vencoderLib = dlopen("/usr/lib/libvencoder.so", RTLD_NOW);
	if(vencoderLib == NULL)
	{
		loge("Could not open /usr/lib/libvencoder.so library: %s", dlerror());
		return VIDEOCODEC_FAIL;
	}

	/* Clear any existing error */
	dlerror();

	VideoEncCreateDl              = (VideoEncCreateFunc)dlsym(vencoderLib, "VideoEncCreate");
	VideoEncDestroyDl             = (VideoEncDestroyFunc)dlsym(vencoderLib, "VideoEncDestroy");
	VideoEncInitDl                = (VideoEncInitFunc)dlsym(vencoderLib, "VideoEncInit");
	AllocInputBufferDl            = (AllocInputBufferFunc)dlsym(vencoderLib, "AllocInputBuffer");
	GetOneAllocInputBufferDl      = (GetOneAllocInputBufferFunc)dlsym(vencoderLib, "GetOneAllocInputBuffer");
	FlushCacheAllocInputBufferDl  = (FlushCacheAllocInputBufferFunc)dlsym(vencoderLib, "FlushCacheAllocInputBuffer");
	ReturnOneAllocInputBufferDl   = (ReturnOneAllocInputBufferFunc)dlsym(vencoderLib, "ReturnOneAllocInputBuffer");
	AddOneInputBufferDl           = (AddOneInputBufferFunc)dlsym(vencoderLib, "AddOneInputBuffer");
	VideoEncodeOneFrameDl         = (VideoEncodeOneFrameFunc)dlsym(vencoderLib, "VideoEncodeOneFrame");
	AlreadyUsedInputBufferDl      = (AlreadyUsedInputBufferFunc)dlsym(vencoderLib, "AlreadyUsedInputBuffer");
	GetOneBitstreamFrameDl        = (GetOneBitstreamFrameFunc)dlsym(vencoderLib, "GetOneBitstreamFrame");
	FreeOneBitStreamFrameDl       = (FreeOneBitStreamFrameFunc)dlsym(vencoderLib, "FreeOneBitStreamFrame");
	VideoEncGetParameterDl        = (VideoEncGetParameterFunc)dlsym(vencoderLib, "VideoEncGetParameter");
	VideoEncSetParameterDl        = (VideoEncSetParameterFunc)dlsym(vencoderLib, "VideoEncSetParameter");
	VideoEncoderSetFreqDl         = (VideoEncoderSetFreqFunc)dlsym(vencoderLib, "VideoEncoderSetFreq");
#elif defined VENC_STATIC_LIBRARY
	loge("Link vencode function with static library");
	VideoEncCreateDl              = &VideoEncCreate;
	VideoEncDestroyDl             = &VideoEncDestroy;
	VideoEncInitDl                = &VideoEncInit;
	AllocInputBufferDl            = &AllocInputBuffer;
	GetOneAllocInputBufferDl      = &GetOneAllocInputBuffer;
	FlushCacheAllocInputBufferDl  = &FlushCacheAllocInputBuffer;
	ReturnOneAllocInputBufferDl   = &ReturnOneAllocInputBuffer;
	AddOneInputBufferDl           = &AddOneInputBuffer;
	VideoEncodeOneFrameDl         = &VideoEncodeOneFrame;
	AlreadyUsedInputBufferDl      = &AlreadyUsedInputBuffer;
	GetOneBitstreamFrameDl        = &GetOneBitstreamFrame;
	FreeOneBitStreamFrameDl       = &FreeOneBitStreamFrame;
	VideoEncGetParameterDl        = &VideoEncGetParameter;
	VideoEncSetParameterDl        = &VideoEncSetParameter;
	VideoEncoderSetFreqDl         = &VideoEncoderSetFreq;
#endif

	if(!VideoEncCreateDl || !VideoEncDestroyDl || !VideoEncInitDl || !AllocInputBufferDl \
	|| !GetOneAllocInputBufferDl || !FlushCacheAllocInputBufferDl || !ReturnOneAllocInputBufferDl \
	|| !AddOneInputBufferDl || !VideoEncodeOneFrameDl || !AlreadyUsedInputBufferDl || !GetOneBitstreamFrameDl \
	|| !FreeOneBitStreamFrameDl || !VideoEncGetParameterDl || !VideoEncSetParameterDl || !VideoEncoderSetFreqDl)
	{
#ifdef VENC_DYNAMIC_LIBRARY
		dlclose(vencoderLib);
		vencoderLib = NULL;
		loge("Could not find symbol: %s", dlerror());
#endif
		loge("Could not get vencode function symbol");
		VideoEncCreateDl = NULL;
		VideoEncDestroyDl = NULL;
		VideoEncInitDl = NULL;
		AllocInputBufferDl = NULL;
		GetOneAllocInputBufferDl = NULL;
		FlushCacheAllocInputBufferDl = NULL;
		ReturnOneAllocInputBufferDl = NULL;
		AddOneInputBufferDl = NULL;
		VideoEncodeOneFrameDl = NULL;
		AlreadyUsedInputBufferDl = NULL;
		GetOneBitstreamFrameDl = NULL;
		FreeOneBitStreamFrameDl = NULL;
		VideoEncGetParameterDl = NULL;
		VideoEncSetParameterDl = NULL;
		VideoEncoderSetFreqDl = NULL;

		return VIDEOCODEC_FAIL;
	}
	//********end load********************
	pVideoEnc = VideoEncCreateDl(type);
	if(pVideoEnc == NULL)
	{
		loge("Error - Create Video encoder fail!");
		return VIDEOCODEC_FAIL;
	}
	return VIDEOCODEC_OK;
}

int PrepareEncoder(encode_param_t *encode_param)
{
	int ret;
	VencBaseConfig baseConfig;

	if(pVideoEnc == NULL)
	{
		loge("Error - Video encoder has not been create normally, should call openEnc first");
		return VIDEOCODEC_FAIL;
	}

	encode_param->inputBuffer.bEnableCorp = 0;
	memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
	baseConfig.nInputWidth= encode_param->src_width;
	baseConfig.nInputHeight = encode_param->src_height;
	baseConfig.nStride = encode_param->src_width;
	baseConfig.nDstWidth = encode_param->dst_width;
	baseConfig.nDstHeight = encode_param->dst_height;
	baseConfig.eInputFormat = encode_param->picture_format;
	baseConfig.bEncH264Nalu = 0;
	ret = setEncParam(pVideoEnc, encode_param);
	if(ret)
	{
		loge("Error - Set Encode Paramant fail");
		goto out;
	}

	ret = VideoEncInitDl(pVideoEnc, &baseConfig);
	if(ret)
	{
		loge("Error - Video Encode Init fail");
		goto out;
	}
	return VIDEOCODEC_OK;

out:
	CloseEncoder(encode_param);
	return VIDEOCODEC_FAIL;
}

int StartEncoder(encode_param_t *encode_param, VencInputBuffer *input, VencOutputBuffer *output, vencode_command type)
{
	int ret = -1;
	int head_num;

	if (type == VENCODE_CMD_HEAD_PPSSPS &&
	    (encode_param->encode_format == VENC_CODEC_H264 ||
	     encode_param->encode_format == VENC_CODEC_H265 ||
	     encode_param->encode_format == VENC_CODEC_JPEG))
	{
#ifdef INPUTSOURCE_FILE
		ret = VideoEncGetParameterDl(pVideoEnc, VENC_IndexParamH264SPSPPS, &encode_param->sps_pps_data);
		fwrite(encode_param->sps_pps_data.pBuffer, 1, encode_param->sps_pps_data.nLength, encode_param->out_file);
		logt("sps_pps_data.nLength: %d\n", encode_param->sps_pps_data.nLength);
		for(head_num = 0; head_num < encode_param->sps_pps_data.nLength; head_num++)
			logt("the sps_pps :%02x\n", *(encode_param->sps_pps_data.pBuffer+head_num));
#else
		if (encode_param->encode_format == VENC_CODEC_H264) {
			ret = VideoEncGetParameterDl(pVideoEnc, VENC_IndexParamH264SPSPPS, &encode_param->sps_pps_data);
		} else if (encode_param->encode_format == VENC_CODEC_H265){
			ret = VideoEncGetParameterDl(pVideoEnc, VENC_IndexParamH265Header, &encode_param->sps_pps_data);
		}else if (encode_param->encode_format == VENC_CODEC_JPEG){
                   ret = 0;
		}
		if (ret) {
			CloseEncoder(encode_param);
			loge("get video parameter fail");
			return VIDEOCODEC_FAIL;
		}
		for (head_num = 0; head_num < encode_param->sps_pps_data.nLength; head_num++)
			logt("the sps_pps :%02x\n", *(encode_param->sps_pps_data.pBuffer+head_num));
#endif
	}
	else if(type == VENCODE_CMD_STREAM && \
		(encode_param->encode_format == VENC_CODEC_H264 ||
		 encode_param->encode_format == VENC_CODEC_H265 ||
		 encode_param->encode_format == VENC_CODEC_JPEG))
	{
		ret = encoder_doProcessData(encode_param, input, output);
		if(ret)
		{
			CloseEncoder(encode_param);
			loge("process encoder data fail");
			return VIDEOCODEC_FAIL;
		}
	}
	else {
		loge("unknown cmd = %d or is not supported encoder format = %d", type, encode_param->encode_format);
		return VIDEOCODEC_FAIL;
	}

	return VIDEOCODEC_OK;
}

int CloseEncoder(encode_param_t *encode_param)
{
	if(pVideoEnc)
	{
		VideoEncDestroyDl(pVideoEnc);
		pVideoEnc = NULL;
	}
#ifdef VENC_DYNAMIC_LIBRARY
	//libvencoder.so should be dlclose after other Func run
	if(vencoderLib)
	{
			dlclose(vencoderLib);
			vencoderLib = NULL;
	}
#endif

#ifdef INPUTSOURCE_FILE
	if(encode_param->in_file)
		fclose(encode_param->in_file);

	if(encode_param->out_file)
		fclose(encode_param->out_file);
#endif
	releaseMb(encode_param);
	return VIDEOCODEC_OK;
}
