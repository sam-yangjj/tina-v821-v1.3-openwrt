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
#include "g2d_plugin.h"
#include "g2d_driver_enh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../buffer_pool.h"
#include "../../utils/log.h"
#include "../frame_queue.h"
#include "../../utils/frame_check.h"

const char* pixel_format_to_string(PixelFormat fmt) {
    switch (fmt) {
        case PIXEL_FORMAT_NV12: return "NV12";
        case PIXEL_FORMAT_RGB888: return "RGB888";
        case PIXEL_FORMAT_ARGB8888: return "ARGB8888";
        case PIXEL_FORMAT_YUV420P: return "YUV420";
        default: return "Unknown";
    }
}

typedef struct {
    int g2d_fd;
    struct SunxiMemOpsS* mem_ops;

    pthread_t thread;
    int running;

    OutputBufferPool output_pool;
    FrameQueue queue_out;

    // 用户配置的输出参数
    int outputFormatFlag;
    PixelFormat output_format;

    int output_width;
    int output_height;

    // 裁剪配置
    int crop_x;
    int crop_y;
    int crop_width;
    int crop_height;
    int crop_enabled;

    // 旋转配置
    int rotation_enabled;
    int rotation_angle;  // 0 / 90 / 180 / 270

} G2DContext;

static int pixel_format_to_g2d(PixelFormat fmt) {
    switch (fmt) {
        case PIXEL_FORMAT_YUV420P: return G2D_FORMAT_YUV420_PLANAR;
        case PIXEL_FORMAT_YV12: return G2D_FORMAT_YUV420_PLANAR;
        case PIXEL_FORMAT_RGB565: return G2D_FORMAT_RGB565;
        case PIXEL_FORMAT_ARGB8888: return G2D_FORMAT_ARGB8888;
        case PIXEL_FORMAT_RGB888: return G2D_FORMAT_RGB888;
        case PIXEL_FORMAT_NV21: return G2D_FORMAT_YUV420UVC_V1U1V0U0;
        case PIXEL_FORMAT_NV12: return G2D_FORMAT_YUV420UVC_U1V1U0V0;
        default: return -1;
    }
}
static void update_fps_g2d() {
    static int frame_count = 0;
    static time_t last_time = 0;

    frame_count++;

    time_t current_time = time(NULL);
    if (current_time != last_time) {
        LOG_INFO("FPS: %d\n", frame_count);
        frame_count = 0;
        last_time = current_time;
    }
}

static void g2d_output_release(struct FrameBuffer* frame, void* user_data)
{
    G2DContext* ctx = (G2DContext*)user_data;

    output_pool_release(&ctx->output_pool,frame);
}

static int save_data(const unsigned char *rgb, size_t size, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERROR("fail to open %s",filename);
        return -1;
    }

    size_t written = fwrite(rgb, 1, size, fp);
    fclose(fp);

    if (written != size) {
        LOG_ERROR("write error, expect to write %zu bytes, actual write %zu bytes", size, written);
        return -1;
    }

    return 0;
}


