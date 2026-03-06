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
#include <unistd.h>
#include "plugins/plugin_interface.h"
#include "plugins/v4l2src/v4l2_camera_plugin.h"
#include "plugins/g2d/g2d_plugin.h"
#include "plugins/gdc/gdc_plugin.h"
#include "utils/log.h"
#include "plugins/Encoder/encoder_plugin.h"
#include <string.h>
typedef struct {
    FILE *fp;
} H264Writer;

// 打开 H264 文件
int h264_writer_open(H264Writer *writer, const char *filename) {
    if (!writer || !filename) return -1;

    writer->fp = fopen(filename, "wb"); // 写新文件, 如果要追加可用 "ab"
    if (!writer->fp) {
        perror("fopen failed");
        return -2;
    }
    return 0;
}

// 写入一段 H264 数据流
int h264_writer_write(H264Writer *writer, const unsigned char *data, size_t size) {
    if (!writer || !writer->fp || !data || size == 0) return -1;

    size_t written = fwrite(data, 1, size, writer->fp);
    if (written != size) {
        perror("fwrite failed");
        return -2;
    }
    return 0;
}

// 关闭文件
void h264_writer_close(H264Writer *writer) {
    if (writer && writer->fp) {
        fclose(writer->fp);
        writer->fp = NULL;
    }
}

void run_pipeline_once() {
    // 1. 创建并初始化插件
    Plugin* cam = create_v4l2_camera_plugin("isp");
    if (!cam) {
        fprintf(stderr, "Failed to create V4L2 plugin\n");
        return;
    }

    V4L2CameraConfig cam_config;
    cam_config.camea_index = 4;
    cam_config.fps = 30;
    cam_config.width = 640;
    cam_config.height = 480;
    cam_config.pixel_format = PIXEL_FORMAT_NV12;
    cam_config.isOfflineTest = 1;
    cam_config.useIsp = 1;
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
    gdc_config.input_width = 640;
    gdc_config.input_height = 480;
    gdc_config.output_width = 640;
    gdc_config.output_height = 480;
    gdc_config.warp_mode = USER_WARP_CUSTOM;
    if (gdc->init(gdc, &gdc_config) != 0) {
        fprintf(stderr, "Failed to init GDC plugin\n");
        destroy_gdc_plugin(gdc);
        destroy_v4l2_camera_plugin(cam);
        return;
    }


    Plugin* g2d = create_g2d_plugin("g2d");
    if (!g2d) goto cleanup;
    g2d_set_rotation(g2d, 180);


    Plugin* jpegenc = create_encoder_plugin("jpegenc");
    if (!jpegenc) goto cleanup;
    encConfig encoder_config = {0};
    encoder_config.input_width = 640;
    encoder_config.input_height = 480;
    encoder_config.input_fps = 30;
    encoder_config.input_format = PIXEL_FORMAT_NV12;
    if (jpegenc->init(jpegenc, &encoder_config) != 0) {
        LOG_ERROR("h264 encoder init fail");
        goto cleanup;
    }

    plugin_link_many(cam, gdc, g2d, jpegenc, NULL);

    printf("plugin_link done\n");

    // 3. 启动链
    if (plugin_chain_start(cam) != 0) {
        fprintf(stderr, "Failed to start plugin chain\n");
        goto cleanup;
    }
    printf("plugin_chain_start done\n");
    //char jpeg_path[256];

    for(int i=0;i<200;i++)
    {
        FrameBuffer* fb = jpegenc->pull(jpegenc);
        if (fb) {
            // sprintf(jpeg_path, "%d.jpeg", i);
            // FILE *out_file = fopen(jpeg_path, "wb");
            // if(out_file == NULL){
            //     printf(" open %s fail\n", jpeg_path);
            //     return -1;
            // }
            frame_get(fb);
            //fwrite(fb->data, fb->size, 1, out_file);
            frame_put(fb);  // 释放帧
            //fclose(out_file);
            //out_file = NULL;
        } else {
            printf("Failed to pull frame\n");
        }
    }

    // 停止链 - 从下游到上游停止（由plugin_chain_stop内部处理）
    printf("Stopping plugin chain (downstream to upstream)...\n");
    plugin_chain_stop(cam);  // 传入链头，函数内部会从链尾开始停止
    printf("Plugin chain stopped\n");

    // 释放资源 - 从下游到上游释放（与停止顺序一致）
cleanup:
    printf("Cleaning up resources (downstream to upstream)...\n");
    if (jpegenc) { printf("Destroying h264enc...\n"); destroy_encoder_plugin(jpegenc); }
    if (g2d) { printf("Destroying g2d_1...\n"); destroy_g2d_plugin(g2d); }
    if (gdc) { printf("Destroying gdc...\n"); destroy_gdc_plugin(gdc); }
    if (cam) { printf("Destroying cam...\n"); destroy_v4l2_camera_plugin(cam); }
    printf("Resources cleaned up\n");
}

int main() {

    run_pipeline_once();

    return 0;
}
