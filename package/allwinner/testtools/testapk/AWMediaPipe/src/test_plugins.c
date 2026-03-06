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
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "plugins/plugin_interface.h"
#include "plugins/v4l2src/v4l2_camera_plugin.h"
#include "plugins/g2d/g2d_plugin.h"
#include "plugins/gdc/gdc_plugin.h"
#include "plugins/npu/yolov5/draw_detections/draw_result.h"
#include "ion_mem_alloc.h"
#include "plugins/npu/yolov5_plugin.h"
#include "utils/log.h"
#include "plugins/filesink/filesink_plugin.h"
#include "plugins/Encoder/encoder_plugin.h"
#include "plugins/rtsp/rtsp_plugin.h"
#include <string.h>
#include "plugins/buffer_pool.h"

// 打印帧信息（调试用）
void dump_frame_info(FrameBuffer* fb, const char* tag) {
    if (!fb) return;
    printf("[%s] %dx%d, format=%d, size=%d, pts=%ld\n", tag,
        fb->width, fb->height, fb->format, fb->size, fb->pts);
}

// 运行一次完整的 pipeline 并释放资源
void run_pipeline_once() {
    // 1. 创建并初始化插件
    Plugin* cam = create_v4l2_camera_plugin("v4l2cam");
    if (!cam) {
        fprintf(stderr, "Failed to create V4L2 plugin\n");
        return;
    }

    V4L2CameraConfig cam_config;
    cam_config.camea_index = 0;
    cam_config.fps = 30;
    cam_config.width = 2592;
    cam_config.height = 1944;
    cam_config.pixel_format = PIXEL_FORMAT_YUV420P;
    if (cam->init(cam, &cam_config) != 0) {
        fprintf(stderr, "Failed to init V4L2 camera\n");
        destroy_v4l2_camera_plugin(cam);
        return;
    }

    Plugin* gdc = create_gdc_plugin("gdc");
    if (!gdc) {
        fprintf(stderr, "Failed to create GDC plugin\n");
        destroy_v4l2_camera_plugin(cam);
        return;
    }

    GDCConfig gdc_config;
    gdc_config.input_width = 2560;
    gdc_config.input_height = 1440;
    gdc_config.output_width = 2560;
    gdc_config.output_height = 1440;
    gdc_config.warp_mode = USER_WARP_CUSTOM;
    if (gdc->init(gdc, &gdc_config) != 0) {
        fprintf(stderr, "Failed to init GDC plugin\n");
        destroy_gdc_plugin(gdc);
        destroy_v4l2_camera_plugin(cam);
        return;
    }

    Plugin* g2d_1 = create_g2d_plugin("g2d_1");
    if (!g2d_1) goto cleanup;
    g2d_set_output_size(g2d_1, 2560, 1440);

    Plugin* g2d_2 = create_g2d_plugin("g2d_2");
    if (!g2d_2) goto cleanup;
    g2d_set_output_format(g2d_2, PIXEL_FORMAT_RGB888);
    g2d_set_output_size(g2d_2, 640, 640);

    Plugin* g2d_3 = create_g2d_plugin("g2d_3");
    if (!g2d_3) goto cleanup;
    g2d_set_output_format(g2d_3, PIXEL_FORMAT_YUV420P);
    g2d_set_output_size(g2d_3, 640, 480);

    Plugin* yolo = create_yolov5_plugin("yolo", 5, "./model/yolov5.nb");
    if (!yolo) goto cleanup;
    if (yolo->init(yolo, NULL) != 0) {
        fprintf(stderr, "Failed to init YOLO plugin\n");
        goto cleanup;
    }

    Plugin* h264enc = create_encoder_plugin("h264enc");
    if (!h264enc) goto cleanup;
    encConfig encoder_config = {0};
    encoder_config.h264_input_width = 640;
    encoder_config.h264_input_height = 480;
    encoder_config.h264_input_fps = 30;
    encoder_config.h264_bitrate = 2 * 1024 * 1024;
    encoder_config.h264_input_format = PIXEL_FORMAT_YUV420P;

    if (h264enc->init(h264enc, &encoder_config) != 0) {
        LOG_ERROR("h264 encoder init fail");
        goto cleanup;
    }

    // Plugin* rtspsink = create_rtsp_plugin("rtspsink", "192.168.3.22", 8554);
    // if (rtspsink->init(rtspsink, NULL)) {
    //     LOG_ERROR("Can not bind 192.168.3.22:8554");
    //     goto cleanup;
    // }

    // 2. 连接插件
    plugin_link_many(cam, g2d_1, gdc, g2d_2,yolo, g2d_3, h264enc, NULL);
    printf("plugin_link done\n");

    // 3. 启动链
    if (plugin_chain_start(cam) != 0) {
        fprintf(stderr, "Failed to start plugin chain\n");
        goto cleanup;
    }
    printf("plugin_chain_start done\n");

    // 4. 拉取一帧
    //usleep(2000* 1000);  // 等待链路稳定，增加等待时间

    FrameBuffer* fb = h264enc->pull(h264enc);
    if (fb) {
        frame_get(fb);
        LOG_INFO("========================Pulled Frame=========================");
        frame_put(fb);  // 释放帧
    } else {
        printf("Failed to pull frame\n");
    }



    // 5. 停止链 - 从下游到上游停止（由plugin_chain_stop内部处理）
    printf("Stopping plugin chain (downstream to upstream)...\n");
    plugin_chain_stop(cam);  // 传入链头，函数内部会从链尾开始停止
    printf("Plugin chain stopped\n");

    // 6. 释放资源 - 从下游到上游释放（与停止顺序一致）
cleanup:
    printf("Cleaning up resources (downstream to upstream)...\n");
    if (h264enc) { printf("Destroying h264enc...\n"); destroy_encoder_plugin(h264enc); }
    if (g2d_3) { printf("Destroying g2d_3...\n"); destroy_g2d_plugin(g2d_3); }
    if (yolo) { printf("Destroying yolo...\n"); destroy_yolov5_plugin(yolo); }
    if (g2d_2) { printf("Destroying g2d_2...\n"); destroy_g2d_plugin(g2d_2); }
    if (g2d_1) { printf("Destroying g2d_1...\n"); destroy_g2d_plugin(g2d_1); }
    if (gdc) { printf("Destroying gdc...\n"); destroy_gdc_plugin(gdc); }
    //if (rtspsink) destroy_rtsp_plugin(rtspsink);
    if (cam) { printf("Destroying cam...\n"); destroy_v4l2_camera_plugin(cam); }
    printf("Resources cleaned up\n");
}

int main() {
    int64_t i = 0;

    while (1) {
        printf("\n--- Running pipeline %d---\n", i+1);
        run_pipeline_once();
        usleep(100*1000);
        i++;
    }
    return 0;
}

#endif
