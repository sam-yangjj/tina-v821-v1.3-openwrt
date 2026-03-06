/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/webrtc_apm/audio_subsys_webrtc_apm_wrap.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-07-17 16:42:37
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-08-01 18:07:19
 */
#ifndef __AUDIO_SUBSYS_WEBRTC_APM_WRAP_H__
#define __AUDIO_SUBSYS_WEBRTC_APM_WRAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef struct webrtc_apm_wrap_config {
    struct {
        int sample_rate;
        int mic_channels;
        int ref_channels;
        int frame_duration;
        int echo_delay_ms;
    } audio;
    struct {
        int enable;
        int frequency;
    } lowpass;
    struct {
        int enable;
        int frequency;
    } highpass;
    struct {
        int enable;
        int quality;
    } resampler;
    struct {
        int enable;
        int target_level_dbfs;
        int compression_gain_db;
        int limiter_enable;
    } agc;
    struct {
        int enable;
        int mode;
    } ans;
    struct {
        int enable;
        int nlp_mode;
        int skew_mode;
        int metrics_mode;
        int delay_logging;
    } aec;
    struct {
        int enable;
        int mode;
    } vad;
} webrtc_apm_wrap_config_t;

#define WEBRTC_APM_WRAP_CONFIG_DEFAULT() {  \
    .audio = {                              \
        .sample_rate = 16000,               \
        .mic_channels = 1,                  \
        .ref_channels = 1,                  \
        .frame_duration = 10,               \
        .echo_delay_ms = 100,               \
    },                                      \
    .lowpass = {                            \
        .enable = true,                     \
        .frequency = 12000,                 \
    },                                      \
    .highpass = {                           \
        .enable = true,                     \
        .frequency = 100,                   \
    },                                      \
    .resampler = {                          \
        .enable = true,                     \
        .quality = 4,                       \
    },                                      \
    .agc = {                                \
        .enable = true,                     \
        .target_level_dbfs = 3,             \
        .compression_gain_db = 20,          \
        .limiter_enable = true,             \
    },                                      \
    .ans = {                                \
        .enable = true,                     \
        .mode = 2,                          \
    },                                      \
    .aec = {                                \
        .enable = true,                     \
        .nlp_mode = 2,                      \
        .skew_mode = false,                 \
        .metrics_mode =false,               \
        .delay_logging = false,             \
    },                                      \
    .vad = {                                \
        .enable = true,                     \
        .mode =1,                           \
    },                                      \
}

extern audio_subsys_module_t audio_subsys_webrtc_apm_wrap;

int audio_subsys_webrtc_apm_wrap_create(void **self, void *cfg);
int audio_subsys_webrtc_apm_wrap_destroy(void *self);
int audio_subsys_webrtc_apm_wrap_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_webrtc_apm_wrap_ioctl(void *self, int cmd, void *arg);
int audio_subsys_webrtc_apm_wrap_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_WEBRTC_APM_WRAP_H__  */
