/*
 * @Description:
 * @FilePath: /tal_audio_subsys/include/resample/audio_subsys_resample.h
 * @Version: v0.0.0
 * @Company:
 * @Author:
 * @Date: 2024-06-28
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-08-01 17:17:53
 */
#ifndef __AUDIO_SUBSYS_RESAMPLE_H__
#define __AUDIO_SUBSYS_RESAMPLE_H__

#include "audio_subsys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sample_rate_in;
    int sample_rate_out;
} resample_config_t;

#define RESAMPLE_CONFIG_16KHZ_TO_8KHZ() {   \
    .sample_rate_in = 16000,                \
    .sample_rate_out = 8000,                \
}

#define RESAMPLE_CONFIG_8KHZ_TO_16KHZ() {   \
    .sample_rate_in = 8000,                 \
    .sample_rate_out = 16000,               \
}

#define RESAMPLE_CONFIG_DEFAULT_BYPASS() {  \
    .sample_rate_in = 16000,                \
    .sample_rate_out = 16000,               \
}

int audio_subsys_resample_create(void **self, void *cfg);
int audio_subsys_resample_process(void *self, const void *in, int inlen, void *out, int outlen);
int audio_subsys_resample_destroy(void *self);

extern audio_subsys_module_t audio_subsys_resample;

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SUBSYS_RESAMPLE_H__ */
