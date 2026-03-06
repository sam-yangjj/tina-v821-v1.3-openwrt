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
#include "buffer_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
void output_pool_init(OutputBufferPool* pool, int num_buffers,
                      int width, int height, PixelFormat format, struct SunxiMemOpsS* mem_ops) {
    if (pool->initialized) return;

    pthread_mutex_init(&pool->lock, NULL);
    pool->count = (num_buffers > MAX_OUTPUT_BUFFERS) ? MAX_OUTPUT_BUFFERS : num_buffers;
    pool->width = width;
    pool->height = height;
    pool->format = format;
    pool->mem_ops = mem_ops;
    pool->initialized = 1;

    for (int i = 0; i < pool->count; ++i) {
        FrameBuffer* buf = (FrameBuffer*)calloc(1, sizeof(FrameBuffer));
        buf->width = width;
        buf->height = height;
        buf->format = format;
        buf->size = get_image_size(format, width, height);

        buf->data = SunxiMemPalloc(mem_ops, buf->size);
        if (!buf->data) {
            fprintf(stderr, "SunxiMemPalloc failed for buffer %d\n", i);
            free(buf);
            exit(-1);
        }

        buf->fd = SunxiMemGetBufferFd(mem_ops, buf->data);
        buf->Phy_Y_addr = SunxiMemGetPhysicAddressCpu(mem_ops, buf->data);
        buf->memOps = mem_ops;
        buf->ref_count = 0;
        buf->release = NULL;

        pool->buffers[i] = buf;
        pool->available[i] = 1;
    }
}

FrameBuffer* output_pool_get(OutputBufferPool* pool) {
    FrameBuffer* result = NULL;

    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < pool->count; ++i) {
        if (pool->available[i]) {
            pool->available[i] = 0;
            result = pool->buffers[i];
            break;
        }
    }
    pthread_mutex_unlock(&pool->lock);

    return result;
}

void output_pool_release(OutputBufferPool* pool, FrameBuffer* buffer) {
    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < pool->count; ++i) {
        if (pool->buffers[i] == buffer) {
            pool->available[i] = 1;
            break;
        }
    }
    pthread_mutex_unlock(&pool->lock);
}

void output_pool_destroy(OutputBufferPool* pool) {
    for (int i = 0; i < pool->count; ++i) {
        if (!pool->available[i]) {
            fprintf(stderr, "Warning: destroying pool with outstanding buffer %d still in use!\n", i);
            //frame_put(pool->buffers[i]);
        }
        FrameBuffer* buf = pool->buffers[i];
        if (buf) {
            // 先保存fd，再释放buf
            int fd = buf->fd;
            if (buf->data) {
                SunxiMemPfree(pool->mem_ops, buf->data);
                buf->data = NULL;
            }
            // 关闭fd在释放buf之后
            if (fd > 0) {
                close(fd);
                fd = -1;
            }

            free(buf);
            pool->buffers[i] = NULL;
            buf = NULL;
        }
    }
    pthread_mutex_destroy(&pool->lock);
    pool->initialized = 0;
}
