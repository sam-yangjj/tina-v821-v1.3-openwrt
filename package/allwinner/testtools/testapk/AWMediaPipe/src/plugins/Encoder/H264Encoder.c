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
#include "H264Encoder.h"
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include "memoryAdapter.h"
#include "../yolov5/draw_detections/draw_result.h"
#include "../utils/log.h"

int64_t pts = 0;

/**
 * @brief Initialize the H.264 encoder
 *
 * @param enc Pointer to H264Encoder structure
 * @param config Encoder configuration parameters
 * @return true on success, false on failure
 */
bool h264_encoder_init(H264Encoder* enc, encoderConfig* config) {
    enc->width = config->h264_width;
    enc->height = config->h264_height;
    enc->fps = config->h264_fps;
    enc->bitrate = config->h264_bitrate;
    enc->input_format = config->h264_input_format;
    // 注意：移除了frame_buffer参数，不再设置enc->frame_buffer

    pthread_mutex_init(&enc->mutex, NULL);

    // 初始化编码器相关结构体
    enc->pVideoEnc = VideoEncCreate(VENC_CODEC_H264);

    VencBaseConfig* baseConfig = (VencBaseConfig*)malloc(sizeof(VencBaseConfig));
    memset(baseConfig, 0, sizeof(VencBaseConfig));
    baseConfig->memops = MemAdapterGetOpsS();
     if(baseConfig->memops == NULL){
        LOG_DEBUG("MemAdapterGetOpsS failed\n");
        if(enc->pVideoEnc)
            VideoEncDestroy(enc->pVideoEnc);
        return false;
    }
    CdcMemOpen(baseConfig->memops);
    baseConfig->nInputWidth = ALIGN_16B(enc->width);
    baseConfig->nInputHeight = ALIGN_16B(enc->height);
    baseConfig->nStride = ALIGN_16B(enc->width);
    baseConfig->nDstWidth = ALIGN_16B(enc->width);
    baseConfig->nDstHeight = ALIGN_16B(enc->height);

    VencAllocateBufferParam* bufferParam = (VencAllocateBufferParam*)malloc(sizeof(VencAllocateBufferParam));
    memset(bufferParam, 0, sizeof(VencAllocateBufferParam));
    // 根据输入格式设置像素格式
    baseConfig->eInputFormat = enc->input_format;
    switch (baseConfig->eInputFormat) {
        case VENC_PIXEL_RGBA:
            bufferParam->nSizeY = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight) * 4;
            bufferParam->nSizeC = 0;
            break;
        case VENC_PIXEL_YUV420P:
        case VENC_PIXEL_YVU420P:
        case VENC_PIXEL_YUV420SP:
        case VENC_PIXEL_YVU420SP:
            bufferParam->nSizeY = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight);
            bufferParam->nSizeC = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight) / 2;
            break;
        case VENC_PIXEL_YUV422SP:
        case VENC_PIXEL_YVU422SP:
        case VENC_PIXEL_YUV422P:
        case VENC_PIXEL_YVU422P:
        case VENC_PIXEL_YUYV422:
        case VENC_PIXEL_UYVY422:
        case VENC_PIXEL_YVYU422:
        case VENC_PIXEL_VYUY422:
            bufferParam->nSizeY = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight);
            bufferParam->nSizeC = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight);
            break;
        default:
            // 默认使用RGBA格式
            bufferParam->nSizeY = ALIGN_16B(baseConfig->nInputWidth) * ALIGN_16B(baseConfig->nInputHeight) * 4;
            bufferParam->nSizeC = 0;
            break;
    }
    enc->baseConfig = baseConfig;
    bufferParam->nBufferNum = 6;
    enc->bufferParam = bufferParam;

    VencH264Param* h264Param = (VencH264Param*)malloc(sizeof(VencH264Param));
    memset(h264Param, 0, sizeof(VencH264Param));
    h264Param->bEntropyCodingCABAC = 1;
    h264Param->nBitrate = enc->bitrate;
    h264Param->nFramerate = enc->fps;
    h264Param->nCodingMode = VENC_FRAME_CODING;
    h264Param->nMaxKeyInterval = 15;
    h264Param->sProfileLevel.nProfile = VENC_H264ProfileBaseline;
    h264Param->sProfileLevel.nLevel = VENC_H264Level31;
    h264Param->sQPRange.nMinqp = 10;
    h264Param->sQPRange.nMaxqp = 40;
    //h264Param->sRcParam.eRcMode = AW_CBR; //CBR用于直播、VBR用于存储
    enc->h264Param = h264Param;

    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamH264Param, h264Param);

    int value = 0;
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamIfilter, &value);

    value = 0;
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamRotation, &value);

    value = 0;
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamSetPSkip, &value);

    VideoEncInit(enc->pVideoEnc, baseConfig);

    unsigned int head_num = 0;
    enc->sps_pps_data = calloc(1,sizeof(VencHeaderData));
    VideoEncGetParameter(enc->pVideoEnc, VENC_IndexParamH264SPSPPS, enc->sps_pps_data);

    //AllocInputBuffer(enc->pVideoEnc, bufferParam);
    //enc->outputBuffer = (VencOutputBuffer*)calloc(1,sizeof(VencOutputBuffer));
    return true;
}

