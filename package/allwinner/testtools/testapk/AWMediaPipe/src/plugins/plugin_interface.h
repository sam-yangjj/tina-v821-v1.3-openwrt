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
#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "ion_mem_alloc.h"


#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#define ALIGN_16B(x) (((x) + (15)) & ~(15))

typedef enum {
    PIXEL_FORMAT_UNKNOWN = 0,

    PIXEL_FORMAT_NV21,        // Semi-planar YUV 4:2:0
    PIXEL_FORMAT_NV12,
    PIXEL_FORMAT_YV12,        // Planar YUV 4:2:0
    PIXEL_FORMAT_YUV420P,
    PIXEL_FORMAT_YUV422P,

    PIXEL_FORMAT_RGB565,
    PIXEL_FORMAT_RGB888,
    PIXEL_FORMAT_BGR888,
    PIXEL_FORMAT_RGBA8888,
    PIXEL_FORMAT_ARGB8888,
    PIXEL_FORMAT_ABGR8888,

    PIXEL_FORMAT_GRAY8,

    PIXEL_FORMAT_H264,

    PIXEL_FORMAT_MAX,

    PIXEL_FORMAT_NONE
} PixelFormat;

size_t get_image_size(PixelFormat fmt, int width, int height);


// Frame buffer structure for zero-copy DMA data, with reference counting
typedef struct FrameBuffer {
    void* data;          // Virtual address of DMA buffer
    unsigned char *Vir_C_addr;
	unsigned char *Phy_Y_addr;
	unsigned char *Phy_C_addr;
    int fd;
    int buf_index;
    unsigned int   nFlag; // key frame
    int width;
    int height;
    int size;            // Size of data in bytes
    PixelFormat format;
    uint64_t pts;        // Presentation timestamp
    unsigned char* sps_pps_data;
    int sps_pps_data_len;
    int ref_count;       // Reference count
    pthread_mutex_t ref_mutex;
    struct SunxiMemOpsS* memOps;
    void (*release)(struct FrameBuffer* frame, void* user_data); // 回调函数
    void* user_data;
    void* user_data2;
} FrameBuffer;

// Increase reference count
static inline void frame_get(FrameBuffer* frame) {
    if (!frame) return;
    pthread_mutex_lock(&frame->ref_mutex);
    frame->ref_count++;
    pthread_mutex_unlock(&frame->ref_mutex);
}

// Decrease reference count and free if zero
static inline void frame_put(FrameBuffer* frame) {
    if (!frame) return;
    int do_free = 0;
    pthread_mutex_lock(&frame->ref_mutex);
    frame->ref_count--;
    if (frame->ref_count <= 0) {
        frame->ref_count = 0;
        do_free = 1;
    }
    pthread_mutex_unlock(&frame->ref_mutex);

    if (do_free) {
        pthread_mutex_destroy(&frame->ref_mutex);
        if(frame->release){
            frame->release(frame,frame->user_data);
        }else{
             // TODO: Free DMA buffer or recycle via platform-specific method
            if(frame->data){
                SunxiMemPfree(frame->memOps,frame->data);
                frame->data = NULL;
            }
            if(frame->fd > 0){
                close(frame->fd);
                frame->fd = -1;
            }
            free(frame);
        }
    }
}

// Forward declaration of Plugin struct
struct Plugin;

typedef struct Plugin {
    char* name;

    // Initialize the plugin, optional config string
    int (*init)(struct Plugin* self, void* config);

    // Start plugin thread or processing
    int (*start)(struct Plugin* self);

    // Stop plugin thread and cleanup
    int (*stop)(struct Plugin* self);

    // Receive a frame pushed from upstream plugin
    int (*process)(struct Plugin* self, FrameBuffer* frame);

    int (*push)(struct Plugin* self, FrameBuffer* fb);  // 下游用

    FrameBuffer* (*pull)(struct Plugin* plugin);

    // Plugin private data
    void* priv;

    // Thread ID managed by the plugin
    pthread_t thread_id;

    // Downstream plugin pointer
    struct Plugin* downstream;

    struct Plugin* upstream;

} Plugin;

int plugin_link(Plugin* upstream, Plugin* downstream);

int plugin_link_many(Plugin* first, ...);

int plugin_chain_start(Plugin* first);

int plugin_chain_stop(Plugin* first);

void pipeline_main_loop();


#ifdef __cplusplus
}
#endif

#endif // PLUGIN_INTERFACE_H
