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
#include "yolov5_plugin.h"
#include "yolov5/YoloThreadPool.h"
#include "yolov5/YoloTaskQueue.h"
#include "../frame_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "../../utils/log.h"
#include "frame_min_heap.h"

/**
 * @brief Private data structure for YOLOv5 plugin instance
 *
 * Contains all the internal state and resources needed by the YOLOv5 plugin,
 * including thread pool management, task queues, and configuration settings.
 */
typedef struct {
    YoloThreadPool* pool;           /**< Multi-threaded inference engine */
    YoloTaskQueue* queue;           /**< Input task queue for inference requests */
    FrameQueue result_queue;        /**< Output queue for processed frames */
    //FrameMinHeap* heap;
    int width;
    int height;
    PixelFormat output_format;
    int running;                    /**< Plugin running state flag */
    pthread_t thread;               /**< Main processing thread handle */
} Yolov5PluginPriv;

/**
 * @brief Inference completion callback function
 *
 * Called when an inference task is completed by the thread pool.
 * This callback can be used to trigger additional processing or
 * notify other components about completed inferences.
 *
 * @param buffer_index Index of the buffer that was processed
 * @param user_data Pointer to plugin private data (Yolov5PluginPriv)
 */
static void yolov5_infer_complete_callback(FrameBuffer* frame, void* user_data) {

    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)user_data;

    if(frame)
    {
        frame->ref_count = 0;
        //if(frame_queue_push_timeout(&priv->result_queue, frame, 50))
        if(frame_queue_push(&priv->result_queue, frame, &priv->running)){
            frame_put(frame);
        }
        //frame->release = NULL;
        //InferenceResult* r = malloc(sizeof(InferenceResult));
        // r->frame_index = frame->buf_index;
        // r->processed_image = frame;
        // LOG_INFO("heap_push");
        // if(heap_push(priv->heap, r))
        // {
        //     frame->ref_count = 1;
        //     frame_put(frame);
        //     free(r);
        // }
    }

}

/**
 * @brief Main processing thread function
 *
 * This thread continuously pulls frames from the upstream plugin and
 * dispatches them to the YOLOv5 thread pool for inference. It handles
 * the conversion from FrameBuffer to YoloTask format and manages the
 * data flow between plugin stages.
 *
 * @param arg Pointer to the Plugin instance
 * @return NULL (standard pthread return)
 */
static void* yolov5_thread_loop(void* arg) {
    Plugin* self = (Plugin*)arg;
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;
    LOG_DEBUG("yolov5_thread_loop");
    static int index = 0;
    while (priv->running) {
        // Check if upstream plugin is available
        if (!self->upstream || !self->upstream->pull) {
            usleep(10 * 1000);  // 10ms delay to prevent busy waiting
            continue;
        }

        // Pull frame from upstream plugin (camera, preprocessor, etc.)
        FrameBuffer* frame = self->upstream->pull(self->upstream);
        if (!frame) {
            LOG_WARN("%s: Get frame timeout", self->name);
            usleep(10 * 1000);  // No frame available, wait and retry
            continue;
        }
        if(frame->height != 640 || frame->width != 640){
            LOG_ERROR("invalid frame width or height");
            break;
        }
        frame_get(frame);
        LOG_INFO("YOLOV5 Processing frame: %dx%d, fd=%d\n", frame->width, frame->height, frame->fd);
        // Convert FrameBuffer to YoloTask for inference processing
        YoloTask* task = (YoloTask*)malloc(sizeof(YoloTask));
        task->frame = frame;
        task->width = frame->width;                  // Frame width in pixels
        task->height = frame->height;                // Frame height in pixels

        // Set up inference completion callback
        task->inference_callback = yolov5_infer_complete_callback;
        task->inference_user_data = priv;
        LOG_INFO("yolo_task_queue_push");
        // Submit task to inference thread pool
        if(yolo_task_queue_push(priv->queue, task, &priv->running) == false){
            frame_put(frame);
            free(task);
            task = NULL;
        }

    }

    return NULL;
}

/**
 * @brief Initialize the YOLOv5 plugin
 *
 * Sets up all internal structures, initializes the thread pool,
 * and prepares the plugin for processing. This must be called
 * before starting the plugin.
 *
 * @param self Pointer to the plugin instance
 * @param config Configuration parameters (currently unused)
 * @return 0 on success, negative error code on failure
 */
static int yolov5_plugin_init(Plugin* self, void* config) {
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;

    // Initialize task queue for inference requests
    yolo_task_queue_init(priv->queue);

    // Initialize result queue for processed frames
    frame_queue_init(&priv->result_queue);
    //priv->heap = heap_create(30);
    // Initialize YOLOv5 thread pool with model and thread configuration
    yolo_thread_pool_init(priv->pool, priv->queue, priv->pool->model_path, priv->pool->thread_count);

    priv->running = 0;  // Plugin not running yet
    return 0;
}

