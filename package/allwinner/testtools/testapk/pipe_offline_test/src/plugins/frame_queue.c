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
#include "frame_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
int frame_queue_init(FrameQueue* fq) {
    if (!fq) return -1;

    fq->head = 0;
    fq->tail = 0;
    fq->size = 0;

    if (pthread_mutex_init(&fq->mutex, NULL) != 0) {
        perror("Failed to init mutex");
        return -1;
    }
    if (pthread_cond_init(&fq->cond_nonempty, NULL) != 0) {
        perror("Failed to init cond_nonempty");
        pthread_mutex_destroy(&fq->mutex);
        return -1;
    }
    if (pthread_cond_init(&fq->cond_nonfull, NULL) != 0) {
        perror("Failed to init cond_nonfull");
        pthread_cond_destroy(&fq->cond_nonempty);
        pthread_mutex_destroy(&fq->mutex);
        return -1;
    }

    return 0;
}

void frame_queue_destroy(FrameQueue* fq) {
    if (!fq) return;

    // 清空队列中的所有frame
    frame_queue_clear(fq);

    fq->head = 0;
    fq->tail = 0;
    pthread_mutex_destroy(&fq->mutex);
    pthread_cond_destroy(&fq->cond_nonempty);
    pthread_cond_destroy(&fq->cond_nonfull);
    // 不要free(fq)，因为它通常是嵌入在其他结构中的
}

// Push a frame into the queue, block if queue is full
int frame_queue_push(FrameQueue* fq, FrameBuffer* frame, int* running_flag) {
    if (!fq || !frame) return -1;

    pthread_mutex_lock(&fq->mutex);
    while (fq->size == FRAME_QUEUE_MAX_SIZE) {
        // 如果线程需要退出，就提前返回
        if (!(*running_flag)) {
            pthread_mutex_unlock(&fq->mutex);
            return -1;
        }
        pthread_cond_wait(&fq->cond_nonfull, &fq->mutex);
    }

    // 插入数据
    fq->queue[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_MAX_SIZE;
    fq->size++;

    // 通知队列非空
    pthread_cond_signal(&fq->cond_nonempty);
    pthread_mutex_unlock(&fq->mutex);

    return 0;
}
int frame_queue_push_timeout(FrameQueue* fq, FrameBuffer* frame, int timeout_ms) {
    if (!fq || !frame) return -1;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // 加上超时时间（毫秒转秒和纳秒）
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    pthread_mutex_lock(&fq->mutex);

    // 等待直到队列不满或超时
    while (fq->size == FRAME_QUEUE_MAX_SIZE) {
        int ret = pthread_cond_timedwait(&fq->cond_nonfull, &fq->mutex, &ts);
        if (ret == ETIMEDOUT) {
            pthread_mutex_unlock(&fq->mutex);
            return -2; // 超时
        }
    }

    // 插入帧并更新状态
    fq->queue[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_MAX_SIZE;
    fq->size++;

    // 通知等待线程队列非空
    pthread_cond_signal(&fq->cond_nonempty);
    pthread_mutex_unlock(&fq->mutex);

    return 0;
}
int frame_queue_push_force(FrameQueue* fq, FrameBuffer* frame) {
    if (!fq || !frame) return -1;

    pthread_mutex_lock(&fq->mutex);

    // 如果队列已满，先移除队头元素
    if (fq->size == FRAME_QUEUE_MAX_SIZE) {
        FrameBuffer* old = fq->queue[fq->head];
        if (old) {
            if(old->ref_count == 0)
            {
                if(frame->release){
                    frame->release(frame,frame->user_data);
                }else{
                    free_frame_buffer(old);
                }
            }
        }
        fq->head = (fq->head + 1) % FRAME_QUEUE_MAX_SIZE;
        fq->size--;
    }

    // 插入新帧
    fq->queue[fq->tail] = frame;
    fq->tail = (fq->tail + 1) % FRAME_QUEUE_MAX_SIZE;
    fq->size++;

    // 通知有新帧
    pthread_cond_signal(&fq->cond_nonempty);
    pthread_mutex_unlock(&fq->mutex);

    return 0;
}


// Pop a frame from the queue, block if queue is empty
FrameBuffer* frame_queue_pop(FrameQueue* fq) {
    if (!fq) return NULL;

    pthread_mutex_lock(&fq->mutex);

    // Wait until queue is not empty
    while (fq->size == 0) {
        pthread_cond_wait(&fq->cond_nonempty, &fq->mutex);
    }

    // Remove the frame and update queue state
    FrameBuffer* frame = fq->queue[fq->head];
    fq->head = (fq->head + 1) % FRAME_QUEUE_MAX_SIZE;
    fq->size--;

    // Signal that queue is not full
    pthread_cond_signal(&fq->cond_nonfull);
    pthread_mutex_unlock(&fq->mutex);

    return frame;
}

FrameBuffer* frame_queue_pop_timeout(FrameQueue* fq, int timeout_ms) {
    if (!fq) return NULL;

    pthread_mutex_lock(&fq->mutex);

    // 获取当前时间，准备构造超时时间
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // 添加超时时间
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;

    // 处理纳秒溢出
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += ts.tv_nsec / 1000000000;
        ts.tv_nsec %= 1000000000;
    }

    // 等待直到 queue 非空 或者超时
    int ret = 0;
    while (fq->size == 0 && ret != ETIMEDOUT) {
        ret = pthread_cond_timedwait(&fq->cond_nonempty, &fq->mutex, &ts);
    }

    FrameBuffer* frame = NULL;
    if (fq->size > 0) {
        frame = fq->queue[fq->head];
        fq->head = (fq->head + 1) % FRAME_QUEUE_MAX_SIZE;
        fq->size--;

        // 通知生产者
        pthread_cond_signal(&fq->cond_nonfull);
    }

    pthread_mutex_unlock(&fq->mutex);
    return frame;  // 如果超时返回 NULL
}

void frame_queue_clear(FrameQueue* fq) {
    if (!fq) return;

    pthread_mutex_lock(&fq->mutex);

    while (fq->size > 0) {
        FrameBuffer* frame = fq->queue[fq->head];

        // 使用引用计数机制安全释放frame，而不是直接释放内存
        if(frame) {
            frame_put(frame); // 减少引用计数，让frame自己决定是否释放
        }

        fq->head = (fq->head + 1) % FRAME_QUEUE_MAX_SIZE;
        fq->size--;
    }

    // 重置指针状态
    fq->head = 0;
    fq->tail = 0;

    // 唤醒所有等待的线程，避免死锁
    pthread_cond_broadcast(&fq->cond_nonempty);
    pthread_cond_broadcast(&fq->cond_nonfull);

    pthread_mutex_unlock(&fq->mutex);
}

void free_frame_buffer(FrameBuffer* frame) {
    if (!frame) return;

    if (frame->data) {
        SunxiMemPfree(frame->memOps, frame->data);
        frame->data = NULL;
    }

    if (frame->fd > 0) {
        close(frame->fd);
        frame->fd = -1;
    }

    free(frame);
}

