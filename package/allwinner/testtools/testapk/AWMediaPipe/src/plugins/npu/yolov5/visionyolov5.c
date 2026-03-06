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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "awnn_lib.h"
#include "visionyolov5.h"
#include "../../utils/log.h"


#define MAX_DETECTIONS 100
#define SRC_W 640                    /* 原始传感器分辨率 */
#define SRC_H 480

/************  预处理：RGB888 → 640×640 NCHW Uint8  ************/
static unsigned char *preprocess_rgb888(unsigned char *rgb_data,
                                        unsigned int src_w,
                                        unsigned int src_h,
                                        unsigned int dst_w,
                                        unsigned int dst_h,
                                        unsigned int *out_sz)
{
    size_t src_size = src_w * src_h * 3;
    /* 输出张量内存：NCHW, uint8 */
    size_t dst_size = dst_w * dst_h * 3;
    unsigned char *dst = (unsigned char *)malloc(dst_size);
    if (!dst) { return NULL; }
    memset(dst, 255, dst_size);      /* Letterbox 灰底 */

    /* 计算 letterbox 缩放及填充 */
    float scale = (float)dst_w / src_w < (float)dst_h / src_h ?
                  (float)dst_w / src_w : (float)dst_h / src_h;
    int resize_w = (int)(src_w * scale);
    int resize_h = (int)(src_h * scale);
    int pad_x = (dst_w - resize_w) / 2;
    int pad_y = (dst_h - resize_h) / 2;

    /* 最近邻缩放到目标区域并写入 NCHW */
    for (int y = 0; y < resize_h; y++) {
        for (int x = 0; x < resize_w; x++) {
            int src_x = (int)(x / scale);
            int src_y = (int)(y / scale);
            const unsigned char *p_src =
                rgb_data + (src_y * src_w + src_x) * 3;

            int dst_x = x + pad_x;
            int dst_y = y + pad_y;

            size_t idx_r = 0 * dst_w * dst_h + dst_y * dst_w + dst_x;
            size_t idx_g = 1 * dst_w * dst_h + dst_y * dst_w + dst_x;
            size_t idx_b = 2 * dst_w * dst_h + dst_y * dst_w + dst_x;

            dst[idx_r] = p_src[0];
            dst[idx_g] = p_src[1];
            dst[idx_b] = p_src[2];
        }
    }

    //free(src);
    *out_sz = (unsigned int)dst_size;
    return dst;
}

unsigned char* rgba8888_to_nchw_resize_pad(
    const unsigned char* rgba_data,  // RGBA8888 输入
    int src_w, int src_h,            // 输入宽高
    int dst_w, int dst_h,            // 输出宽高（带padding）
    unsigned int* out_sz             // 返回大小（可选）
) {
    float scale = fminf((float)dst_w / src_w, (float)dst_h / src_h);
    int resize_w = (int)(src_w * scale);
    int resize_h = (int)(src_h * scale);
    int pad_x = (dst_w - resize_w) / 2;
    int pad_y = (dst_h - resize_h) / 2;

    size_t dst_size = 3 * dst_w * dst_h;
    unsigned char* dst = (unsigned char*)malloc(dst_size);
    if (!dst) return NULL;

    // 清空整个目标缓冲区（可选：背景为0）
    memset(dst, 0, dst_size);

    // 最近邻缩放到目标区域并写入 NCHW（RGB）
    for (int y = 0; y < resize_h; y++) {
        for (int x = 0; x < resize_w; x++) {
            int src_x = (int)(x / scale);
            int src_y = (int)(y / scale);
            const unsigned char *p_src =
                rgba_data + (src_y * src_w + src_x) * 4;  // RGBA8888 每像素4字节

            int dst_x = x + pad_x;
            int dst_y = y + pad_y;

            size_t idx_r = 0 * dst_w * dst_h + dst_y * dst_w + dst_x;
            size_t idx_g = 1 * dst_w * dst_h + dst_y * dst_w + dst_x;
            size_t idx_b = 2 * dst_w * dst_h + dst_y * dst_w + dst_x;

            dst[idx_r] = p_src[0]; // R
            dst[idx_g] = p_src[1]; // G
            dst[idx_b] = p_src[2]; // B
            // 忽略 p_src[3]（Alpha 通道）
        }
    }

    *out_sz = (unsigned int)dst_size;
    return dst;
}