/**
 * @brief Destroy the H.264 encoder and free resources
 *
 * @param enc Pointer to H264Encoder structure
 */
void h264_encoder_destroy(H264Encoder* enc) {
    pthread_mutex_destroy(&enc->mutex);
    //ReleaseAllocInputBuffer(enc->pVideoEnc);
    if(enc->sps_pps_data)
        free(enc->sps_pps_data);

    //VideoEncUnInit(enc->pVideoEnc);

    if(enc->pVideoEnc){
        VideoEncDestroy(enc->pVideoEnc);
        enc->pVideoEnc = NULL;
    }

    if(enc->bufferParam){
        free(enc->bufferParam);
        enc->bufferParam = NULL;
    }
    if(enc->h264Param){
        free(enc->h264Param);
        enc->h264Param = NULL;
    }

    if(enc->baseConfig->memops){
        CdcMemClose(enc->baseConfig->memops);
        enc->baseConfig->memops = NULL;
    }

    if(enc->baseConfig){
        free(enc->baseConfig);
        enc->baseConfig = NULL;
    }
}

bool h264_encoder_encode(H264Encoder* enc, VencInputBuffer* inBuff, VencOutputBuffer* outputBuf, size_t* out_len) {
    LOG_DEBUG("enter h264_encoder_encode");
    pthread_mutex_lock(&enc->mutex);
    bool ret = false;

    //LOG_DEBUG("FlushCacheAllocInputBuffer\n");
    //FlushCacheAllocInputBuffer(enc->pVideoEnc, inBuff);
    inBuff->bEnableCorp = 0;
    pts += 1*1000/20;
    inBuff->nPts = pts;
    inBuff->envLV = 250;
    AddOneInputBuffer(enc->pVideoEnc, inBuff);
    VideoEncodeOneFrame(enc->pVideoEnc);
    AlreadyUsedInputBuffer(enc->pVideoEnc, inBuff);
    //ReturnOneAllocInputBuffer(enc->pVideoEnc, inBuff);

    LOG_DEBUG("GetOneBitstreamFrame\n");

    // 获取编码输出
    int result = GetOneBitstreamFrame(enc->pVideoEnc, outputBuf);
    LOG_DEBUG("GetOneBitstreamFrame finish\n");

    if (result == 0) {
        LOG_DEBUG("GetOneBitstreamFrame success!\n");
        *out_len = outputBuf->nSize0 + outputBuf->nSize1;
        ret = true;
    }
    //FreeOneBitStreamFrame(enc->pVideoEnc, outputBuf);

    pthread_mutex_unlock(&enc->mutex);
    return ret;
}
