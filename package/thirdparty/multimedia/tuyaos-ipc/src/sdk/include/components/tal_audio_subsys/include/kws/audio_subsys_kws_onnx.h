/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/kws/audio_subsys_kws_onnx.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-11-22 15:09:54
 * @LastEditors: clare.phang
 * @LastEditTime: 2025-01-10 18:24:10
 */
#ifndef __AUDIO_SUBSYS_KWS_ONNX_H__
#define __AUDIO_SUBSYS_KWS_ONNX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef struct kws_onnx_config {
    int     ringbuffer_enable;  ///< 0: disable, 1: enable
    int     thread_enable;      ///< 0: disable, 1: enable
    int     sample_rate;
    int     interval;
    int     model_type;
    int     num_bins;
    int     batch_size;
    float   threshold;
    char   *key_word;
    char   *model_path;
    char   *token_path;
} kws_onnx_config_t;

#define KWS_ONNX_CONFIG_DEFAULT_DSTCN_MAXPOOLING_NIHAO_WENWEN() {   \
    .thread_enable = 1,                                             \
    .ringbuffer_enable = 1,                                         \
    .sample_rate = 16000,                                           \
    .interval = 100,                                                \
    .model_type = 0,                                                \
    .num_bins = 40,                                                 \
    .batch_size = 8,                                                \
    .threshold = 0.8,                                               \
    .key_word = (char *)"你好问问",                                  \
    .model_path = (char *)"./model/dstcn-maxpooling-wenwen.ort",    \
    .token_path = (char *)"./model/dstcn-maxpooling-wenwen.token.txt",  \
}

#define KWS_ONNX_CONFIG_DEFAULT_FSMN_CTC_NIHAO_WENWEN() {           \
    .thread_enable = 1,                                             \
    .ringbuffer_enable = 1,                                         \
    .sample_rate = 16000,                                           \
    .interval = 100,                                                \
    .model_type = 1,                                                \
    .num_bins = 80,                                                 \
    .batch_size = 2,                                                \
    .threshold = 0.8,                                               \
    .key_word = (char *)"你好问问",                                  \
    .model_path = (char *)"./model/fsmn-ctc-wenwen.ort",            \
    .token_path = (char *)"./token/fsmn-ctc-wenwen.token.txt",      \
}

extern audio_subsys_module_t audio_subsys_kws_onnx;

int audio_subsys_kws_onnx_create(void **self, void *cfg);
int audio_subsys_kws_onnx_destroy(void *self);
int audio_subsys_kws_onnx_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_kws_onnx_ioctl(void *self, int cmd, void *arg);
int audio_subsys_kws_onnx_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_KWS_ONNX_H__ */
