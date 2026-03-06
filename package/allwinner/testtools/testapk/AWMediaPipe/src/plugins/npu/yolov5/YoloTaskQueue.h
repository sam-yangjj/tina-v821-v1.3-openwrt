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
#ifndef YOLO_TASK_QUEUE_H
#define YOLO_TASK_QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "frame_queue.h"

// 推理完成回调函数类型
typedef void (*InferenceCompleteCallback)(FrameBuffer* frame, void* user_data);

// 推理任务结构体
typedef struct YoloTask {
    // 图像数据
    int width;                   // 图像宽度
    int height;                  // 图像高度

    // DMA零拷贝相关
    // int dma_fd;                  // DMA buffer fd
    // void* dma_virt;              // 虚拟地址
    // void* dma_phy;               // 物理地址
    // unsigned int dma_length;     // buffer长度
    FrameBuffer* frame;

    int buffer_index;            // buffer池索引

    // 推理完成回调
    InferenceCompleteCallback inference_callback;
    void* inference_user_data;   // 回调用户数据
    void* frame_user_data;
} YoloTask;

// 线程安全的环形任务队列
#define YOLO_TASK_QUEUE_CAPACITY 16

typedef struct YoloTaskQueue {
    YoloTask* tasks[YOLO_TASK_QUEUE_CAPACITY];
    int head;
    int tail;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} YoloTaskQueue;

// 初始化/销毁
void yolo_task_queue_init(YoloTaskQueue* queue);
void yolo_task_queue_destroy(YoloTaskQueue* queue);
// 入队/出队
bool yolo_task_queue_push(YoloTaskQueue* queue, const YoloTask* task, int* running_flag);
bool yolo_task_queue_pop(YoloTaskQueue* queue, YoloTask** task, int* running_flag);
//bool yolo_task_queue_pop(YoloTaskQueue* queue, YoloTask** task);
// 查询队列是否为空/满
bool yolo_task_queue_is_empty(YoloTaskQueue* queue);
bool yolo_task_queue_is_full(YoloTaskQueue* queue);

#endif // YOLO_TASK_QUEUE_H
