/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/doabf/audio_subsys_doabf.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-09-26 18:43:13
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-10-09 15:52:37
 */
#ifndef __AUDIO_SUBSYS_DOABF_H__
#define __AUDIO_SUBSYS_DOABF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

#define BF_PCM_FLAG     202
#define DOA_STR_FLAG    101

typedef struct doabf_config {
    struct {
        int sample_rate;
        int bits_pre_sample;
        int mic_channels;
        int ref_channels;
        int out_ref_channels;
    } audio;
    struct doa {
        int circular;   ///< 0: linear, 1: circular
        float mic_distance;
        int margin;
    } doa;
    struct bf {
        int mode;       ///< 0: ds, 1: gsc, 2: mvdr
        struct ds {
            int tdoa_window;
            int beam_window;
        } ds;
        struct gsc {
            int num_k;
            float alpha;
        } gsc;
        struct mvdr {
            float frame_len;
            float frame_shift;
            int fft_point;
            float energy_thresh;
            int sil_to_speech_trigger;
            int speech_to_sil_trigger;
        } mvdr;
    } bf;
} doabf_config_t;

#define DOABF_CONFIG_DEFAULT() {                \
    .audio = {                                  \
        .sample_rate = 16000,                   \
        .bits_pre_sample = sizeof(short) * 8,   \
        .mic_channels = 4,                      \
        .ref_channels = 2,                      \
        .out_ref_channels = 1,                  \
    },                                          \
    .doa  = {                                   \
        .circular = 1,                          \
        .mic_distance = 0.042,                  \
        .margin = 16,                           \
    },                                          \
    .bf = {                                     \
        .mode = 0,                              \
        .ds = {                                 \
            .tdoa_window = 4000,                \
            .beam_window = 4000,                \
        },                                      \
        .gsc = {                                \
            .num_k = 400,                       \
            .alpha = 0.01,                      \
        },                                      \
        .mvdr = {                               \
            .frame_len = 0.025,                 \
            .frame_shift = 0.01,                \
            .fft_point = 512,                   \
            .energy_thresh = 1.5e-7,            \
            .sil_to_speech_trigger = 3,         \
            .speech_to_sil_trigger = 10,        \
        }                                       \
    }                                           \
}

extern audio_subsys_module_t audio_subsys_doabf;

int audio_subsys_doabf_create(void **self, void *cfg);
int audio_subsys_doabf_destroy(void *self);
int audio_subsys_doabf_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_doabf_ioctl(void *self, int cmd, void *arg);
int audio_subsys_doabf_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_DOABF_H__ */
