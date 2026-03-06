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
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT                                                                                                                                                                              * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)                                                                                                                                                                                  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,                                                                                                                                                                       * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <vencoder.h>
#include <memoryAdapter.h>

#define ENCODE_SRC_WIDTH    1280
#define ENCODE_SRC_HEIGHT   720
#define ENCODE_FRAMERATE    30
#define ENCODE_BITRATE      2*1024*1024
#define ENCODE_VBV_SIZE     2*1024*1024

struct vertical_block {
    unsigned char y;
    uint16_t c;
};

typedef struct {
    VencH264Param           h264Param;
    VencSuperFrameConfig    sSuperFrameCfg;
    VeProcSet               sVeProcInfo;
    VideoEncoder            *pVideoEnc;
    FILE                    *fp;
}h264_func_t;

static void vertical_block_make_color(struct vertical_block *vb, unsigned char y, uint16_t c)
{
    vb->y = y;
    vb->c = c;
}

static void draw_nv21_buffer(unsigned int w, unsigned int h, unsigned char *y, unsigned char *uv, int sequence)
{
    struct vertical_block vb[4];

    switch (sequence) {

    case 0:
        vertical_block_make_color(&vb[0], 0x51, 0x5af0);
        vertical_block_make_color(&vb[1], 0x91, 0x3622);
        vertical_block_make_color(&vb[2], 0x29, 0xf06e);
        vertical_block_make_color(&vb[3], 0xd2, 0x1092);
        break;

    case 1:
        vertical_block_make_color(&vb[3], 0x51, 0x5af0);
        vertical_block_make_color(&vb[0], 0x91, 0x3622);
        vertical_block_make_color(&vb[1], 0x29, 0xf06e);
        vertical_block_make_color(&vb[2], 0xd2, 0x1092);
        break;

    case 2:
        vertical_block_make_color(&vb[2], 0x51, 0x5af0);
        vertical_block_make_color(&vb[3], 0x91, 0x3622);
        vertical_block_make_color(&vb[0], 0x29, 0xf06e);
        vertical_block_make_color(&vb[1], 0xd2, 0x1092);
        break;

    case 3:
        vertical_block_make_color(&vb[1], 0x51, 0x5af0);
        vertical_block_make_color(&vb[2], 0x91, 0x3622);
        vertical_block_make_color(&vb[3], 0x29, 0xf06e);
        vertical_block_make_color(&vb[0], 0xd2, 0x1092);
        break;

    default:
        break;

    }

    int block = w / 4;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            if (j < block)
                y[i * w + j] = vb[0].y;
            else if ((j >= block) && (j < (block * 2)))
                y[i * w + j] = vb[1].y;
            else if ((j >= block * 2) && (j < (block * 3)))
                y[i * w + j] = vb[2].y;
            else if ((j >= block * 3) && (j < (block * 4)))
                y[i * w + j] = vb[3].y;
        }
    }

    block = w / 4 / 2;
    uint16_t *tmp = (uint16_t *)uv;
    for (int i = 0; i < h / 2; ++i) {
        for (int j = 0; j < w / 2; ++j) {
            if (j < block)
                tmp[i * (w/2) + j] = vb[0].c;
            else if ((j >= block) && (j < (block * 2)))
                tmp[i * (w/2) + j] = vb[1].c;
            else if ((j >= block * 2) && (j < (block * 3)))
                tmp[i * (w/2) + j] = vb[2].c;
            else if ((j >= block * 3) && (j < (block * 4)))
                tmp[i * (w/2) + j] = vb[3].c;
        }
    }
}