/**
 * @brief Start the YOLOv5 plugin processing
 *
 * Starts the inference thread pool and the main processing thread.
 * After this call, the plugin will begin processing frames from
 * the upstream plugin.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success, negative error code on failure
 */
static int yolov5_start(Plugin* self) {
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;
    priv->running = 1;

    // Start the YOLOv5 inference thread pool
    yolo_thread_pool_start(priv->pool);

    // Start the main processing thread
    return pthread_create(&priv->thread, NULL, yolov5_thread_loop, self);
}

/**
 * @brief Stop the YOLOv5 plugin processing
 *
 * Signals all threads to stop and waits for them to complete.
 * This ensures a clean shutdown of all plugin resources.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success, negative error code on failure
 */
static int yolov5_stop(Plugin* self) {
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;
    priv->running = 0;

    // Stop the inference thread pool
    yolo_thread_pool_stop(priv->pool);

    pthread_cond_broadcast(&priv->result_queue.cond_nonempty);
    pthread_cond_broadcast(&priv->result_queue.cond_nonfull);
    pthread_cond_broadcast(&priv->queue->not_full);
    pthread_cond_broadcast(&priv->queue->not_empty);
    // Wait for main processing thread to complete
    pthread_join(priv->thread, NULL);

    return 0;
}

/**
 * @brief Process a single frame (push mode - not used)
 *
 * In pull mode architecture, this function is not used as the plugin
 * actively pulls frames from upstream rather than receiving pushed frames.
 *
 * @param self Pointer to the plugin instance
 * @param frame Frame to process
 * @return 0 (no operation performed)
 */
static int yolov5_process(Plugin* self, FrameBuffer* frame) {
    // Pull mode: data flow is managed by yolov5_thread_loop
    return 0;
}

/**
 * @brief Push a frame to downstream (not applicable for this plugin)
 *
 * @param self Pointer to the plugin instance
 * @param frame Frame to push
 * @return 0 (no operation performed)
 */
static int yolov5_push(Plugin* self, FrameBuffer* frame) {
    return 0;
}

/**
 * @brief Pull a processed frame from the plugin output
 *
 * Downstream plugins call this function to retrieve frames that have
 * been processed by YOLOv5 inference. The returned frames contain
 * the original image data with detection results drawn as overlays.
 *
 * @param self Pointer to the plugin instance
 * @return Pointer to processed FrameBuffer, or NULL if no frame available
 */
static FrameBuffer* yolov5_pull(Plugin* self) {
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;

    // InferenceResult* matched = heap_pop(priv->heap);
    // if(matched == NULL)
    //     return NULL;
    // return matched->processed_image;
    return frame_queue_pop_timeout(&priv->result_queue, 100);
}

/**
 * @brief Destroy the YOLOv5 plugin and free all resources
 *
 * Cleans up all allocated memory, destroys thread pool and queues,
 * and frees the plugin's private data structure.
 *
 * @param self Pointer to the plugin instance
 */
static void  yolov5_plugin_destroy(Plugin* self) {
    if (!self || !self->priv) return;

    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)self->priv;

    // 确保线程已停止
    if (priv->running) {
        self->stop(self);
    }

    // Destroy inference thread pool
    if (priv->pool) {
        yolo_thread_pool_destroy(priv->pool);
        free(priv->pool);
        priv->pool = NULL;
    }

    // Destroy task queue
    if (priv->queue) {
        yolo_task_queue_destroy(priv->queue);
        free(priv->queue);
        priv->queue = NULL;
    }

    // Destroy result queue
    frame_queue_destroy(&priv->result_queue);

    // Free private data
    free(priv);
    self->priv = NULL;

    LOG_INFO("yolov5_plugin_destroy\n");

}

Plugin* create_yolov5_plugin(const char* name, int thread_count, const char* model_path) {
    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    Yolov5PluginPriv* priv = (Yolov5PluginPriv*)calloc(1, sizeof(Yolov5PluginPriv));
    priv->queue = (YoloTaskQueue*)calloc(1, sizeof(YoloTaskQueue));
    priv->pool = (YoloThreadPool*)calloc(1, sizeof(YoloThreadPool));
    priv->pool->thread_count = thread_count;
    if (model_path) {
        strcpy(priv->pool->model_path, model_path);
    }
    plugin->name = name ? strdup(name) : "Yolov5Plugin";
    plugin->init = yolov5_plugin_init;
    plugin->start = yolov5_start;
    plugin->stop = yolov5_stop;
    plugin->process = yolov5_process;
    plugin->push = yolov5_push;
    plugin->pull = yolov5_pull;
    plugin->priv = priv;
    return plugin;
}
void destroy_yolov5_plugin(Plugin* self)
{
    if (self) {
        if (self->priv) {
            yolov5_plugin_destroy(self);
        }
        if (self->name) {
            free(self->name);
            self->name = NULL;
        }
        free(self);
        self = NULL;
    }
}

