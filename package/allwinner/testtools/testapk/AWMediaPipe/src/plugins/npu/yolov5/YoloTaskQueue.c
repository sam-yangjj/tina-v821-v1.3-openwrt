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
#include "YoloTaskQueue.h"
#include <string.h>

void yolo_task_queue_init(YoloTaskQueue* queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
}

void yolo_task_queue_destroy(YoloTaskQueue* queue) {

    for (int i = 0; i < queue->size; i++) {
        int index = (queue->head + i) % YOLO_TASK_QUEUE_CAPACITY;
        if(queue->tasks[index]){
            free(queue->tasks[index]); // 如果 task 是动态分配的
            queue->tasks[index] = NULL;
        }
    }
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
}

bool yolo_task_queue_is_empty(YoloTaskQueue* queue) {
    return queue->size == 0;
}

bool yolo_task_queue_is_full(YoloTaskQueue* queue) {
    return queue->size == YOLO_TASK_QUEUE_CAPACITY;
}

// 入队，满时阻塞，成功返回true
// bool yolo_task_queue_push(YoloTaskQueue* queue, const YoloTask* task) {
//     pthread_mutex_lock(&queue->mutex);
//     while (queue->size == YOLO_TASK_QUEUE_CAPACITY) {
//         pthread_cond_wait(&queue->not_full, &queue->mutex);
//     }
//     queue->tasks[queue->tail] = task;
//     queue->tail = (queue->tail + 1) % YOLO_TASK_QUEUE_CAPACITY;
//     queue->size++;
//     pthread_cond_signal(&queue->not_empty);
//     pthread_mutex_unlock(&queue->mutex);
//     return true;
// }

// 出队，空时阻塞，成功返回true
// bool yolo_task_queue_pop(YoloTaskQueue* queue, YoloTask** task) {
//     pthread_mutex_lock(&queue->mutex);
//     while (queue->size == 0) {
//         pthread_cond_wait(&queue->not_empty, &queue->mutex);
//     }
//     *task = queue->tasks[queue->head];
//     queue->head = (queue->head + 1) % YOLO_TASK_QUEUE_CAPACITY;
//     queue->size--;
//     pthread_cond_signal(&queue->not_full);
//     pthread_mutex_unlock(&queue->mutex);
//     return true;
// }


bool yolo_task_queue_push(YoloTaskQueue* queue, const YoloTask* task, int* running_flag) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == YOLO_TASK_QUEUE_CAPACITY) {
        if (!(*running_flag)) {
            pthread_mutex_unlock(&queue->mutex);
            return false;  // 退出线程的信号
        }
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    queue->tasks[queue->tail] = task;
    queue->tail = (queue->tail + 1) % YOLO_TASK_QUEUE_CAPACITY;
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    return true;
}
bool yolo_task_queue_pop(YoloTaskQueue* queue, YoloTask** task, int* running_flag) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0) {
        if (!(*running_flag)) {
            pthread_mutex_unlock(&queue->mutex);
            return false;  // 退出线程的信号
        }
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    *task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % YOLO_TASK_QUEUE_CAPACITY;
    queue->size--;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return true;
}

