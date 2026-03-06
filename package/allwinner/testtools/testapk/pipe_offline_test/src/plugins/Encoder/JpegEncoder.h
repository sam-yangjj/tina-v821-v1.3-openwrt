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
#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
// #include "buffer.h"
#include "vencoder.h"
#include "ion_mem_alloc.h"
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

// 应用配置结构
typedef struct {
    int src_width;
    int src_height;
    int fps;
    VENC_PIXEL_FMT input_format;
} encoderConfig;

// H264编码器上下文
typedef struct JPEGEncoder {
    int width;
    int height;
    int fps;
    int bitrate;
    pthread_mutex_t mutex;
    VENC_PIXEL_FMT input_format;

    VideoEncoder* pVideoEnc;
    VencBaseConfig* baseConfig;
    VencAllocateBufferParam* bufferParam;
    EXIFInfo exifinfo;

    VencOutputBuffer* outputBuffer;
    int64_t frame_id;
} JPEGEncoder;

// 初始化/销毁 - 移除H264FrameBuffer参数
bool jpeg_encoder_init(JPEGEncoder* enc, encoderConfig* config);
void jpeg_encoder_destroy(JPEGEncoder* enc);

// 编码一帧，输入为DMA buffer，输出为h264数据和长度
// 返回true表示编码成功，false表示失败
bool jpeg_encoder_encode(JPEGEncoder* enc, VencInputBuffer* inBuff, VencOutputBuffer* outputBuf, size_t* out_len);

#endif // JPEG_ENCODER_H
