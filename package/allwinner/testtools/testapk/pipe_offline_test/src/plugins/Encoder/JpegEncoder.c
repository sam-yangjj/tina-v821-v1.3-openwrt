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
#include "JpegEncoder.h"
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include "memoryAdapter.h"
#include "../../utils/log.h"


#define JPEG_SIZE_1080P  (1920*1080)
#define JPEG_SIZE_2K     (2560*1920)
#define JPEG_SIZE_4K     (3840*2160)
int64_t pts = 0;

void InitJpegExif(EXIFInfo *exifinfo)
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


/**
 * @brief Initialize the H.264 encoder
 *
 * @param enc Pointer to H264Encoder structure
 * @param config Encoder configuration parameters
 * @return true on success, false on failure
 */
bool jpeg_encoder_init(JPEGEncoder* enc, encoderConfig* config) {
    enc->width = config->src_width;
    enc->height = config->src_height;
    enc->fps = config->fps;
    enc->input_format = config->input_format;
    // 注意：移除了frame_buffer参数，不再设置enc->frame_buffer

    pthread_mutex_init(&enc->mutex, NULL);

    // 初始化编码器相关结构体
    enc->pVideoEnc = VideoEncCreate(VENC_CODEC_JPEG);

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

    unsigned int pixels = 0;
    int vbvSize =0;
    pixels = enc->width * enc->height;
    if(pixels <=JPEG_SIZE_1080P)
    {
        vbvSize = 2*1024*1024;
    }else if(pixels >JPEG_SIZE_1080P && pixels <=JPEG_SIZE_4K)
    {
        vbvSize = 4*1024*1024;
    }else{
        vbvSize = 16*1024*1024;
    }
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamSetVbvSize, &vbvSize);

    InitJpegExif(&enc->exifinfo);
    int quality = 90;
    int jpeg_mode = 1;
    VencJpegVideoSignal vs;
    vs.src_colour_primaries = VENC_YCC;
    vs.dst_colour_primaries = VENC_BT601;
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamJpegQuality, &quality);
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
    VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamJpegVideoSignal, &vs);

    if(1 == jpeg_mode){
        int jpeg_biteRate = 12*1024*1024;
        int jpeg_frameRate = 30;
        VencBitRateRange bitRateRange;
        bitRateRange.bitRateMax = 14*1024*1024;
        bitRateRange.bitRateMin = 10*1024*1024;

        VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamBitrate, &jpeg_biteRate);
        VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamFramerate, &jpeg_frameRate);
        VideoEncSetParameter(enc->pVideoEnc, VENC_IndexParamSetBitRateRange, &bitRateRange);
    }

    VideoEncInit(enc->pVideoEnc, baseConfig);

    return true;
}

/**
 * @brief Destroy the H.264 encoder and free resources
 *
 * @param enc Pointer to H264Encoder structure
 */
void jpeg_encoder_destroy(JPEGEncoder* enc) {
    pthread_mutex_destroy(&enc->mutex);
    //ReleaseAllocInputBuffer(enc->pVideoEnc);

    //VideoEncUnInit(enc->pVideoEnc);

    if(enc->pVideoEnc){
        VideoEncDestroy(enc->pVideoEnc);
        enc->pVideoEnc = NULL;
    }

    if(enc->bufferParam){
        free(enc->bufferParam);
        enc->bufferParam = NULL;
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

bool jpeg_encoder_encode(JPEGEncoder* enc, VencInputBuffer* inBuff, VencOutputBuffer* outputBuf, size_t* out_len) {
    LOG_DEBUG("enter h264_encoder_encode");
    pthread_mutex_lock(&enc->mutex);
    bool ret = false;

    //LOG_DEBUG("FlushCacheAllocInputBuffer\n");
    //FlushCacheAllocInputBuffer(enc->pVideoEnc, inBuff);
    inBuff->bEnableCorp = 0;
    inBuff->sCropInfo.nLeft =  240;
    inBuff->sCropInfo.nTop =  240;
    inBuff->sCropInfo.nWidth =  240;
    inBuff->sCropInfo.nHeight =  240;

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