static void* g2d_thread_loop(void* arg) {
    Plugin* self = (Plugin*)arg;
    G2DContext* ctx = (G2DContext*)self->priv;

    static int index1 = 0;
    uint8_t* ref_frame = NULL;
    // if(ctx->output_format == PIXEL_FORMAT_UNKNOWN)
    //     ctx->output_format = PIXEL_FORMAT_YUV420P;
    // output_pool_init(&ctx->output_pool, MAX_OUTPUT_BUFFERS, ctx->output_width, ctx->output_height, ctx->output_format, ctx->mem_ops);

    while (ctx->running) {
        if (!self->upstream || !self->upstream->pull) continue;

        FrameBuffer* in = self->upstream->pull(self->upstream);
        if (!in)
        {
            LOG_WARN("%s: get frame timeout, try again", self->name);
            usleep(10*1000);
            continue;
        }
        frame_get(in);
        if(!in->data || in->fd < 0)
        {
            frame_put(in);
            continue;
        }

        if (in->width <= 0 || in->height <= 0 || in->format < 0) {
            LOG_ERROR("Invalid input frame dimensions or format");
            frame_put(in);
            continue;
        }
        static int index = 0;
        //LOG_INFO("%s: G2D get frame: %dx%d, fd=%d, index=%d", self->name, in->width, in->height, in->fd, index);

        // in->width = ALIGN_16B(in->width);
        // in->height = ALIGN_16B(in->height);

        if (ctx->rotation_enabled == 1) {

            if(ctx->rotation_angle == 90 || ctx->rotation_angle == 270)
            {
                ctx->output_width = in->height;
                ctx->output_height = in->width;
            }else {
                ctx->output_width = in->width;
                ctx->output_height = in->height;
            }
        }
        if(!ctx->outputFormatFlag)
            ctx->output_format = in->format;

        if(!ctx->crop_enabled)
        {
            ctx->crop_x = 0;
            ctx->crop_y = 0;
        }

        int out_w = ctx->output_width > 0 ? ctx->output_width : in->width;
        int out_h = ctx->output_height > 0 ? ctx->output_height : in->height;

        out_h = ALIGN_16B(out_h);
        out_w = ALIGN_16B(out_w);
        if (!ctx->output_pool.initialized) {
            output_pool_init(&ctx->output_pool, MAX_OUTPUT_BUFFERS, out_w, out_h, ctx->output_format, ctx->mem_ops);
        }

        int g2d_fmt = pixel_format_to_g2d(ctx->output_format);

        if (g2d_fmt < 0) {
            LOG_ERROR("[g2d] Unsupported output format\n");
            continue;
        }

        FrameBuffer* out = output_pool_get(&ctx->output_pool);
        if (!out) {
            LOG_WARN("%s: No available output buffer", self->name);
            frame_put(in);
            usleep(10 * 1000); // 等待空闲
            continue;
        }
        out->release = g2d_output_release;
        out->user_data = ctx;
        //LOG_INFO("%s: get buffer from pool success", self->name);
        g2d_blt_h blit;
        memset(&blit, 0, sizeof(blit));
        if(ctx->rotation_enabled == 1)
        {
            if(ctx->rotation_angle == 90)
                blit.flag_h = G2D_ROT_90;
            else if(ctx->rotation_angle == 180)
                blit.flag_h = G2D_ROT_180;
            else if(ctx->rotation_angle == 270)
                blit.flag_h = G2D_ROT_270;
            else if(ctx->rotation_angle == 0)
                blit.flag_h = G2D_ROT_0;
        }else
            blit.flag_h = G2D_BLT_NONE_H;

        blit.src_image_h.format = pixel_format_to_g2d(in->format);
        blit.src_image_h.width  = in->width;
        blit.src_image_h.height = in->height;
        blit.src_image_h.clip_rect.x = 0;
        blit.src_image_h.clip_rect.y = 0;
        blit.src_image_h.clip_rect.w = in->width;
        blit.src_image_h.clip_rect.h = in->height;
        blit.src_image_h.align[0] = 0;
        blit.src_image_h.align[1] = 0;
        blit.src_image_h.align[2] = 0;
        blit.src_image_h.laddr[0] = 0;
        blit.src_image_h.laddr[1] = 0; // start of U vector
        blit.src_image_h.laddr[2] = 0; // start of V vector
        blit.src_image_h.use_phy_addr = 0;
        blit.src_image_h.fd = in->fd;
        if(ctx->rotation_enabled == 0)
        {
            blit.src_image_h.bbuff = 1;
            blit.src_image_h.alpha = 0xff;
            blit.src_image_h.mode = G2D_PIXEL_ALPHA;
        }

        blit.dst_image_h.format = g2d_fmt;
        blit.dst_image_h.width  = out_w;
        blit.dst_image_h.height = out_h;
        blit.dst_image_h.clip_rect.x = ctx->crop_x;
        blit.dst_image_h.clip_rect.y = ctx->crop_y;
        blit.dst_image_h.clip_rect.w = out_w;
        blit.dst_image_h.clip_rect.h = out_h;
        blit.dst_image_h.fd = out->fd;
        blit.dst_image_h.align[0] = 0;
        blit.dst_image_h.align[1] = 0;
        blit.dst_image_h.align[2] = 0;
        blit.dst_image_h.laddr[0] = 0;
        blit.dst_image_h.laddr[1] = 0;
        blit.dst_image_h.laddr[2] = 0;
        blit.dst_image_h.use_phy_addr = 0;
        if(ctx->rotation_enabled == 0)
        {
            blit.dst_image_h.bbuff = 1;
            blit.dst_image_h.mode = G2D_PIXEL_ALPHA;
            blit.dst_image_h.alpha = 0xff;
        }

        if (ioctl(ctx->g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)&blit) < 0) {
            LOG_ERROR("ioctl G2D_CMD_BITBLT_H failed");
            g2d_output_release(out,ctx);
            continue;
        }

        // out->ref_count = 0;
        // out->memOps = ctx->mem_ops;
        //frame_queue_push_force(&ctx->queue_out, out);
        out->buf_index = index++;
        out->pts = in->pts;

        if(index1 == 1){
            ref_frame = init_checker(out->size);
        }

        if(check_frame(out->data, ref_frame, out->size, index1++)){
            printf("[ERROR]第 %d 帧与基准帧不一致, %s 输出异常\n", index1, self->name);
        }else{
            printf("%s: 第 %d 帧与基准帧一致\n", self->name, index1);
        }


        if(frame_queue_push(&ctx->queue_out, out, &ctx->running))
        //if(frame_queue_push_timeout(&ctx->queue_out, out, 20))
        {
            g2d_output_release(out,ctx);
        }
        //update_fps_g2d();
        frame_put(in);

    }

    release_checker(ref_frame);

    return NULL;
}