int Xmain(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <width> <height> <color_mode>\n", argv[0]);
        printf("Supported modes: rgb, bgr, grb, rbg\n");
        return -1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int sequence = atoi(argv[3]);

    // NV21 数据大小
    int y_size = width * height;
    int uv_size = width * height / 2;
    uint8_t *nv21_data = (uint8_t *)malloc(y_size + uv_size);
    if (!nv21_data) {
        printf("Memory allocation failed\n");
        return -1;
    }

    draw_nv21_buffer(width, height, nv21_data, nv21_data + y_size, sequence);

    uint8_t *y_plane = nv21_data;
    uint8_t *uv_plane = nv21_data + y_size;

    // 写入文件
    char filename[256];
    snprintf(filename, sizeof(filename), "%dx%d_%d.colorbar.nv21", width, height, sequence);
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("Failed to open file: %s\n", filename);
        free(nv21_data);
        return -1;
    }

    fwrite(nv21_data, 1, y_size + uv_size, fp);
    fclose(fp);
    free(nv21_data);

    printf("Colorbar NV21 file saved as: %s\n", filename);
    return 0;
}

static int EventHandler(VideoEncoder* pEncoder, void* pAppData, VencEventType eEvent,
                             unsigned int nData1, unsigned int nData2, void* pEventData)
{
    switch(eEvent)
    {
        case VencEvent_UpdateMbModeInfo:
        {
            break;
        }
        case VencEvent_UpdateMbStatInfo:
        {
            break;
        }
        case VencEvent_UpdateIspToVeParam:
        {
            break;
        }
        default:
            break;
    }

    return 0;
}

static int InputBufferDone(VideoEncoder* pEncoder,    void* pAppData,
                          VencCbInputBufferDoneInfo* pBufferDoneInfo)
{
    return 0;
}

int initH264Func(h264_func_t *h264_func)
{
    h264_func->h264Param.nCodingMode = VENC_FRAME_CODING;
    h264_func->h264Param.bEntropyCodingCABAC = 1;
    h264_func->h264Param.nBitrate = ENCODE_BITRATE;
    h264_func->h264Param.nFramerate = ENCODE_FRAMERATE;
    h264_func->h264Param.nMaxKeyInterval = ENCODE_FRAMERATE;
    h264_func->h264Param.nCodingMode = VENC_FRAME_CODING;
    h264_func->h264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
    h264_func->h264Param.sProfileLevel.nLevel = VENC_H264Level51;
    h264_func->h264Param.sRcParam.eRcMode = AW_VBR;
    h264_func->h264Param.sQPRange.nMinqp = 25;
    h264_func->h264Param.sQPRange.nMaxqp = 47;
    h264_func->h264Param.sQPRange.nMaxPqp = 25;
    h264_func->h264Param.sQPRange.nMinPqp = 47;
    h264_func->h264Param.sQPRange.nQpInit = 37;
    h264_func->h264Param.sQPRange.bEnMbQpLimit = 1;
    h264_func->h264Param.sRcParam.sVbrParam.uMaxBitRate = ENCODE_BITRATE;
    h264_func->h264Param.sRcParam.sVbrParam.nMovingTh = 20;
    h264_func->h264Param.sRcParam.sVbrParam.nQuality = 1;
    h264_func->h264Param.sRcParam.sVbrParam.nIFrmBitsCoef = 10;
    h264_func->h264Param.sRcParam.sVbrParam.nPFrmBitsCoef = 10;
    h264_func->h264Param.sGopParam.bUseGopCtrlEn = 1;
    h264_func->h264Param.sGopParam.eGopMode = AW_NORMALP;

    h264_func->sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
    h264_func->sSuperFrameCfg.nMaxIFrameBits = 30000*8;
    h264_func->sSuperFrameCfg.nMaxPFrameBits = 15000*8;

    h264_func->sVeProcInfo.bProcEnable = 1;
    h264_func->sVeProcInfo.nProcFreq = 120;
    h264_func->sVeProcInfo.nStatisBitRateTime = 1000;
    h264_func->sVeProcInfo.nStatisFrRateTime = 1000;

    return 0;
}

