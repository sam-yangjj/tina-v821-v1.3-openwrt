/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/kws/audio_subsys_kws_tflm.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-10-29 17:54:39
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-11-14 13:42:42
 */
#ifndef __AUDIO_SUBSYS_KWS_TFLM_H__
#define __AUDIO_SUBSYS_KWS_TFLM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

/** TODO: config 字段成员可以细致优化 */
typedef struct kws_tflm_config {
    struct tflm {
        int clip_duration_ms;           ///< 音频剪辑的长度（毫秒）需要和训练模型的参数一致
        int model_windows_ms;           ///< 频率窗口的时间（毫秒）需要和训练模型的参数一致
        int model_strid_windows_ms;     ///< 频率窗口之间移动的时间（毫秒）需要和训练模型的参数一致
        int num_mfcc_features;          ///< 计算梅尔频率倒谱系数（MFCC）时要保留的系数数量.需要和训练模型的参数一致
        int num_fbank;                  ///< 计算梅尔频率倒谱系数（MFCC）时要使用的梅尔滤波器组.需要和训练模型的参数一致
        int sample_rate;                ///< 音频采样率
        int sample_width;               ///< 音频宽度
        float peempha;                  ///< 音频增强系数，用来提高高频部分的音频。一般人声的系数设置为0.95~0.97之间
        float max_threshold;            ///< threshold 最大概率的阈值
        int model_width;                ///< tensorflow数据位宽，譬如 8(int8) 16(int16), 理论上存在 int8 uint8 int16 uint16 int32 uint32 float32.
        int model_type;                 ///< tensorflow模型类型
        int tensor_size;                ///< tensorflow的张量需要内存大小
        int tensor_model_size;          ///< tensorflow训练的模型大小
        unsigned char *tensor_model;    ///< tensorflow训练的模型
    } tflm;                             ///< WARNING: 需要保持与tflm.h tflm_config_t 定义一致。
    struct vad {
        int enable;                     ///< KWS vad Enable control
        int mode;                       ///< KWS vad Mode, 0: Quality mode, 1: Low bitrate mode, 2. Aggressive mode, 3: Very aggressive mode
    } vad;
    struct kws {
        int half_retry;                 ///< tensorflow clip duration half retry, 为了应对KWS音频帧被切分，导致模型推理结果不准确，是否做重试冗余判断
        float half_retry_scale;         ///< tensorflow clip duration retry scale, 为了应对KWS音频帧被切分，导致模型推理结果不准确，若做冗余判断设置切分系数调整
    } kws;
} kws_tflm_config_t;

#define KWS_TFLM_CONFIG_DEFAULT() {                                     \
    .tflm = {                                                           \
        .clip_duration_ms = 1000,                                       \
        .model_windows_ms = 32,                                         \
        .model_strid_windows_ms = (32 / 2), /* model_windows_ms / 2 */  \
        .num_mfcc_features = 20,                                        \
        .num_fbank = 40,                                                \
        .sample_rate = 8000,                                            \
        .sample_width = 16,                                             \
        .peempha = 0.970f,                                              \
        .max_threshold = 0.985f,                                        \
        .model_width = 8,                                               \
        .model_type = 2,                                                \
        .tensor_size = (64 * 1024),                                     \
        .tensor_model_size = 0,                                         \
        .tensor_model = NULL,                                           \
    },                                                                  \
    .vad = {                                                            \
        .enable = 1,                                                    \
        .mode = 2,                                                      \
    },                                                                  \
    .kws = {                                                            \
        .half_retry = 1,                                                \
        .half_retry_scale = 0.500f,                                     \
    },                                                                  \
}

extern audio_subsys_module_t audio_subsys_kws_tflm;

int audio_subsys_kws_tflm_create(void **self, void *cfg);
int audio_subsys_kws_tflm_destroy(void *self);
int audio_subsys_kws_tflm_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_kws_tflm_ioctl(void *self, int cmd, void *arg);
int audio_subsys_kws_tflm_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /* !__AUDIO_SUBSYS_KWS_TFLM_H__ */
