/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/filter/audio_subsys_filter.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-06-26 17:43:02
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-07-25 17:48:51
 */
#ifndef __AUDIO_SUBSYS_FILTER_H__
#define __AUDIO_SUBSYS_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARC_LINUX
#include "audio_subsys.h"
#endif

typedef struct filter_config {
    int sample_rate;
    int hpf_enable;
    int lpf_enable;
    float hpf_cutoff;
    float lpf_cutoff;
} filter_config_t;

#define FILTER_CONFIG_DEFAULT() { \
    .sample_rate = 16000,   \
    \
    .hpf_enable = 1,    \
    .lpf_enable = 1,    \
    .hpf_cutoff = 100,  \
    .lpf_cutoff = 4000, \
}

#ifndef ARC_LINUX
extern audio_subsys_module_t audio_subsys_filter;
#endif

int audio_subsys_filter_create(void **self, void *cfg);
int audio_subsys_filter_destroy(void *self);
#ifndef ARC_LINUX
int audio_subsys_filter_set_callback(void *self, audio_subsys_callback_t cb, void *userdata);
#endif
int audio_subsys_filter_ioctl(void *self, int cmd, void *arg);
int audio_subsys_filter_process(void *self, const void *in, int inlen, void *out, int outlen);

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_FILTER_H__ */
