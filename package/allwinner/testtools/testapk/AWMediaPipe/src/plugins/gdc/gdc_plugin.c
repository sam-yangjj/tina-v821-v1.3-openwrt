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
#include "gdc_plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "vencoder.h"
#include "../utils/log.h"
#include "../frame_queue.h"
#include "../buffer_pool.h"

typedef struct {
    int input_width;               /**< Input image width */
    int input_height;              /**< Input image height */

    int output_width;              /**< Output image width */
    int output_height;             /**< Output image height */

    unsigned int mode;             /**< GDC mode */
    char maphex_x_path[64];
    char maphex_y_path[64];
    const float* maphex_x_ptr;     /**< LUT X pointer */
    const float* maphex_y_ptr;     /**< LUT Y pointer */

} GdcConfig;


// GDC插件私有数据结构
typedef struct {
    FrameQueue queue_out;
    OutputBufferPool output_pool;

    pthread_t thread;
    int running;
    PixelFormat output_format;

    // GDC配置
    GdcConfig gdc_config;

    unsigned int nInBufferSize;      /**< Input buffer size in bytes */
    unsigned int nOutBufferSize;     /**< Output buffer size in bytes */
    VencIspFunction mIspFunction;    /**< ISP function structure */
    VencIspBufferInfo mInBuffer;     /**< Input buffer info */
    VencIspBufferInfo mOutBuffer;    /**< Output buffer info */
    sInputPara mGdcParam;            /**< GDC parameters */
    struct SunxiMemOpsS* pMemops;    /**< Memory operations */
    VideoEncoderIsp *isp;            /**< ISP handle */

} GdcPluginPriv;

static eWarpType map_user_warp_mode_to_internal(UserWarpMode mode) {
    switch (mode) {
        case USER_WARP_DEFAULT:
            return Warp_LDC;

        case USER_WARP_WIDE:
            return Warp_Fish2Wide;

        case USER_WARP_ULTRA_WIDE:
            return Warp_LDC_Pro;

        case USER_WARP_PANORAMA_180:
            return Warp_Pano180;

        case USER_WARP_PANORAMA_360:
            return Warp_Pano360;

        case USER_WARP_TOP_DOWN:
            return Warp_Normal;

        case USER_WARP_PERSPECTIVE:
            return Warp_Perspective;

        case USER_WARP_BIRDSEYE:
            return Warp_BirdsEye;

        case USER_WARP_CUSTOM:
            return Warp_User;

        default:
            // 如果需要，可以设置默认 warp 模式或返回一个非法值
            return Warp_User;  // 默认模式
    }
}

static void gdc_output_release(struct FrameBuffer* frame, void* user_data)
{
    GdcPluginPriv* priv = (GdcPluginPriv*)user_data;

    output_pool_release(&priv->output_pool, frame);
}

