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


typedef struct {
    Plugin* cam;
    Plugin* g2d1;
    Plugin* gdc;
    Plugin* g2d2;
    Plugin* yolo;
    Plugin* g2d3;
    Plugin* encoder;
    Plugin* filesink;
} CameraPipelineForNPU;


int init_camera_pipeline_npu(CameraPipelineForNPU* pipeline, int cam_index)
{
    pipeline->cam = create_v4l2_camera_plugin("cam0");
    V4L2CameraConfig cam_config = {cam_index, 2592, 1944, 30, PIXEL_FORMAT_YUV420P, 1};
    pipeline->cam->init(pipeline->cam, &cam_config);
    pipeline->g2d1 = create_g2d_plugin("g2d1");
    g2d_set_output_format(pipeline->g2d1, PIXEL_FORMAT_YUV420P);
    g2d_set_output_size(pipeline->g2d1, 2560, 1440);

    pipeline->gdc = create_gdc_plugin("gdc");
    GDCConfig gdc_config;
    gdc_config.input_width = 2560;
    gdc_config.input_height = 1440;
    gdc_config.output_width = 2560;
    gdc_config.output_height = 1440;
    gdc_config.warp_mode = USER_WARP_CUSTOM;
    pipeline->gdc->init(pipeline->gdc, &gdc_config);


    pipeline->g2d2 = create_g2d_plugin("g2d2");
    g2d_set_output_size(pipeline->g2d2, 640, 640);

    pipeline->yolo = create_yolov5_plugin("yolo", 5, "./model/yolov5.nb");
    pipeline->yolo->init(pipeline->yolo, NULL);

    pipeline->g2d3 = create_g2d_plugin("g2d3");
    g2d_set_output_format(pipeline->g2d3, PIXEL_FORMAT_YUV420P);
    g2d_set_output_size(pipeline->g2d3, 640, 480);

    pipeline->encoder = create_encoder_plugin("h264enc");
    encConfig encoder_config = {0};
    encoder_config.h264_input_width = 640;
    encoder_config.h264_input_height = 480;
    encoder_config.h264_input_fps = 30;
    encoder_config.h264_bitrate = 2 * 1024 * 1024;
    encoder_config.h264_input_format = PIXEL_FORMAT_YUV420P;
    if (pipeline->encoder->init(pipeline->encoder, &encoder_config) != 0) {
        LOG_ERROR("h264 encoder init fail");
    }

    pipeline->filesink = create_filesink_plugin("filesink", "/tmp/output.h264");
    filesink_set_filename(pipeline->filesink, "/tmp/output.h264");
    pipeline->filesink->init(pipeline->filesink, NULL);
    plugin_link_many(pipeline->cam, pipeline->g2d1, pipeline->gdc, pipeline->g2d2,
            pipeline->yolo,pipeline->g2d3, pipeline->encoder, pipeline->filesink, NULL);
}

void start_camera_pipeline(CameraPipelineForNPU* pipeline) {
    plugin_chain_start(pipeline->cam);
}
void destroy_camera_pipeline_npu(CameraPipelineForNPU* pipeline) {
    destroy_filesink_plugin(pipeline->filesink);
    destroy_encoder_plugin(pipeline->encoder);
    destroy_g2d_plugin(pipeline->g2d2);
    destroy_yolov5_plugin(pipeline->yolo);
    destroy_g2d_plugin(pipeline->g2d1);
    destroy_v4l2_camera_plugin(pipeline->cam);
}

void stop_camera_pipeline(CameraPipelineForNPU* pipeline) {
    plugin_chain_stop(pipeline->cam);
}

int main() {

    CameraPipelineForNPU pipeline;
    init_camera_pipeline_npu(&pipeline, 0);
    start_camera_pipeline(&pipeline);

    int frame_count = 0;
    char path[128];

    while (1) {
        usleep(100 * 1000);
    }

    stop_camera_pipeline(&pipeline);

    destroy_camera_pipeline_npu(&pipeline);
    return 0;
}
#endif