static int setEncParam(VideoEncoder *pVideoEnc, h264_func_t *h264_func)
{
    int result = 0;
    VeProcSet mProcSet;
    mProcSet.bProcEnable = 0;
    mProcSet.nProcFreq = 30;
    mProcSet.nStatisBitRateTime = 1000;
    mProcSet.nStatisFrRateTime  = 1000;
    VencSetParameter(pVideoEnc, VENC_IndexParamProcSet, &mProcSet);

    initH264Func(h264_func);

    VencProductModeInfo ProductInfo;
    memset(&ProductInfo, 0, sizeof(VencProductModeInfo));
    ProductInfo.eProductMode = PRODUCT_STATIC_IPC;
    ProductInfo.nBitrate = ENCODE_BITRATE;
    ProductInfo.nFrameRate = ENCODE_FRAMERATE;
    ProductInfo.nDstWidth = ENCODE_SRC_WIDTH;
    ProductInfo.nDstHeight = ENCODE_SRC_HEIGHT;
    VencSetParameter(pVideoEnc, VENC_IndexParamProductCase, (void*)&ProductInfo);

    unsigned int vbv_size = ENCODE_VBV_SIZE;
    VencSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);

    VencSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264_func->h264Param);
    VencSetParameter(pVideoEnc, VENC_IndexParamProcSet, &h264_func->sVeProcInfo);

    VencH264VideoSignal sVideoSignal;
    memset(&sVideoSignal, 0, sizeof(VencH264VideoSignal));
    sVideoSignal.video_format = DEFAULT;
    sVideoSignal.full_range_flag = 0;
    sVideoSignal.src_colour_primaries = VENC_BT709;
    sVideoSignal.dst_colour_primaries = sVideoSignal.src_colour_primaries;
    VencSetParameter(pVideoEnc, VENC_IndexParamVUIVideoSignal, &sVideoSignal);

    return 0;
}

static void save_stream(h264_func_t *h264_func, VencOutputBuffer *outputBuffer)
{
    if (!h264_func->fp) {
        h264_func->fp = fopen("/tmp/vetester.raw", "wb");
        if (!h264_func->fp) {
            printf("open /tmp/vetester.raw fail!\n");
            return;
        }
    }

    if (outputBuffer->nFlag & VENC_BUFFERFLAG_KEYFRAME) {
        VencHeaderData header_data;
        VencGetParameter(h264_func->pVideoEnc, VENC_IndexParamH264SPSPPS, (void*)&header_data);
        if (header_data.nLength && header_data.pBuffer)
            fwrite(header_data.pBuffer, header_data.nLength, 1, h264_func->fp);
    }
    if (outputBuffer->nSize0 && outputBuffer->pData0)
        fwrite(outputBuffer->pData0, outputBuffer->nSize0, 1, h264_func->fp);
    if (outputBuffer->nSize1 && outputBuffer->pData1)
        fwrite(outputBuffer->pData1, outputBuffer->nSize1, 1, h264_func->fp);
    if (outputBuffer->nSize2 && outputBuffer->pData2)
        fwrite(outputBuffer->pData2, outputBuffer->nSize2, 1, h264_func->fp);
}

