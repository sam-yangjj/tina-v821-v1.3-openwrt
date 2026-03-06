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
#include "frame_min_heap.h"
#include <stdlib.h>

static void heapify_up(FrameMinHeap* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->data[parent]->frame_index <= heap->data[idx]->frame_index)
            break;
        InferenceResult* tmp = heap->data[parent];
        heap->data[parent] = heap->data[idx];
        heap->data[idx] = tmp;
        idx = parent;
    }
}

static void heapify_down(FrameMinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->data[left]->frame_index < heap->data[smallest]->frame_index)
        smallest = left;
    if (right < heap->size && heap->data[right]->frame_index < heap->data[smallest]->frame_index)
        smallest = right;

    if (smallest != idx) {
        InferenceResult* tmp = heap->data[idx];
        heap->data[idx] = heap->data[smallest];
        heap->data[smallest] = tmp;
        heapify_down(heap, smallest);
    }
}

FrameMinHeap* heap_create(int capacity) {
    FrameMinHeap* heap = malloc(sizeof(FrameMinHeap));
    heap->data = (InferenceResult*)calloc(1,sizeof(InferenceResult*) * capacity);
    heap->size = 0;
    heap->capacity = capacity;
    pthread_mutex_init(&heap->mutex, NULL);
    pthread_cond_init(&heap->cond, NULL);
    return heap;
}

void heap_destroy(FrameMinHeap* heap) {
    if (!heap) return;

    pthread_mutex_lock(&heap->mutex);
    for (int i = 0; i < heap->size; ++i) {
        free(heap->data[i]);
    }
    pthread_mutex_unlock(&heap->mutex);

    free(heap->data);
    pthread_mutex_destroy(&heap->mutex);
    pthread_cond_destroy(&heap->cond);
    free(heap);
}

int heap_push(FrameMinHeap* heap, InferenceResult* res) {
    pthread_mutex_lock(&heap->mutex);

    if (heap->size >= heap->capacity) {
        pthread_mutex_unlock(&heap->mutex);
        return -1;
    }

    heap->data[heap->size] = res;
    heapify_up(heap, heap->size);
    heap->size++;

    pthread_cond_signal(&heap->cond);
    pthread_mutex_unlock(&heap->mutex);
    return 0;
}

InferenceResult* heap_pop(FrameMinHeap* heap) {
    pthread_mutex_lock(&heap->mutex);

    if (heap->size == 0) {
        pthread_mutex_unlock(&heap->mutex);
        return NULL;
    }

    InferenceResult* top = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    heapify_down(heap, 0);

    pthread_mutex_unlock(&heap->mutex);
    return top;
}

InferenceResult* heap_peek(FrameMinHeap* heap) {
    pthread_mutex_lock(&heap->mutex);
    InferenceResult* top = heap->size > 0 ? heap->data[0] : NULL;
    pthread_mutex_unlock(&heap->mutex);
    return top;
}

int heap_is_empty(FrameMinHeap* heap) {
    pthread_mutex_lock(&heap->mutex);
    int empty = (heap->size == 0);
    pthread_mutex_unlock(&heap->mutex);
    return empty;
}

int heap_size(FrameMinHeap* heap) {
    pthread_mutex_lock(&heap->mutex);
    int size = heap->size;
    pthread_mutex_unlock(&heap->mutex);
    return size;
}
