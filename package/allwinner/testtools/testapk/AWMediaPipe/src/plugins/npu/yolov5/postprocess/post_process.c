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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "post_process.h"

#define CLASSES 80
#define ANCHORS 3
#define MAX_PROPOSALS 1000

static const char* class_names[80] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

static float sigmoid(float x) {
    return 1.f / (1.f + expf(-x));
}

static float desigmoid(float x) {
    return -logf(1.f / x - 1.f);
}

static float intersection_area(const Object *a, const Object *b) {
    float x1 = fmaxf(a->x, b->x);
    float y1 = fmaxf(a->y, b->y);
    float x2 = fminf(a->x + a->width, b->x + b->width);
    float y2 = fminf(a->y + a->height, b->y + b->height);

    float w = fmaxf(0.f, x2 - x1);
    float h = fmaxf(0.f, y2 - y1);
    return w * h;
}

static void qsort_desc(Object *objs, int left, int right) {
    int i = left, j = right;
    float pivot = objs[(left + right) / 2].prob;
    while (i <= j) {
        while (objs[i].prob > pivot) i++;
        while (objs[j].prob < pivot) j--;
        if (i <= j) {
            Object tmp = objs[i];
            objs[i] = objs[j];
            objs[j] = tmp;
            i++;
            j--;
        }
    }
    if (left < j) qsort_desc(objs, left, j);
    if (i < right) qsort_desc(objs, i, right);
}

static void nms(Object *objs, int *picked, int total, int *picked_num, float threshold) {
    int i, j;
    float areas[MAX_PROPOSALS];
    for (i = 0; i < total; i++)
        areas[i] = objs[i].width * objs[i].height;

    *picked_num = 0;
    for (i = 0; i < total; i++) {
        int keep = 1;
        for (j = 0; j < *picked_num; j++) {
            int idx = picked[j];
            float inter = intersection_area(&objs[i], &objs[idx]);
            float union_area = areas[i] + areas[idx] - inter;
            if (inter / union_area > threshold) {
                keep = 0;
                break;
            }
        }
        if (keep)
            picked[(*picked_num)++] = i;
    }
}

static void generate_proposals(int stride, const float *feat, float prob_threshold, Object *objects, int *num_objects, int input_w, int input_h) {
    static float anchors[18] = {
        10, 13, 16, 30, 33, 23,
        30, 61, 62, 45, 59, 119,
        116, 90, 156, 198, 373, 326
    };

    int anchor_group = (stride == 8) ? 1 : (stride == 16) ? 2 : 3;
    int feat_w = input_w / stride;
    int feat_h = input_h / stride;
    int feat_size = feat_w * feat_h;
    int feat_size_all = feat_size * (CLASSES + 5);

    float de_threshold = desigmoid(prob_threshold);

    int count = *num_objects;
    for (int h = 0; h < feat_h; h++) {
        for (int w = 0; w < feat_w; w++) {
            for (int a = 0; a < ANCHORS; a++) {
                int offset = a * feat_size_all + (h * feat_w + w) * (CLASSES + 5);
                float box_score = feat[offset + 4];

                float class_score = -FLT_MAX;
                int class_idx = -1;
                for (int c = 0; c < CLASSES; c++) {
                    float s = feat[offset + 5 + c];
                    if (s > class_score) {
                        class_score = s;
                        class_idx = c;
                    }
                }

                if (box_score >= de_threshold && class_score >= de_threshold) {
                    float final_score = sigmoid(box_score) * sigmoid(class_score);
                    if (final_score >= prob_threshold && count < MAX_PROPOSALS) {
                        float dx = sigmoid(feat[offset + 0]);
                        float dy = sigmoid(feat[offset + 1]);
                        float dw = sigmoid(feat[offset + 2]);
                        float dh = sigmoid(feat[offset + 3]);

                        float anchor_w = anchors[(anchor_group - 1) * 6 + a * 2 + 0];
                        float anchor_h = anchors[(anchor_group - 1) * 6 + a * 2 + 1];

                        float cx = (dx * 2.f - 0.5f + w) * stride;
                        float cy = (dy * 2.f - 0.5f + h) * stride;
                        float pw = dw * dw * 4.f * anchor_w;
                        float ph = dh * dh * 4.f * anchor_h;

                        Object obj;
                        obj.x = cx - pw * 0.5f;
                        obj.y = cy - ph * 0.5f;
                        obj.width = pw;
                        obj.height = ph;
                        obj.prob = final_score;
                        obj.label = class_idx;
						obj.label_text = class_names[class_idx];

                        objects[count++] = obj;
                    }
                }
            }
        }
    }
    *num_objects = count;
}

int yolov5_post_process_c(int img_w, int img_h, float **output, Object *results, int max_results) {
    float prob_threshold = 0.4f;
    float nms_threshold = 0.45f;

    Object *proposals = (Object *)malloc(sizeof(Object) * MAX_PROPOSALS);
    int proposal_count = 0;

    generate_proposals(32, output[2], prob_threshold, proposals, &proposal_count, 640, 640);
    generate_proposals(16, output[1], prob_threshold, proposals, &proposal_count, 640, 640);
    generate_proposals(8,  output[0], prob_threshold, proposals, &proposal_count, 640, 640);

    qsort_desc(proposals, 0, proposal_count - 1);

    int picked[MAX_PROPOSALS];
    int picked_num = 0;
    nms(proposals, picked, proposal_count, &picked_num, nms_threshold);

    int final_count = (picked_num < max_results) ? picked_num : max_results;
    for (int i = 0; i < final_count; i++) {
        results[i] = proposals[picked[i]];

        // Resize to original image coordinates (assuming letterbox resize from 640x640)
        float scale = fminf(640.0f / img_h, 640.0f / img_w);
        int resize_w = (int)(scale * img_w);
        int resize_h = (int)(scale * img_h);
        int pad_x = (640 - resize_w) / 2;
        int pad_y = (640 - resize_h) / 2;

        results[i].x = fmaxf(0.f, (results[i].x - pad_x) / scale);
        results[i].y = fmaxf(0.f, (results[i].y - pad_y) / scale);
        results[i].width = fminf(img_w - results[i].x, results[i].width / scale);
        results[i].height = fminf(img_h - results[i].y, results[i].height / scale);
    }

    free(proposals);
    proposals = NULL;
    return final_count;
}
