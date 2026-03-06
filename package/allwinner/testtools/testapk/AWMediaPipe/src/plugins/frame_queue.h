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
#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <pthread.h>
#include "plugin_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_QUEUE_MAX_SIZE 5

// Thread-safe frame queue for producer-consumer model
typedef struct FrameQueue {
    FrameBuffer* queue[FRAME_QUEUE_MAX_SIZE];
    int head;
    int tail;
    int size;

    pthread_mutex_t mutex;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;

} FrameQueue;

// Initialize the frame queue
int frame_queue_init(FrameQueue* fq);

// Destroy the frame queue, release resources
void frame_queue_destroy(FrameQueue* fq);

// Push a frame into the queue, block if full
int frame_queue_push(FrameQueue* fq, FrameBuffer* frame, int* running_flag);

int frame_queue_push_timeout(FrameQueue* fq, FrameBuffer* frame, int timeout_ms);

int frame_queue_push_force(FrameQueue* fq, FrameBuffer* frame);

// Pop a frame from the queue, block if empty
FrameBuffer* frame_queue_pop(FrameQueue* fq);

FrameBuffer* frame_queue_pop_timeout(FrameQueue* fq, int timeout_ms);

void frame_queue_clear(FrameQueue* fq);

void free_frame_buffer(FrameBuffer* frame);

#ifdef __cplusplus
}
#endif

#endif // FRAME_QUEUE_H
