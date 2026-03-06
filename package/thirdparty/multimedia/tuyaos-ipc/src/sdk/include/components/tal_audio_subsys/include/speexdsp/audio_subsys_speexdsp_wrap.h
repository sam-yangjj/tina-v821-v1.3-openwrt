/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/speexdsp/audio_subsys_speexdsp_wrap.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-07-03 10:40:54
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-10-22 10:37:37
 */
#ifndef __AUDIO_SUBSYS_SPEEXDSP_WRAP_H__
#define __AUDIO_SUBSYS_SPEEXDSP_WRAP_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "audio_subsys.h"

typedef struct speexdsp_wrap_config {
    struct {
        int sample_rate;
        int mic_channels;
        int ref_channels;
        int frame_duration;
        int echo_delay_ms;
    } audio;
    struct {
        int enable;
        int sample_rate;
        int quality;
    } resample;
    struct {
        int enable;
        float agc_level;
        int agc_increment;
        int agc_decrement;
        int agc_max_gain;
    } agc;
    struct {
        int enable;
        int noise_suppress;
        int dereverb;
        float dereverb_decay;
        float dereverb_level;
    } ns;
    struct {
        int enable;
        int echo_suppress;
        int echo_suppress_active;
    } aec;
    struct {
        int enable;
        int prob_start;
        int prob_continue;
    } vad;
} speexdsp_wrap_config_t;

#define SPEEXDSP_WRAP_CONFIG_DEFAULT() {    \
    .audio = {                              \
        .sample_rate = 16000,               \
        .mic_channels = 1,                  \
        .ref_channels = 1,                  \
        .frame_duration = 10,               \
        .echo_delay_ms = 100,               \
    },                                      \
    .resample = {                           \
        .enable = false,                    \
        .sample_rate = 16000,               \
        .quality = 10,                      \
    },                                      \
    .agc = {                                \
        .enable = true,                     \
        .agc_level = 32768.0f,              \
        .agc_increment = 12,                \
        .agc_decrement = -40,               \
        .agc_max_gain = 30,                 \
    },                                      \
    .ns = {                                 \
        .enable = true,                     \
        .noise_suppress = -15,              \
        .dereverb = false,                  \
        .dereverb_decay = 0.0f,             \
        .dereverb_level = 0.0f,             \
    },                                      \
    .aec = {                                \
        .enable = true,                     \
        .echo_suppress = -40,               \
        .echo_suppress_active = -15,        \
    },                                      \
    .vad = {                                \
        .enable = true,                     \
        .prob_start = 80,                   \
        .prob_continue = 65,                \
    }                                       \
}

extern audio_subsys_module_t audio_subsys_speexdsp_wrap;

int audio_subsys_speexdsp_wrap_create(void **self, void *cfg);
int audio_subsys_speexdsp_wrap_destroy(void *self);
int audio_subsys_speexdsp_wrap_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_speexdsp_wrap_ioctl(void *self, int cmd, void *arg);
int audio_subsys_speexdsp_wrap_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_SPEEXDSP_WRAP_H__ */