static int g2d_start(Plugin* self) {
    G2DContext* ctx = (G2DContext*)self->priv;

    ctx->mem_ops = GetMemAdapterOpsS();
    SunxiMemOpen(ctx->mem_ops);
    ctx->g2d_fd = open("/dev/g2d", O_RDWR);
    if (ctx->g2d_fd < 0) {
        LOG_ERROR("open /dev/g2d");
        return -1;
    }

    frame_queue_init(&ctx->queue_out);

    ctx->running = 1;
    return pthread_create(&ctx->thread, NULL, g2d_thread_loop, self);
}

static int g2d_stop(Plugin* self) {
    G2DContext* ctx = (G2DContext*)self->priv;

    // 只停止线程，不释放资源
    ctx->running = 0;
    pthread_cond_broadcast(&ctx->queue_out.cond_nonempty);
    pthread_cond_broadcast(&ctx->queue_out.cond_nonfull);
    pthread_join(ctx->thread, NULL);

    return 0;
}

static FrameBuffer* g2d_pull(Plugin* self) {
    G2DContext* ctx = (G2DContext*)self->priv;
    return frame_queue_pop(&ctx->queue_out);
}

static int g2d_push(Plugin* self, FrameBuffer* fb) {
    if (self->downstream && self->downstream->push) {
        return self->downstream->push(self->downstream, fb);
    }
    return -1;
}