int main(int argc, char *argv[])
{
    int result;
    h264_func_t h264_func;
    VideoEncoder* pVideoEnc;
    VencBaseConfig baseConfig;

    memset(&baseConfig, 0, sizeof(baseConfig));
    baseConfig.memops = MemAdapterGetOpsS();
    if (!baseConfig.memops) {
        printf("MemAdapterGetOpsS failed\n");
        return -1;
    }

    CdcMemOpen(baseConfig.memops);

    memset(&h264_func, 0, sizeof(h264_func));
    pVideoEnc = VencCreate(VENC_CODEC_H264);
    if (!pVideoEnc) {
        printf("VencCreate failed\n");
        return -1;
    }
    h264_func.pVideoEnc = pVideoEnc;

    setEncParam(pVideoEnc, &h264_func);

    baseConfig.nInputWidth = ENCODE_SRC_WIDTH;
    baseConfig.nInputHeight = ENCODE_SRC_HEIGHT;
    baseConfig.nStride = ENCODE_SRC_WIDTH;
    baseConfig.nDstWidth = ENCODE_SRC_WIDTH;
    baseConfig.nDstHeight = ENCODE_SRC_HEIGHT;
    baseConfig.bEncH264Nalu = 0;
    baseConfig.nChannel = 0;
    baseConfig.eInputFormat = VENC_PIXEL_YVU420SP;
    baseConfig.bOnlineMode = 1;
    baseConfig.bOnlineChannel = 0;
    baseConfig.bVcuOn = 1;
    baseConfig.bVcuAutoMode = 1;
    result = VencInit(pVideoEnc, &baseConfig);
    if (result != 0) {
        printf("VencInit failed\n");
        return result;
    }

    VencCbType vencCallBack;
    vencCallBack.EventHandler = EventHandler;
    vencCallBack.InputBufferDone = InputBufferDone;
    VencSetCallbacks(pVideoEnc, &vencCallBack, &h264_func);

    result = VencStart(pVideoEnc);
    if (result) {
        printf("VencStart failed\n");
        return result;
    }

    VencInputBuffer mInBuffer;
    memset(&mInBuffer, 0, sizeof(mInBuffer));
    mInBuffer.nWidth = ENCODE_SRC_WIDTH;
    mInBuffer.nHeight = ENCODE_SRC_HEIGHT;
    unsigned int nInBufferSize = mInBuffer.nWidth * mInBuffer.nHeight * 3 / 2;
    mInBuffer.pAddrVirY = CdcMemPalloc(baseConfig.memops, nInBufferSize, NULL, NULL);
    if(!mInBuffer.pAddrVirY)
    {
        printf("palloc failed, size = %u\n", nInBufferSize);
        return -1;
    }
    mInBuffer.pAddrVirC = mInBuffer.pAddrVirY + mInBuffer.nWidth * mInBuffer.nHeight;
    mInBuffer.pAddrPhyY = CdcMemGetPhysicAddress(baseConfig.memops, mInBuffer.pAddrVirY);
    mInBuffer.pAddrPhyC = mInBuffer.pAddrPhyY + mInBuffer.nWidth * mInBuffer.nHeight;
    printf("inbuffer:width=%d, height=%d, Y=%p, C0=%p\n",
         mInBuffer.nWidth, mInBuffer.nHeight,
         mInBuffer.pAddrPhyY, mInBuffer.pAddrPhyC);

    //300 frames, 10 seconds
    int try_cnt;
    int pic_change = 0;
    VencOutputBuffer outputBuffer;
    for (int i = 0; i < 100; i++) {
        if (i % 5 == 0) {
            draw_nv21_buffer(ENCODE_SRC_WIDTH, ENCODE_SRC_HEIGHT,
                    mInBuffer.pAddrVirY, mInBuffer.pAddrVirY+mInBuffer.nWidth*mInBuffer.nHeight, (pic_change%4));
            pic_change++;
        }
        CdcMemFlushCache(baseConfig.memops, mInBuffer.pAddrVirY, (int)nInBufferSize, MEM_SYNC_CPU_TO_DEV);
        mInBuffer.nPts += 33333;
        result = VencQueueInputBuf(pVideoEnc, &mInBuffer);
        if (result) {
            printf("VencQueueInputBuf failed\n");
            continue;
        }

        try_cnt = 0;
        while (1) {
            if (try_cnt >= 10) {
                printf("VencDequeueOutputBuf failed\n");
                return -1;
            }

            result = VencDequeueOutputBuf(pVideoEnc, &outputBuffer);
            if (result == 0) {
                save_stream(&h264_func, &outputBuffer);
                break;
            }
            try_cnt++;
            usleep(10*1000);
        }
    }

    if (h264_func.fp)
        fclose(h264_func.fp);

    VencDestroy(pVideoEnc);

    CdcMemPfree(baseConfig.memops, mInBuffer.pAddrVirY, NULL, NULL);
    CdcMemClose(baseConfig.memops);
}
