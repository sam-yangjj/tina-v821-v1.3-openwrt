#include "frame_check.h"

// 比较函数（完全一致）
static int compare_frames(const uint8_t *frame1, const uint8_t *frame2, size_t size) {
    return memcmp(frame1, frame2, size) == 0;
}

// 初始化，设置单帧大小
uint8_t* init_checker(size_t frame_size) {
    uint8_t* ref_frame = (uint8_t *)malloc(frame_size);
    if (!ref_frame) {
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }
    return ref_frame;
}

// 输入一帧数据进行检测
int check_frame(const uint8_t *frame, uint8_t *ref_frame, int frame_size, int frame_index) {

    if (frame_index == 1) {
        // 第一帧作为基准帧
        memcpy(ref_frame, frame, frame_size);
        printf("第 1 帧作为基准帧\n");
    } else if(frame_index > 1){
        if (!compare_frames(ref_frame, frame, frame_size)) {
            //printf("第 %d 帧与基准帧不一致！GDC 输出异常\n", frame_index);
            //exit(-1);
            return -1;
        } else {
            //printf("第 %d 帧与基准帧一致\n", frame_index);
            return 0;
        }
    }
    return 0;
}

// 释放资源
void release_checker(uint8_t *ref_frame) {
    if (ref_frame) {
        free(ref_frame);
        ref_frame = NULL;
    }
}