/************  成员函数实现 ************/
static int yolov5_init_impl(YoloV5Net *self)
{
    self->context = awnn_create(self->nbg_path);
    if (!self->context) {
        fprintf(stderr, "awnn_create failed\n");
        return -1;
    }

    self->fontCtx = (FontContext*) calloc(1,sizeof(FontContext));
    if (!load_font(self->fontCtx, "DejaVuSans.ttf", 16.0f)) {
        fprintf(stderr, "Failed to load font\n");
        return -1;
    }

    return 0;
}

// RGB24 转 YUV420P，输入 rgb，输出 yuv，均为平面格式
static void rgb24_to_yuv420p(const uint8_t* rgb, uint8_t* yuv, int width, int height) {
    int frameSize = width * height;
    uint8_t* yPlane = yuv;
    uint8_t* uPlane = yuv + frameSize;
    uint8_t* vPlane = yuv + frameSize + frameSize / 4;
    int r, g, b, y, u, v;
    int i, j;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            int rgb_idx = (j * width + i) * 3;
            r = rgb[rgb_idx + 0];
            g = rgb[rgb_idx + 1];
            b = rgb[rgb_idx + 2];
            y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            yPlane[j * width + i] = (uint8_t)(y < 0 ? 0 : (y > 255 ? 255 : y));
            if ((j % 2 == 0) && (i % 2 == 0)) {
                int uv_idx = (j / 2) * (width / 2) + (i / 2);
                uPlane[uv_idx] = (uint8_t)(u < 0 ? 0 : (u > 255 ? 255 : u));
                vPlane[uv_idx] = (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
            }
        }
    }
}

/**
 * @brief 将 RGB24 格式图像转换为 RGBA 格式
 * @param rgb 输入的 RGB24 数据
 * @param rgba 输出的 RGBA 数据
 * @param width 图像宽度
 * @param height 图像高度
 */
void rgb24_to_rgba(const uint8_t* rgb, uint8_t* rgba, int width, int height) {
    int in_index = 0;
    int out_index = 0;
    int total_pixels = width * height;

    for (int i = 0; i < total_pixels; i++) {
        // Copy R, G, B
        rgba[out_index++] = rgb[in_index++]; // R
        rgba[out_index++] = rgb[in_index++]; // G
        rgba[out_index++] = rgb[in_index++]; // B
        rgba[out_index++] = 0xFF;            // A (fully opaque)
    }
}
char file_path[128];
static int yolov5_run_impl(YoloV5Net *self, unsigned char *rgb24_data, int src_w, int src_h, Object* objs)
{
    /* === 预处理 === */
    unsigned int tensor_size = 0;
    unsigned char *tensor =
        preprocess_rgb888(rgb24_data, src_w, src_h, self->input_w, self->input_h, &tensor_size);
    if (!tensor) return -1;

    void *inputs[] = {tensor};
    awnn_set_input_buffers(self->context, inputs);
    awnn_run(self->context);
    float **outputs = awnn_get_output_buffers(self->context);


    int num = yolov5_post_process_c(/*src_w*/src_w, /*src_h*/src_h,
                                    outputs, objs, MAX_DETECTIONS);


    for (int i = 0; i < num; ++i) {
        printf("%2d: %-15s %.1f%% "
               "[x=%.1f y=%.1f w=%.1f h=%.1f]\n",
               i,
               objs[i].label_text,
               objs[i].prob * 100.f,
               objs[i].x, objs[i].y,
               objs[i].width, objs[i].height);
    }
    free(tensor);
    return num;
}

static void yolov5_destroy_impl(YoloV5Net *self)
{
    if(self->fontCtx){
        free_font(self->fontCtx);
        free(self->fontCtx);
        self->fontCtx = NULL;
    }

    if (self->context){
        awnn_destroy(self->context);
        self->context = NULL;
    }
}

/************  “构造函数” ************/
void yolov5_create(YoloV5Net *net, const char *nbg_path)
{
    net->nbg_path = nbg_path;
    net->input_w  = 640;
    net->input_h  = 640;
    net->input_c  = 3;
    net->context  = NULL;

    net->init    = yolov5_init_impl;
    net->run     = yolov5_run_impl;
    net->destroy = yolov5_destroy_impl;
}
