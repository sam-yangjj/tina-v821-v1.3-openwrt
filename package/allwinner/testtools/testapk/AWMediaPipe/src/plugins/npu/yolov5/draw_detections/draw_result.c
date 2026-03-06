#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "draw_result.h"

// 初始化字体上下文
int load_font(FontContext *ctx, const char *font_path, float font_size) {
    FILE *fp = fopen(font_path, "rb");
    if (!fp) {
        fprintf(stderr, "无法打开字体文件 %s\n", font_path);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char *ttf_buffer = malloc(size);
    fread(ttf_buffer, 1, size, fp);
    fclose(fp);

    ctx->font_bitmap_w = 512;
    ctx->font_bitmap_h = 512;
    ctx->font_bitmap = calloc(1, ctx->font_bitmap_w * ctx->font_bitmap_h);
    ctx->cdata = malloc(sizeof(stbtt_bakedchar) * 96);
    stbtt_BakeFontBitmap(ttf_buffer, 0, font_size, ctx->font_bitmap,
                         ctx->font_bitmap_w, ctx->font_bitmap_h, 32, 96, ctx->cdata);
    free(ttf_buffer);
    ttf_buffer = NULL;
    ctx->font_loaded = 1;
    return 1;
}

// 释放字体资源
void free_font(FontContext *ctx) {
    if (ctx->font_bitmap){
        free(ctx->font_bitmap);
        ctx->font_bitmap = NULL;
    }
    if (ctx->cdata){
        free(ctx->cdata);
        ctx->cdata = NULL;
    }
}

// 绘制文字
void draw_text(FontContext *ctx, unsigned char *img, int img_w, int img_h,
               int x, int y, const char *text,
               unsigned char r, unsigned char g, unsigned char b) {
    if (!ctx->font_loaded) return;

    stbtt_bakedchar *cdata = (stbtt_bakedchar *)ctx->cdata;

    while (*text) {
        if (*text < 32 || *text >= 128) {
            text++;
            continue;
        }

        stbtt_bakedchar *bch = &cdata[*text - 32];
        int w = bch->x1 - bch->x0;
        int h = bch->y1 - bch->y0;

        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                int fx = x + bch->xoff + j;
                int fy = y + bch->yoff + i;
                if (fx >= 0 && fx < img_w && fy >= 0 && fy < img_h) {
                    unsigned char alpha = ctx->font_bitmap[(bch->y0 + i) * ctx->font_bitmap_w + (bch->x0 + j)];
                    int idx = (fy * img_w + fx) * 3;
                    img[idx + 0] = (r * alpha + img[idx + 0] * (255 - alpha)) / 255;
                    img[idx + 1] = (g * alpha + img[idx + 1] * (255 - alpha)) / 255;
                    img[idx + 2] = (b * alpha + img[idx + 2] * (255 - alpha)) / 255;
                }
            }
        }
        x += bch->xadvance;
        text++;
    }
}

// 画矩形框
void draw_rect(unsigned char *img, int w, int h, int x, int y, int rw, int rh,
               unsigned char r, unsigned char g, unsigned char b) {
    for (int i = x; i < x + rw; ++i) {
        if (y >= 0 && y < h && i >= 0 && i < w) {
            int idx = (y * w + i) * 3;
            img[idx] = r; img[idx + 1] = g; img[idx + 2] = b;
        }
        if ((y + rh) >= 0 && (y + rh) < h && i >= 0 && i < w) {
            int idx = ((y + rh) * w + i) * 3;
            img[idx] = r; img[idx + 1] = g; img[idx + 2] = b;
        }
    }
    for (int j = y; j < y + rh; ++j) {
        if (x >= 0 && x < w && j >= 0 && j < h) {
            int idx = (j * w + x) * 3;
            img[idx] = r; img[idx + 1] = g; img[idx + 2] = b;
        }
        if ((x + rw) >= 0 && (x + rw) < w && j >= 0 && j < h) {
            int idx = (j * w + x + rw) * 3;
            img[idx] = r; img[idx + 1] = g; img[idx + 2] = b;
        }
    }
}

void save_to_png(const unsigned char *img, int w, int h, int ch, const char *output_path) {
    if (!img || w <= 0 || h <= 0 || ch <= 0 || !output_path) {
        fprintf(stderr, "无效的输入参数\n");
        return;
    }

    // 使用stbi_write_png函数将图像数据写入PNG文件
    int success = stbi_write_png(output_path, w, h, ch, img, w * ch);
    if (!success) {
        fprintf(stderr, "保存为PNG文件失败: %s\n", output_path);
    } else {
        printf("成功保存为PNG文件: %s\n", output_path);
    }
}



// 主绘制函数
int draw_results_on_image(const char *image_path, const char *output_path,
                          const Object *objs, int num_objs, FontContext *ctx) {
    int w, h, ch;
    unsigned char *img = stbi_load(image_path, &w, &h, &ch, 3);
    if (!img) {
        fprintf(stderr, "failed to load image: %s\n", image_path);
        return 0;
    }

    for (int i = 0; i < num_objs; ++i) {
        const Object *obj = &objs[i];
        int x = (int)obj->x;
        int y = (int)obj->y;
        int rw = (int)obj->width;
        int rh = (int)obj->height;

        draw_rect(img, w, h, x, y, rw, rh, 255, 0, 0); // red box

        char label[128];
        snprintf(label, sizeof(label), "%s %.1f%%", obj->label_text, obj->prob);
        draw_text(ctx, img, w, h, x + 2, y - 12, label, 0, 255, 0); // green text
    }

    int success = stbi_write_png(output_path, w, h, 3, img, w * 3);
    stbi_image_free(img);
    if (!success) {
        fprintf(stderr, "save failed: %s\n", output_path);
        return 0;
    }

    return 1;
}

int phnum = 0;

int draw_results_on_memory(unsigned char *img, int w, int h, int ch,
                           const Object *objs, int num_objs, FontContext *ctx) {
    if (!img || w <= 0 || h <= 0 || ch <= 0) {
        fprintf(stderr, "incorrect image parameter\n");
        return 0;
    }

    for (int i = 0; i < num_objs; ++i) {
        const Object *obj = &objs[i];
        int x = (int)obj->x;
        int y = (int)obj->y;
        int rw = (int)obj->width;
        int rh = (int)obj->height;

        // 在内存中的图像数据上绘制矩形框
        draw_rect(img, w, h, x, y, rw, rh, 255, 0, 0); // 红色框

        // 格式化标签文本
        char label[128];
        snprintf(label, sizeof(label), "%s %.1f%%", obj->label_text, obj->prob * 100);
        // 在内存中的图像数据上绘制文本
        draw_text(ctx, img, w, h, x + 2, y - 12, label, 0, 255, 0); // 绿色文本
    }

    #if 0
    char png_path[128];
    sprintf(png_path, "/tmp/img/result%d.png", phnum++%10);
    save_to_png(img,w,h,ch,png_path);
    #endif
    return 1;
}