Plugin* create_g2d_plugin(const char* name) {
    Plugin* p = (Plugin*)calloc(1, sizeof(Plugin));
    G2DContext* ctx = (G2DContext*)calloc(1, sizeof(G2DContext));

    ctx->output_format = PIXEL_FORMAT_UNKNOWN; // 默认格式
    ctx->output_width = 0;  // 跟随输入尺寸
    ctx->output_height = 0;
    ctx->crop_enabled = 0;
    ctx->rotation_enabled = 0;
    ctx->rotation_angle = 0;
    ctx->outputFormatFlag = 0;

    printf("g2d_create start\n");
    p->name = name ? strdup(name) : strdup("G2DPlugin");
    p->start = g2d_start;
    p->stop = g2d_stop;
    p->pull = g2d_pull;
    p->push = g2d_push;
    p->priv = ctx;
    printf("g2d_create end\n");
    return p;
}

int g2d_set_output_format(Plugin* self, PixelFormat fmt) {
    if (!self || !self->priv) return -1;
    G2DContext* ctx = (G2DContext*)self->priv;
    // 如果用户已经设置了旋转，则不能再设置缩放
    if (ctx->rotation_enabled) {
        fprintf(stderr, "[G2D] Error: Format change cannot be combined with rotation.\n");
        return -1;
    }
    ctx->outputFormatFlag = 1;
    ctx->output_format = fmt;
    return 0;
}

int g2d_set_output_size(Plugin* self, int width, int height) {
    if (!self || !self->priv) return -1;
    G2DContext* ctx = (G2DContext*)self->priv;

    // 如果用户已经设置了旋转，则不能再设置缩放
    if (ctx->rotation_enabled) {
        fprintf(stderr, "[G2D] Error: Scaling cannot be combined with rotation.\n");
        return -1;
    }
    ctx->output_width = width;
    ctx->output_height = height;

    ctx->output_width = ALIGN_16B(ctx->output_width);
    ctx->output_height = ALIGN_16B(ctx->output_height);
    return 0;
}

int g2d_set_crop_region(Plugin* self, int crop_x, int crop_y, int crop_width, int crop_height) {
    if (!self || !self->priv) return -1;
    G2DContext* ctx = (G2DContext*)self->priv;
    ctx->crop_x = crop_x;
    ctx->crop_y = crop_y;
    ctx->crop_width = crop_width;
    ctx->crop_height = crop_height;
    ctx->crop_enabled = 1;
    return 0;
}

int g2d_set_rotation(Plugin* self, int angle) {
    if (angle != 0 && angle != 90 && angle != 180 && angle != 270) {
        fprintf(stderr, "Unsupported rotation angle: %d\n", angle);
        return -1;
    }
    if (!self || !self->priv) return -1;
    G2DContext* ctx = (G2DContext*)self->priv;

    // 如果用户已经设置了缩放或格式转换，则不能设置旋转
    if ((ctx->output_width > 0 && ctx->output_height > 0)) {
        fprintf(stderr, "[G2D] Error: Rotation cannot be combined with scaling or format conversion.\n");
        return -1;
    }
    ctx->rotation_enabled = 1;
    ctx->rotation_angle = angle;
    return 0;
}

static void g2d_plugin_destroy(Plugin* self) {
    G2DContext* ctx = (G2DContext*)self->priv;

    if (ctx) {
        // 确保线程已停止
        if (ctx->running) {
            self->stop(self);
        }

        // 销毁frame队列
        frame_queue_destroy(&ctx->queue_out);


        output_pool_destroy(&ctx->output_pool);


        // 关闭设备和释放内存操作
        if (ctx->g2d_fd >= 0) {
            close(ctx->g2d_fd);
            ctx->g2d_fd = -1;
        }
        if (ctx->mem_ops) {
            SunxiMemClose(ctx->mem_ops);
            ctx->mem_ops = NULL;
        }

        free(ctx);
        ctx = NULL;
    }
    self->priv = NULL;
    LOG_INFO("g2d_plugin_destroy\n");

}

void destroy_g2d_plugin(Plugin* self)
{
    if (self) {
        if (self->priv) {
            g2d_plugin_destroy(self);
        }
        if (self->name) {
            free(self->name);
            self->name = NULL;
        }

        free(self);
    }
}

