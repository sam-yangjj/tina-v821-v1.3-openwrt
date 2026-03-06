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
#include "YoloThreadPool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "../../utils/log.h"

#define MAX_DETECTIONS 100

typedef struct {
    YoloThreadPool* pool;
    int thread_idx;
} ThreadArg;

void yolo_thread_pool_init(YoloThreadPool* pool, YoloTaskQueue* queue, const char* model_path, int thread_count) {
    pool->queue = queue;
    pool->thread_count = thread_count > YOLO_THREAD_POOL_SIZE ? YOLO_THREAD_POOL_SIZE : thread_count;
    pool->running = 0;
    if (model_path) {
        strncpy(pool->model_path, model_path, sizeof(pool->model_path)-1);
        pool->model_path[sizeof(pool->model_path)-1] = '\0';
    }

    awnn_init();
    // 为每个线程创建独立的yoloNet对象
    for (int i = 0; i < pool->thread_count; ++i) {
        yolov5_create(&pool->yoloNets[i], pool->model_path);
        pool->yoloNets[i].init(&pool->yoloNets[i]);
    }
}
pthread_mutex_t npu_mutex = PTHREAD_MUTEX_INITIALIZER;
void* yolo_thread_func(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    YoloThreadPool* pool = thread_arg->pool;
    int thread_idx = thread_arg->thread_idx;
    free(thread_arg); // 参数只用一次，释放内存

    YoloV5Net* yoloNet = &pool->yoloNets[thread_idx];
    Object results[MAX_DETECTIONS];
    int res_num = 0;

    while (pool->running) {
        YoloTask *task = NULL;
        if (!yolo_task_queue_pop(pool->queue, &task, &pool->running)) {
            break;  // 退出线程
        }

        if (task->frame && task->frame->data) {
            // YOLO推理
            // 在 run 前后加锁
            pthread_mutex_lock(&npu_mutex);
            res_num = yoloNet->run(yoloNet, (unsigned char*)task->frame->data, task->width, task->height, results);
            pthread_mutex_unlock(&npu_mutex);

            // 绘制检测结果
            if (res_num > 0) {
                LOG_INFO("draw_results_on_memory");
                draw_results_on_memory((unsigned char*)task->frame->data, task->width, task->height, 3, results, res_num, yoloNet->fontCtx);
                LOG_INFO("Drew %d detections on image\n", res_num);
            }

            // 调用回调
            if (task->inference_callback) {
                task->inference_callback(task->frame, task->inference_user_data);
            }
        }

        free(task); // 释放task结构体
        task = NULL;
    }

    return NULL;
}

void yolo_thread_pool_start(YoloThreadPool* pool) {
    pool->running = 1;
    for (int i = 0; i < pool->thread_count; ++i) {
        ThreadArg* arg = (ThreadArg*)malloc(sizeof(ThreadArg));
        arg->pool = pool;
        arg->thread_idx = i;
        pthread_create(&pool->thread_ids[i], NULL, yolo_thread_func, arg);
    }
}

void yolo_thread_pool_stop(YoloThreadPool* pool) {
    pool->running = 0;
    // 唤醒所有等待线程
    for (int i = 0; i < pool->thread_count; ++i) {
        pthread_cond_broadcast(&pool->queue->not_full);
        pthread_cond_broadcast(&pool->queue->not_empty);
    }

    for (int i = 0; i < pool->thread_count; ++i) {
        pthread_join(pool->thread_ids[i], NULL);
    }
}

void yolo_thread_pool_destroy(YoloThreadPool* pool) {
    for (int i = 0; i < pool->thread_count; ++i) {
        pool->yoloNets[i].destroy(&pool->yoloNets[i]);
    }
    awnn_uninit();
}
