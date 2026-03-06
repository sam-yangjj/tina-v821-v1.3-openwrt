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
#include "encoder_plugin.h"
#include "../frame_queue.h"
#include "ion_mem_alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include "../../utils/log.h"
#include "JpegEncoder.h"
#include "../buffer_pool.h"
#include "../../utils/frame_check.h"

// 程序启动时记录
static struct timeval start_time;


/**
 * @brief Private data structure for the encoder plugin
 *
 * Contains the encoder instance, output queue, thread management,
 * and buffer allocation for encoded frames.
 */
typedef struct {
    JPEGEncoder* encoder;                    // encoder instance
    FrameQueue queue_out;                   // Output queue for encoded frames
    OutputBufferPool output_pool;
    int running;                            // Thread running flag
    pthread_t thread;                       // Worker thread handle
    struct SunxiMemOpsS* memOps;           // Memory operations for DMA allocation
} EncoderPluginPriv;


void init_pts_base() {
    gettimeofday(&start_time, NULL);
}

// 获取当前帧的 PTS（单位：90kHz）
uint64_t get_current_pts_90k() {
    struct timeval now;
    gettimeofday(&now, NULL);

    // 计算时间差（秒 + 微秒）
    double delta_sec = (now.tv_sec - start_time.tv_sec) +
                       (now.tv_usec - start_time.tv_usec) / 1e6;

    // 转换成 90kHz PTS
    return (uint64_t)(delta_sec * 90000);
}

