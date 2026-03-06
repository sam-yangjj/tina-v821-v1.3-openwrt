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
#ifndef YOLOV5_PLUGIN_H
#define YOLOV5_PLUGIN_H

#include "../plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a YOLOv5 inference plugin instance
 *
 * Creates and initializes a YOLOv5 AI inference plugin with the specified
 * configuration. The plugin provides real-time object detection capabilities
 * using hardware-accelerated NPU processing.
 *
 * The plugin creates a thread pool for parallel inference processing, allowing
 * multiple frames to be processed simultaneously for improved throughput.
 * Each thread in the pool maintains its own YOLOv5 network instance to avoid
 * resource conflicts.
 *
 * @param name Plugin instance name (can be NULL for default "Yolov5Plugin")
 * @param thread_count Number of inference threads in the thread pool
 *                     (recommended: 1-4 based on system capabilities)
 * @param model_path Path to the YOLOv5 model file (.nbg format)
 *                   (can be NULL to use default model path)
 * @return Pointer to the created Plugin instance, or NULL on failure
 *
 * @note The plugin must be initialized and started before it can process frames.
 *       The model file must be compatible with the target NPU hardware.
 *
 * @example
 * ```c
 * Plugin* yolo = create_yolov5_plugin("YOLO", 2, "/path/to/model.nbg");
 * yolo->init(yolo, NULL);
 * yolo->start(yolo);
 * ```
 */
Plugin* create_yolov5_plugin(const char* name, int thread_count, const char* model_path);

void destroy_yolov5_plugin(Plugin* self);

#ifdef __cplusplus
}
#endif

#endif // YOLOV5_PLUGIN_H
