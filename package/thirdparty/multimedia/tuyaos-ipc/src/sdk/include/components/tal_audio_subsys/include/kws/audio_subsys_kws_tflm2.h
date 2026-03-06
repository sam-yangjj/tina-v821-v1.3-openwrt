/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/kws/audio_subsys_kws_tflm2.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-12-16 19:15:23
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-12-25 16:32:19
 */
#ifndef __AUDIO_SUBSYS_KWS_TFLM2_H__
#define __AUDIO_SUBSYS_KWS_TFLM2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef struct kws_tflm2_config {
    int ringbuffer_enable;  ///< 0: disable, 1: enable
    int interval;           ///< unit ms
    struct tflm2 {
        void   *tflite_model;
        int     tflite_model_len;
        int     tensor_arena_size;
        int     max_audio_sample_size;
        int     audio_sample_frequency;
        int     feature_size;
        int     feature_count;
        int     feature_stride_ms;
        int     feature_duration_ms;
        int     average_window_duration_ms;
        float   detection_threshold;
        int     suppression_ms;
        int     minimum_count;
        int     category_count;
        const char **category_labels;
    } tflm2;
} kws_tflm2_config_t;

#define KWS_TFLM2_CONFIG_DEFAULT() {                                    \
    .ringbuffer_enable = 1,                                             \
    .interval = 100,                                                    \
    .tflm2 = {                                                          \
        .tflite_model = NULL,                                           \
        .tflite_model_len = 0,                                          \
        .tensor_arena_size = 30 * 1024,                                 \
        .max_audio_sample_size = 512,                                   \
        .audio_sample_frequency = 16000,                                \
        .feature_size = 40,                                             \
        .feature_count = 49,                                            \
        .feature_stride_ms = 20,                                        \
        .feature_duration_ms = 30,                                      \
        .average_window_duration_ms = 1000,                             \
        .detection_threshold = 0.8,                                     \
        .suppression_ms = 1500,                                         \
        .minimum_count = 3,                                             \
        .category_count = 4,                                            \
        .category_labels = (const char*[]){"silence","unknown","yes1","no1"}, \
    }                                                                   \
}

extern audio_subsys_module_t audio_subsys_kws_tflm2;

int audio_subsys_kws_tflm2_create(void **self, void *cfg);
int audio_subsys_kws_tflm2_destroy(void *self);
int audio_subsys_kws_tflm2_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_kws_tflm2_ioctl(void *self, int cmd, void *arg);
int audio_subsys_kws_tflm2_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /* !__AUDIO_SUBSYS_KWS_TFLM2_H__ */