static void update_fps_encoder() {
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


static void encoder_output_release(struct FrameBuffer* frame, void* user_data)
{
    EncoderPluginPriv* priv = (EncoderPluginPriv*)user_data;

    VencOutputBuffer* output = (VencOutputBuffer*)frame->user_data2;

    FreeOneBitStreamFrame(priv->encoder->pVideoEnc, output);

    if(output){
        free(output);
        output = NULL;
    }

    if (frame){
        free(frame);
        frame = NULL;
    }

}


/**
 * @brief Main thread loop for encoder processing
 *
 * This function runs in a separate thread and continuously:
 * 1. Pulls frames from upstream plugin
 * 2. Converts FrameBuffer to VencInputBuffer
 * 3. Encodes the frame using encoder
 * 4. Creates output FrameBuffer with encoded data
 * 5. Pushes encoded frame to output queue
 *
 * @param arg Pointer to the Plugin instance
 * @return NULL
 */
static void* encoder_thread_loop(void* arg) {
    Plugin* self = (Plugin*)arg;
    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;
    LOG_DEBUG("Encoder thread started");
    VencInputBuffer inBuffer;

    uint8_t* ref_frame = NULL;
    static int index = 0;
    while (priv->running) {
        // Check if upstream plugin is available
        if (!self->upstream || !self->upstream->pull) {
            usleep(1000); // 1ms
            continue;
        }

        // Pull frame from upstream plugin
        FrameBuffer* in_frame = self->upstream->pull(self->upstream);
        if (!in_frame) {
            usleep(1000); // 1ms
            continue;
        }
        frame_get(in_frame);
        if(!in_frame->data)
        {
            frame_put(in_frame);
            continue;
        }
        //LOG_INFO("%s: Encoding frame: %dx%d, fd=%d, index=%d", self->name, in_frame->width, in_frame->height, in_frame->fd, in_frame->buf_index);
        memset(&inBuffer,0,sizeof(VencInputBuffer));
        LOG_DEBUG("Encoder received frame: %dx%d, format=%d, size=%d, fd=%d\n",
               in_frame->width, in_frame->height, in_frame->format, in_frame->size, in_frame->fd);

        uint8_t* converted_buffer = NULL;
        uint8_t* src = NULL;
        size_t converted_size = 0;
        //GetOneAllocInputBuffer(priv->encoder->pVideoEnc,&inBuffer);
        #if 0
        if(in_frame->format == PIXEL_FORMAT_RGB888)
        {
            converted_size = in_frame->width * in_frame->height * 4;
            converted_buffer = inBuffer.pAddrVirY;
            src = in_frame->data;

            int total_pixels = in_frame->width * in_frame->height;
            // 从后往前复制，避免覆盖
            for (int i = total_pixels - 1; i >= 0; --i) {
                converted_buffer[i * 4 + 0] = 0xFF;             // A
                converted_buffer[i * 4 + 3] = src[i * 3 + 0];  // B
                converted_buffer[i * 4 + 2] = src[i * 3 + 1];  // G
                converted_buffer[i * 4 + 1] = src[i * 3 + 2];  // R
            }
        }
        #endif

        unsigned char* base = (unsigned char*)in_frame->data;
        int width = in_frame->width;
        int height = in_frame->height;
        // LOG_DEBUG("in_frame format = %d",in_frame->format);
        switch (in_frame->format) {
            case PIXEL_FORMAT_NV12:
            case PIXEL_FORMAT_NV21:
            case PIXEL_FORMAT_YUV420P:
            case PIXEL_FORMAT_YV12:
                // Semi-planar: Y + interleaved UV
                inBuffer.pAddrVirY = base;
                inBuffer.pAddrVirC = base + width * height;
                inBuffer.pAddrPhyY = SunxiMemGetPhysicAddressCpu(in_frame->memOps,base);
                inBuffer.pAddrPhyC = inBuffer.pAddrPhyY + width * height;
                break;
            // Packed RGB formats — single plane
            case PIXEL_FORMAT_RGB565:
            case PIXEL_FORMAT_RGB888:
            case PIXEL_FORMAT_BGR888:
            case PIXEL_FORMAT_ARGB8888:
            case PIXEL_FORMAT_ABGR8888:
            case PIXEL_FORMAT_RGBA8888:
                inBuffer.pAddrVirY = base;
                inBuffer.pAddrPhyY = SunxiMemGetPhysicAddressCpu(in_frame->memOps,base);
                break;

            default:
                LOG_ERROR("Unsupported pixel format: %d", in_frame->format);
                break;
        }
        // Encode the frame
        size_t encoded_size = 0;
        inBuffer.nID = 0;
        //inBuffer.nPts = in_frame->pts;
        inBuffer.nWidth = in_frame->width;
        inBuffer.nHeight = in_frame->height;
        inBuffer.nAlign = 16;
        VencOutputBuffer* outputBuf = (VencOutputBuffer*)malloc(sizeof(VencOutputBuffer));

        bool encode_success = jpeg_encoder_encode(priv->encoder, &inBuffer, outputBuf, &encoded_size);
        if (encode_success && encoded_size > 0) {
            // Create output FrameBuffer for encoded data
            FrameBuffer* out_frame = (FrameBuffer*)calloc(1, sizeof(FrameBuffer));
            if (out_frame) {
                // Allocate memory for encoded  data
                out_frame->size = encoded_size;

                out_frame->pts = outputBuf->nPts;
                //out_frame->memOps = priv->memOps;
                out_frame->ref_count = 0;
                out_frame->format = PIXEL_FORMAT_JPEG;
                out_frame->size = encoded_size;
                out_frame->release = encoder_output_release;
                out_frame->user_data = priv;
                out_frame->nFlag = outputBuf->nFlag;
                out_frame->buf_index = in_frame->buf_index;
                out_frame->user_data2 = outputBuf;
                // Copy encoded data from encoder's output buffer
                if (outputBuf->nSize0 > 0 && outputBuf->pData0) {
                    //LOG_DEBUG("memcpy outputBuf.pData0");
                    out_frame->data = outputBuf->pData0;
                    //memcpy(out_frame->data, outputBuf.pData0, outputBuf.nSize0);
                }
                if (outputBuf->nSize1 > 0 && outputBuf->pData1) {
                    LOG_DEBUG("memcpy outputBuf.pData1");
                    memcpy(out_frame->data + outputBuf->nSize0,
                            outputBuf->pData1, outputBuf->nSize1);
                }
                //FreeOneBitStreamFrame(priv->encoder->pVideoEnc,&outputBuf);

                // Release input frame
                frame_put(in_frame);
                // Push encoded frame to output queue
                // update_fps_encoder();

                if(index == 1){
                    ref_frame = init_checker(out_frame->size);
                }

                if(check_frame(out_frame->data, ref_frame, out_frame->size, index++)){
                    printf("[ERROR]第 %d 帧与基准帧不一致, %s 输出异常\n", index, self->name);
                }else{
                    printf("%s: 第 %d 帧与基准帧一致\n", self->name, index);
                }

                if(frame_queue_push(&priv->queue_out, out_frame, &priv->running))
                {
                    encoder_output_release(out_frame, priv);

                }
                    //LOG_INFO("Encoded frame push success");
            }else{
                FreeOneBitStreamFrame(priv->encoder->pVideoEnc, outputBuf);
                free(outputBuf);
                outputBuf = NULL;
            }
        } else {
            LOG_ERROR("Encoding failed or no output data");
            frame_put(in_frame);
            FreeOneBitStreamFrame(priv->encoder->pVideoEnc, outputBuf);
            free(outputBuf);
            outputBuf = NULL;
        }

    }

    release_checker(ref_frame);
    printf("Encoder thread stopped\n");

    return NULL;
}

VENC_PIXEL_FMT map_pixel_format(PixelFormat fmt) {
    switch (fmt) {
        case PIXEL_FORMAT_NV12:
            return VENC_PIXEL_YUV420SP;
        case PIXEL_FORMAT_NV21:
            return VENC_PIXEL_YVU420SP;
        case PIXEL_FORMAT_YUV420P:
            return VENC_PIXEL_YUV420P;
        case PIXEL_FORMAT_YV12:
            return VENC_PIXEL_YVU420P;
        case PIXEL_FORMAT_RGBA8888:
            return VENC_PIXEL_RGBA;
        case PIXEL_FORMAT_ARGB8888:
            return VENC_PIXEL_ARGB;
        case PIXEL_FORMAT_ABGR8888:
            return VENC_PIXEL_ABGR;

        // RGB formats not directly supported: map or return a default
        case PIXEL_FORMAT_RGB565:
        case PIXEL_FORMAT_RGB888:
        case PIXEL_FORMAT_BGR888:
        case PIXEL_FORMAT_GRAY8:
            return -1;
        default:
            return VENC_PIXEL_YUV420SP;  // 默认或 fallback 格式
    }
}


/**
 * @brief Initialize the encoder plugin
 *
 * Sets up the encoder, memory operations, buffer pools,
 * and other necessary components for encoding operation.
 *
 * @param self Pointer to the plugin instance
 * @param config Configuration parameters
 * @return 0 on success, -1 on failure
 */
static int encoder_init(Plugin* self, void* config) {
    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;

    encConfig* enc_config = (encConfig*) config;

    LOG_DEBUG("Initializing encoder plugin");

    encoderConfig encoder_config;
    encoder_config.src_width = enc_config->input_width;
    encoder_config.src_height = enc_config->input_height;
    encoder_config.fps = enc_config->input_fps;

    encoder_config.input_format = map_pixel_format(enc_config->input_format);
    if(encoder_config.input_format < 0)
    {
        LOG_ERROR("Encoder don't support this pixel format");
        exit(-1);
    }

    priv->memOps = GetMemAdapterOpsS();
    SunxiMemOpen(priv->memOps);


    // Initialize output queue
    frame_queue_init(&priv->queue_out);
    priv->running = 0;

    // Initialize memory operations for DMA allocation

    // Initialize Encoder
    priv->encoder = (JPEGEncoder*)calloc(1, sizeof(JPEGEncoder));
    if (!priv->encoder) {
        LOG_ERROR("Failed to allocate JPEGEncoder");
        return -1;
    }

    // Initialize the encoder
    if (!jpeg_encoder_init(priv->encoder, &encoder_config)) {
        LOG_ERROR("Failed to initialize JPEGEncoder");
        return -1;
    }

    LOG_INFO("Encoder plugin initialized successfully");
    return 0;
}

/**
 * @brief Start the encoder plugin
 *
 * Creates and starts the encoder thread for processing frames.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success, -1 on failure
 */
static int encoder_start(Plugin* self) {
    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;

    printf("Starting encoder plugin\n");
    priv->running = 1;
    init_pts_base();
    int ret = pthread_create(&priv->thread, NULL, encoder_thread_loop, self);
    if (ret != 0) {
        printf("Failed to create encoder thread\n");
        priv->running = 0;
        return -1;
    }

    printf("Encoder plugin started successfully\n");
    return 0;
}

/**
 * @brief Stop the encoder plugin
 *
 * Stops the encoder thread and waits for it to complete.
 *
 * @param self Pointer to the plugin instance
 * @return 0 on success
 */
static int encoder_stop(Plugin* self) {
    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;

    printf("Stopping encoder plugin\n");
    priv->running = 0;

    pthread_cond_broadcast(&priv->queue_out.cond_nonempty);
    pthread_cond_broadcast(&priv->queue_out.cond_nonfull);
    // Wait for thread to complete
    pthread_join(priv->thread, NULL);

    printf("Encoder plugin stopped\n");
    return 0;
}

/**
 * @brief Process function (not used in pull-based design)
 *
 * This function is not used as the encoder follows a pull-based
 * data flow pattern where it actively retrieves frames from upstream.
 *
 * @param self Pointer to the plugin instance
 * @param frame Input frame buffer
 * @return 0 (always success, but not used)
 */
static int encoder_process(Plugin* self, FrameBuffer* frame) {
    // Not used in pull-based design
    return 0;
}

/**
 * @brief Push function (not used as encoder is not a sink)
 *
 * @param self Pointer to the plugin instance
 * @param frame Input frame buffer
 * @return 0 (not implemented)
 */
static int encoder_push(Plugin* self, FrameBuffer* frame) {
    // Not used - encoder pulls from upstream
    return 0;
}

/**
 * @brief Pull encoded frames from the encoder
 *
 * Returns the next available encoded frame from the output queue.
 * The caller is responsible for calling frame_put() to release the frame.
 *
 * @param self Pointer to the plugin instance
 * @return Pointer to encoded FrameBuffer, or NULL if no frame available
 */
static FrameBuffer* encoder_pull(Plugin* self) {
    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;
    return frame_queue_pop(&priv->queue_out);
}

/**
 * @brief Destroy the encoder plugin and free resources
 *
 * Cleans up all allocated resources including encoder, buffers,
 * and memory operations.
 *
 * @param self Pointer to the plugin instance
 */
static void encoder_destroy(Plugin* self) {
    if (!self || !self->priv) return;

    EncoderPluginPriv* priv = (EncoderPluginPriv*)self->priv;

    LOG_INFO("Destroying encoder plugin\n");

    // 确保线程已停止
    if (priv->running) {
        self->stop(self);
    }

    // Destroy output queue
    // output_pool_destroy(&priv->output_pool);
    // FrameBuffer* frame = NULL;
    // while ((frame = frame_queue_pop_timeout(&priv->queue_out,100))) {
    //     FreeOneBitStreamFrame(priv->encoder->pVideoEnc, frame);

    // }

    frame_queue_destroy(&priv->queue_out);
    // Destroy encoder
    if (priv->encoder) {
        jpeg_encoder_destroy(priv->encoder);
        free(priv->encoder);
        priv->encoder = NULL;
    }

    if(priv->memOps)
        SunxiMemClose(priv->memOps);
    priv->memOps = NULL;

    free(priv);
    self->priv = NULL;
    printf("Encoder plugin destroyed\n");
}


/**
 * @brief Create a new encoder plugin instance
 *
 * Allocates and initializes a new encoder plugin with default configuration.
 * The plugin follows the standard MediaPipeX plugin interface.
 *
 * @param name Optional name for the plugin instance
 * @return Pointer to the created plugin, or NULL on failure
 */
Plugin* create_encoder_plugin(const char* name) {
    // Allocate plugin structure
    Plugin* plugin = (Plugin*)calloc(1, sizeof(Plugin));
    if (!plugin) {
        printf("Failed to allocate Plugin structure\n");
        return NULL;
    }

    // Allocate private data
    EncoderPluginPriv* priv = (EncoderPluginPriv*)calloc(1, sizeof(EncoderPluginPriv));
    if (!priv) {
        printf("Failed to allocate EncoderPluginPriv structure\n");
        free(plugin);
        return NULL;
    }

    // Set plugin properties
    plugin->name = name ? strdup(name) : strdup("EncoderPlugin");
    plugin->init = encoder_init;
    plugin->start = encoder_start;
    plugin->stop = encoder_stop;
    plugin->process = encoder_process;
    plugin->push = encoder_push;
    plugin->pull = encoder_pull;
    plugin->priv = priv;

    printf("Created encoder plugin: %s\n", plugin->name);
    return plugin;
}


void destroy_encoder_plugin(Plugin* self){
    if (self) {
        if (self->priv) {
            encoder_destroy(self);
        }
        if (self->name) {
            free(self->name);
            self->name = NULL;
        }
        free(self);
        self = NULL;
    }
}