static void update_fps_gdc() {
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


static void* gdc_thread_loop(void* arg) {
    Plugin* self = (Plugin*)arg;
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;

    while (priv->running) {
        // 检查是否有上游插件
        if (!self->upstream || !self->upstream->pull) {
            usleep(10 * 1000);
            continue;
        }

        // 主动从上游拉取FrameBuffer
        FrameBuffer* in = self->upstream->pull(self->upstream);
        if (!in) {
            usleep(10 * 1000);
            continue;
        }

        LOG_INFO("GDC processing frame: %dx%d, format=%d", in->width, in->height, in->format);

        int out_w = priv->gdc_config.output_width > 0 ? priv->gdc_config.output_width : in->width;
        int out_h = priv->gdc_config.output_height > 0 ? priv->gdc_config.output_height : in->height;
        // out_h = ALIGN_16B(out_h);
        // out_w = ALIGN_16B(out_w);
        PixelFormat outFormat = priv->output_format != PIXEL_FORMAT_UNKNOWN ? priv->output_format : in->format;
        //LOG_INFO("GDC out frame: %dx%d, format=%d",out_w, out_h, outFormat);
        if (!priv->output_pool.initialized) {
            output_pool_init(&priv->output_pool, MAX_OUTPUT_BUFFERS, out_w, out_h, outFormat, priv->pMemops);
        }

        // 分配输出FrameBuffer
        // FrameBuffer* out = (FrameBuffer*)calloc(1, sizeof(FrameBuffer));
        // if (!out) {
        //     frame_put(in);
        //     continue;
        // }

        FrameBuffer* out = output_pool_get(&priv->output_pool);
        if (!out) {
            LOG_WARN("%s: No available output buffer", self->name);
            frame_put(in);
            usleep(1000); // 等待空闲
            continue;
        }

        // 设置输出参数
        out->width = out_w;
        out->height = out_h;
        out->format = outFormat;
        out->size = get_image_size(out->format, out->width, out->height);
        out->pts = in->pts;
        //out->ref_count = 0;
        //pthread_mutex_init(&out->ref_mutex, NULL);
        //out->memOps = priv->pMemops;
        out->release = gdc_output_release;
        out->user_data = priv;

        //  填充GDC输入buffer
        VencIspBufferInfo inbuf;
        inbuf.nWidth = in->width;
        inbuf.nHeight = in->height;
        inbuf.pAddrVirY = in->data;
        inbuf.pAddrVirC = inbuf.pAddrVirY + inbuf.nHeight * inbuf.nWidth;
        inbuf.pAddrPhyY = in->Phy_Y_addr;
        inbuf.pAddrPhyC0 = inbuf.pAddrPhyY + inbuf.nWidth * inbuf.nHeight;
        inbuf.pAddrPhyC1 = inbuf.pAddrPhyC0 + inbuf.nWidth * inbuf.nHeight / 4;
        inbuf.colorFormat = 0;

        LOG_DEBUG("inbuf:width=%d, height=%d, Y=%p, C0=%p, C1=%p",
            inbuf.nWidth, inbuf.nHeight,
            inbuf.pAddrPhyY, inbuf.pAddrPhyC0,inbuf.pAddrPhyC1);

        VencIspBufferInfo outbuf;
        outbuf.nWidth = out->width;
        outbuf.nHeight = out->height;
        outbuf.pAddrVirY = out->data;
        outbuf.pAddrVirC = outbuf.pAddrVirY + out->width * out->height;
        outbuf.pAddrPhyY = out->Phy_Y_addr;
        outbuf.pAddrPhyC0 = outbuf.pAddrPhyY + out->width * out->height;
        outbuf.pAddrPhyC1 = outbuf.pAddrPhyC0 + out->width * out->height / 4;
        outbuf.colorFormat = 0;

        LOG_DEBUG("outbuffer:width=%d, height=%d, Y=%p, C0=%p, C1=%p",
            outbuf.nWidth, outbuf.nHeight,
            outbuf.pAddrPhyY, outbuf.pAddrPhyC0, outbuf.pAddrPhyC1);

        priv->mIspFunction.bGdcFlag = 1;
        priv->mIspFunction.pGdcInfo = &priv->mGdcParam;
        priv->mIspFunction.bGdcInfoChange = 1;


        if(priv->isp == NULL)
        {
            LOG_ERROR("priv->isp is NULL\n");
        }
        if (VideoEncIspFunction(priv->isp, &inbuf, &outbuf, &priv->mIspFunction) != 0) {
            LOG_ERROR("GDC process error");
            return NULL;
        }

        LOG_INFO("%s: GDC processed frame: %dx%d -> %dx%d", self->name,
               inbuf.nWidth, inbuf.nHeight, outbuf.nWidth, outbuf.nHeight);

        // 释放输入帧
        frame_put(in);

        update_fps_gdc();
        // 推入输出队列
        if(frame_queue_push_timeout(&priv->queue_out, out, 20))
        {
            LOG_INFO("%s: frame push time out", self->name);
            gdc_output_release(out, priv);
        }
    }

    return NULL;
}

/**
 * Set up GDC user mode, including loading LUT (lookup table) data from files.
 * @param ctx Pointer to the internal gdc_s context.
 * @param config Pointer to the GdcConfig structure.
 */
static void setting_gdc_user_mode(GdcPluginPriv *ctx, const GdcConfig* config)
{
    int width = 0, height = 0, lut_w = 0, lut_h = 0, lut_size = 0;
    FILE *fp = NULL;

    width = config->output_width;
    height = config->output_height;
    LOG_DEBUG("width = %d, height = %d\n",width,height);

    lut_w = (int)floor((width  + 15) / 16) + 1;
    lut_h = (int)floor((height + 15) / 16) + 1;
    lut_size = lut_w * lut_h * sizeof(short);
    //lut_size = 29302;
    LOG_WARN("lut_w %d lut_h %d lut_size %d\n", lut_w, lut_h, lut_size);
    memset(&ctx->mGdcParam, 0, sizeof(sInputPara));
    ctx->mGdcParam.lut_x = (short*)calloc(1, lut_size);
    ctx->mGdcParam.lut_y = (short*)calloc(1, lut_size);
#if 0
//user custom.
    memcpy(ctx->mGdcParam.lut_x, user_define_buffer_x, size);
    memcpy(ctx->mGdcParam.lut_x, user_define_buffer_y, size);
#else
    LOG_DEBUG("fopen config->maphex_x_path\n");
    fp = fopen(LUT_X_PATH, "rb");
    if (fp != NULL) {
        fread(ctx->mGdcParam.lut_x, sizeof(short), lut_size, fp);
        fclose(fp);
        fp = NULL;
    } else {
        LOG_WARN("read file %s error!\n", LUT_X_PATH);
    }
    LOG_DEBUG("fopen config->maphex_y_path\n");
    fp = fopen(LUT_Y_PATH, "rb");
    if (fp != NULL) {
        fread(ctx->mGdcParam.lut_y, sizeof(short), lut_size, fp);
        fclose(fp);
        fp = NULL;
    } else {
        LOG_WARN("read file %s error!\n", LUT_Y_PATH);
    }
#endif
}

static void prepare_gdc(Plugin* self, const GdcConfig* config) {

    GdcPluginPriv* ctx = (GdcPluginPriv*)self->priv;

    ctx->mGdcParam.bGDC_en = 1;
    ctx->mGdcParam.warpMode = config->mode;
    if (ctx->mGdcParam.warpMode == Warp_User) {
        LOG_DEBUG("setting_gdc_user_mode!\n");
        setting_gdc_user_mode(ctx, config);
        LOG_DEBUG("setting_gdc_user_mode end!\n");

        return ;
    }

    if (ctx->mGdcParam.warpMode == Warp_LDC_Pro) {
        char str[64] = "/mnt/gdc_test/ldcpro_2d_str100.bin";
        LOG_DEBUG("sizeof(str) %d", sizeof(str));
        memcpy(ctx->mGdcParam.path_lut_ldc_pro, str, sizeof(str));
    }
    ctx->mGdcParam.mountMode = Mount_Wall;
    ctx->mGdcParam.calib_pic_w  = 2560;
    ctx->mGdcParam.calib_pic_h = 1440;

    ctx->mGdcParam.fx = 2055.88;
    ctx->mGdcParam.fy = 2042.50;
    ctx->mGdcParam.cx = 1279.50;
    ctx->mGdcParam.cy = 719.50;
    ctx->mGdcParam.fx_scale = 1994.32;
    ctx->mGdcParam.fy_scale = 1981.34;
    ctx->mGdcParam.cx_scale = 1279.50;
    ctx->mGdcParam.cy_scale = 719.50;

    ctx->mGdcParam.eLensDistModel = DistModel_WideAngle;

    ctx->mGdcParam.distCoef_wide_ra[0] = -0.3849;
    ctx->mGdcParam.distCoef_wide_ra[1] = 0.1567;
    ctx->mGdcParam.distCoef_wide_ra[2] = -0.0030;
    ctx->mGdcParam.distCoef_wide_ta[0] = -0.00005;
    ctx->mGdcParam.distCoef_wide_ta[1] = 0.0016;

    ctx->mGdcParam.distCoef_fish_k[0]  = -0.0246;
    ctx->mGdcParam.distCoef_fish_k[1]  = -0.0164;
    ctx->mGdcParam.distCoef_fish_k[2]  = 0.0207;
    ctx->mGdcParam.distCoef_fish_k[3]  = -0.0074;

    ctx->mGdcParam.centerOffsetX         =      0;
    ctx->mGdcParam.centerOffsetY         =      0;
    ctx->mGdcParam.rotateAngle           =      0;     //[0,360]
    ctx->mGdcParam.radialDistortCoef     =      0;     //[-255,255]
    ctx->mGdcParam.trapezoidDistortCoef  =      0;     //[-255,255]
    ctx->mGdcParam.fanDistortCoef        =      0;     //[-255,255]
    ctx->mGdcParam.pan                   =      0;     //pano360:[0,360]; others:[-90,90]
    ctx->mGdcParam.tilt                  =      0;     //[-90,90]
    ctx->mGdcParam.zoomH                 =      100;   //[0,100]
    ctx->mGdcParam.zoomV                 =      100;   //[0,100]
    ctx->mGdcParam.scale                 =      100;   //[0,100]
    ctx->mGdcParam.innerRadius           =      0;     //[0,width/2]
    ctx->mGdcParam.roll                  =      0;     //[-90,90]
    ctx->mGdcParam.pitch                 =      0;     //[-90,90]
    ctx->mGdcParam.yaw                   =      0;     //[-90,90]

    ctx->mGdcParam.perspFunc             =    Persp_Only;
    ctx->mGdcParam.perspectiveProjMat[0] =    1.0;
    ctx->mGdcParam.perspectiveProjMat[1] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[2] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[3] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[4] =    1.0;
    ctx->mGdcParam.perspectiveProjMat[5] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[6] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[7] =    0.0;
    ctx->mGdcParam.perspectiveProjMat[8] =    1.0;

    if(ctx->mGdcParam.warpMode == Warp_LDC)
    {
        ctx->mGdcParam.birdsImg_width    = 768;
        ctx->mGdcParam.birdsImg_height   = 1080;
    }
    ctx->mGdcParam.mountHeight           =      0.85; //meters
    ctx->mGdcParam.roiDist_ahead         =      4.5;  //meters
    ctx->mGdcParam.roiDist_left          =     -1.5;  //meters
    ctx->mGdcParam.roiDist_right         =      1.5;  //meters
    ctx->mGdcParam.roiDist_bottom        =      0.65; //meters

    ctx->mGdcParam.peaking_en            =      1;    //0/1
    ctx->mGdcParam.peaking_clamp         =      1;    //0/1
    ctx->mGdcParam.peak_m                =     16;    //[0,63]
    ctx->mGdcParam.th_strong_edge        =      6;    //[0,15]
    ctx->mGdcParam.peak_weights_strength =      2;    //[0,15]
}

static int gdc_plugin_init(Plugin* self, void* gdc_config) {
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;

    GDCConfig* config  = (GDCConfig*)gdc_config;

    priv->pMemops = GetMemAdapterOpsS();
    SunxiMemOpen(priv->pMemops);

    priv->isp = VideoEncIspCreate();
    if (!priv->isp) {
        LOG_ERROR("VideoEncIspCreate failed\n");
        return -1;
    }

    frame_queue_init(&priv->queue_out);
    priv->running = 0;

    // gdcElement->gdcConfig.maphex_y_path = LUT_Y_PATH;
    strcpy(priv->gdc_config.maphex_x_path, LUT_X_PATH);
    strcpy(priv->gdc_config.maphex_y_path, LUT_Y_PATH);
    priv->gdc_config.mode = map_user_warp_mode_to_internal(config->warp_mode);
    LOG_INFO("priv->gdc_config.mode = %d",priv->gdc_config.mode);
    priv->gdc_config.input_width = config->input_width;
    priv->gdc_config.input_height = config->input_height;
    priv->gdc_config.output_width = config->output_width;
    priv->gdc_config.output_height = config->output_height;
    prepare_gdc(self, &priv->gdc_config);

    LOG_INFO("GDC plugin initialized");
    return 0;
}

static int gdc_start(Plugin* self) {
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;
    priv->running = 1;

    return pthread_create(&priv->thread, NULL, gdc_thread_loop, self);
}

static int gdc_stop(Plugin* self) {
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;

    priv->running = 0;

    pthread_cond_broadcast(&priv->queue_out.cond_nonempty);
    pthread_cond_broadcast(&priv->queue_out.cond_nonfull);
    pthread_join(priv->thread, NULL);
    return 0;
}

static int gdc_push(Plugin* self, FrameBuffer* frame) {
    return 0;
}

static FrameBuffer* gdc_pull(Plugin* self) {
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;
    return frame_queue_pop_timeout(&priv->queue_out, 100);
}



Plugin* create_gdc_plugin(const char* name) {
    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    GdcPluginPriv* priv = (GdcPluginPriv*)calloc(1, sizeof(GdcPluginPriv));

    plugin->name = name ? strdup(name) : "GdcPlugin";
    plugin->init = gdc_plugin_init;
    plugin->start = gdc_start;
    plugin->stop = gdc_stop;
    //plugin->process = gdc_process;
    plugin->push = gdc_push;
    plugin->pull = gdc_pull;
    plugin->priv = priv;

    priv->output_format = PIXEL_FORMAT_UNKNOWN; // 使用输入格式

    return plugin;
}

static void gdc_plugin_destroy(Plugin* self) {
    GdcPluginPriv* priv = (GdcPluginPriv*)self->priv;
    if (priv) {
        // 确保线程已停止
        if (priv->running) {
            self->stop(self);
        }

        // 销毁frame队列
        frame_queue_destroy(&priv->queue_out);

        output_pool_destroy(&priv->output_pool);

        if(priv->isp){
            VideoEncIspDestroy(priv->isp);
            priv->isp = NULL;
        }

        // if(priv->mGdcParam.lut_x){
        //     free(priv->mGdcParam.lut_x);
        //     priv->mGdcParam.lut_x = NULL;
        // }

        // if(priv->mGdcParam.lut_y){
        //     free(priv->mGdcParam.lut_y);
        //     priv->mGdcParam.lut_y = NULL;
        // }

        // 释放内存操作
        if (priv->pMemops) {
            SunxiMemClose(priv->pMemops);
            priv->pMemops = NULL;
        }

        free(priv);
    }
    self->priv = NULL;
    LOG_INFO("gdc_plugin_destroy\n");
}

// 独立的销毁函数，供外部调用
void destroy_gdc_plugin(Plugin* plugin) {
    if (plugin) {
        if (plugin->priv) {
            gdc_plugin_destroy(plugin);
        }
        if (plugin->name) {
            free(plugin->name);
            plugin->name = NULL;
        }
        free(plugin);
    }
}
