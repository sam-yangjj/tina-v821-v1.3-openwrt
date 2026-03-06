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
#ifndef DRAW_RESULT_H
#define DRAW_RESULT_H

#include "../postprocess/post_process.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char *font_bitmap;
    int font_bitmap_w;
    int font_bitmap_h;
    int font_loaded;
    void *cdata; // stbtt_bakedchar[96]
} FontContext;

// 初始化字体上下文
int load_font(FontContext *ctx, const char *font_path, float font_size);

// 释放字体资源
void free_font(FontContext *ctx);

// 在图像上绘制文字
void draw_text(FontContext *ctx, unsigned char *img, int img_w, int img_h,
               int x, int y, const char *text,
               unsigned char r, unsigned char g, unsigned char b);

// 画矩形框
void draw_rect(unsigned char *img, int w, int h, int x, int y, int rw, int rh,
               unsigned char r, unsigned char g, unsigned char b);

// 主绘制函数
int draw_results_on_image(const char *image_path, const char *output_path,
                          const Object *objs, int num_objs,
                          FontContext *ctx);

int draw_results_on_memory(unsigned char *img, int w, int h, int ch,
                           const Object *objs, int num_objs, FontContext *ctx);


void save_to_png(const unsigned char *img, int w, int h, int ch, const char *output_path);

#ifdef __cplusplus
}
#endif

#endif // DRAW_RESULT_H
