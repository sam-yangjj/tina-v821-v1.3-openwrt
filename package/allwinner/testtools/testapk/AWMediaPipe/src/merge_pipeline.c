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
#if 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "plugins/plugin_interface.h"
#include "plugins/v4l2src/v4l2_camera_plugin.h"
#include "plugins/g2d/g2d_plugin.h"
#include "plugins/npu/yolov5_plugin.h"
#include "plugins/gdc/gdc_plugin.h"
#include "plugins/Encoder/encoder_plugin.h"
#include "plugins/filesink/filesink_plugin.h"
#include "plugins/npu/yolov5/draw_detections/draw_result.h"
#include "utils/log.h"

#define CAM_WIDTH 640
#define CAM_HEIGHT 480
#define CHANNELS 3 // RGB888

typedef struct {
    Plugin* cam;
    Plugin* g2d;
    Plugin* gdc;
} CameraPipelineForMerge;


// 初始化一个摄像头pipeline
int init_camera_pipeline(CameraPipelineForMerge* pipeline, const char* prefix, int cam_index) {
    char name[32];

    snprintf(name, sizeof(name), "%s_cam", prefix);
    pipeline->cam = create_v4l2_camera_plugin(name);
    V4L2CameraConfig cam_config = {cam_index, CAM_WIDTH, CAM_HEIGHT, 30, PIXEL_FORMAT_YUV420P, 1};
    pipeline->cam->init(pipeline->cam, &cam_config);

    snprintf(name, sizeof(name), "%s_gdc", prefix);
    pipeline->gdc = create_gdc_plugin(name);
    GDCConfig gdc_config = {USER_WARP_CUSTOM, CAM_WIDTH, CAM_HEIGHT, CAM_WIDTH, CAM_HEIGHT};
    pipeline->gdc->init(pipeline->gdc, &gdc_config);

    snprintf(name, sizeof(name), "%s_g2d", prefix);
    pipeline->g2d = create_g2d_plugin(name);
    g2d_set_output_format(pipeline->g2d, PIXEL_FORMAT_RGB888);
    g2d_set_output_size(pipeline->g2d, CAM_WIDTH, CAM_HEIGHT);

    plugin_link_many(pipeline->cam, pipeline->gdc, pipeline->g2d, NULL);

    return 0;
}

void start_camera_pipeline(CameraPipelineForMerge* pipeline) {
    plugin_chain_start(pipeline->cam);
}

void stop_camera_pipeline(CameraPipelineForMerge* pipeline) {
    plugin_chain_stop(pipeline->cam);
}

void destroy_camera_pipeline(CameraPipelineForMerge* pipeline) {
    destroy_g2d_plugin(pipeline->g2d);
    destroy_gdc_plugin(pipeline->gdc);
    destroy_v4l2_camera_plugin(pipeline->cam);
}
FrameBuffer* pull_frame(CameraPipelineForMerge* pipeline) {
    return pipeline->g2d->pull(pipeline->g2d);
}

// 合并两张RGB图像为一张宽度翻倍的图像
uint8_t* merge_frames(FrameBuffer* fb1, FrameBuffer* fb2, int* out_width, int* out_height) {
    if (!fb1 || !fb2) return NULL;

    int width = fb1->width + fb2->width;
    int height = fb1->height;
    int size = width * height * CHANNELS;

    uint8_t* merged = (uint8_t*)malloc(size);
    if (!merged) return NULL;

    for (int y = 0; y < height; y++) {
        memcpy(merged + y * width * CHANNELS,
               (uint8_t*)fb1->data + y * fb1->width * CHANNELS,
               fb1->width * CHANNELS);
        memcpy(merged + y * width * CHANNELS + fb1->width * CHANNELS,
               (uint8_t*)fb2->data + y * fb2->width * CHANNELS,
               fb2->width * CHANNELS);
    }

    *out_width = width;
    *out_height = height;
    return merged;
}

int main() {
    CameraPipelineForMerge Pipeline1, Pipeline2;

    #if 1
    init_camera_pipeline(&Pipeline1, "cam1", 1);
    init_camera_pipeline(&Pipeline2, "cam2", 5);

    start_camera_pipeline(&Pipeline1);
    start_camera_pipeline(&Pipeline2);


    int frame_count = 0;
    char path[128];

    while (1) {
        FrameBuffer* fb1 = pull_frame(&Pipeline1);
        FrameBuffer* fb2 = pull_frame(&Pipeline2);

        if (!fb1 || !fb2) {
            printf("Failed to pull frame\n");
            continue;
        }

        frame_get(fb1);
        frame_get(fb2);

        int out_width, out_height;
        uint8_t* merged = merge_frames(fb1, fb2, &out_width, &out_height);
        if (merged) {
            snprintf(path, sizeof(path), "/tmp/img/result%d.png", frame_count++ % 10);
            save_to_png(merged, out_width, out_height, CHANNELS, path);
            printf("Saved: %s\n", path);
            free(merged);
        }

        frame_put(fb1);
        frame_put(fb2);

        if(frame_count >= 1000)
            break;
    }

    stop_camera_pipeline(&Pipeline1);
    stop_camera_pipeline(&Pipeline2);
    destroy_camera_pipeline(&Pipeline1);
    destroy_camera_pipeline(&Pipeline2);

    #endif
    return 0;
}
#endif
