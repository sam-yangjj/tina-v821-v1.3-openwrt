/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/doabf/audio_subsys_odaslive.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-08-28 16:51:08
 * @LastEditors: clare.phang
 * @LastEditTime: 2025-01-21 14:46:35
 */
#ifndef __AUDIO_SUBSYS_ODASLIVE_H__
#define __AUDIO_SUBSYS_ODASLIVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_subsys.h"

typedef enum odaslive_flag {
    ODASLIVE_FLAG_POTENTIAL     = 101,  ///< potential sources, json string
    ODASLIVE_FLAG_TRACKED       = 102,  ///< tracked sources, json string

    ODASLIVE_FLAG_SEPARATED     = 201,  ///< separated sources, pcm mono audio data
    ODASLIVE_FLAG_POSTFILTERED  = 202,  ///< post filtered sources, pcm mono audio data
} odaslive_flag_t;

typedef struct odaslive_config {
    struct {
        int sample_rate;
        int bits_pre_sample;
        int mic_channels;
        int ref_channels;
        int out_ref_channels;
    } audio;
    char *configfile;   /**< odaslive config file path */
    int thread_enable;  /**< odaslive thread enable, 0: disable use processor, 1: enable use thread */
    int thread_multi;   /**< odaslive thread runtime mode, 0: single thread, 1: multi thread */
    int stream_enable;  /**< odaslive stream enable, 0: disable, 1: enable,
                            1. use stream need mutex thread_* enable
                            2. disable default use socket, need thread_* enable */
    int sep_mix_enable; /**< odaslive separated 4chs pcm to mix mono out enable */
    int pfs_mix_enable; /**< odaslive postfiltered 4chs pcm to mix mono out enable */
} odaslive_config_t;

#define ODASLIVE_DEFAULT_CONFIG() {             \
    .audio = {                                  \
        .sample_rate = 16000,                   \
        .bits_pre_sample = sizeof(short) * 8,   \
        .mic_channels = 4,                      \
        .ref_channels = 2,                      \
        .out_ref_channels = 1,                  \
    },                                          \
    .configfile = (char *)"./odaslive.cfg",     \
    .thread_enable = 1,                         \
    .thread_multi = 0,                          \
    .stream_enable = 0,                         \
    .sep_mix_enable = 0,                        \
    .pfs_mix_enable = 0,                        \
}

#define ODASLIVE_STREAM_CONFIG() {              \
    .audio = {                                  \
        .sample_rate = 16000,                   \
        .bits_pre_sample = sizeof(short) * 8,   \
        .mic_channels = 4,                      \
        .ref_channels = 2,                      \
        .out_ref_channels = 1,                  \
    },                                          \
    .configfile = (char *)"./odaslive.cfg",     \
    .thread_enable = 0,                         \
    .thread_multi = 0,                          \
    .stream_enable = 1,                         \
    .sep_mix_enable = 0,                        \
    .pfs_mix_enable = 0,                        \
}

extern audio_subsys_module_t audio_subsys_odaslive;

int audio_subsys_odaslive_create(void **self, void *cfg);
int audio_subsys_odaslive_destroy(void *self);
int audio_subsys_odaslive_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
int audio_subsys_odaslive_ioctl(void *self, int cmd, void *arg);
int audio_subsys_odaslive_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_ODASLIVE_H__ */
